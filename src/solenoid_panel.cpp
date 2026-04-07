#include "solenoid_panel.h"
#include "window.h"
#include "globals.h"
#include "i2c_controller.h"
#include "translations.h"
#include <litesql.hpp>
#include <stdexcept>

SolenoidPanel::SolenoidPanel(const Glib::RefPtr<Gtk::Builder>& builder,
                             Window* window)
    : window_(window),
      current_mode_(Mode::PICKUP),
      countdown_(kTimeoutSeconds)
{
    builder->get_widget("SolenoidPanel", panel_box_);
    if (!panel_box_)
        throw std::runtime_error("No \"SolenoidPanel\" object in MainWindow.glade");

    builder->get_widget("BackButtonKeySelect", back_button_);
    if (!back_button_)
        throw std::runtime_error("No \"BackButtonKeySelect\" object in MainWindow.glade");
    back_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &SolenoidPanel::OnBackButtonClicked));

    BuildWidgets();
}

void SolenoidPanel::BuildWidgets() {
    // --- Grid 4×8 ---
    grid_ = Gtk::manage(new Gtk::Grid());
    grid_->set_row_homogeneous(true);
    grid_->set_column_homogeneous(true);
    grid_->set_row_spacing(4);
    grid_->set_column_spacing(4);
    grid_->set_margin_start(6);
    grid_->set_margin_end(6);
    grid_->set_margin_top(6);
    grid_->set_margin_bottom(6);

    for (int i = 0; i < kRows * kCols; i++) {
        int row = i / kCols;
        int col = i % kCols;
        auto* btn = Gtk::manage(new Gtk::Button(PosToString(i + 1)));
        btn->set_sensitive(false);
        btn->signal_clicked().connect(
            sigc::bind(sigc::mem_fun(*this, &SolenoidPanel::OnSlotButtonClicked), i));
        slot_buttons_[i] = btn;
        grid_->attach(*btn, col, row, 1, 1);
    }

    // --- Barra inferior de información ---
    info_bar_ = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 6));
    info_bar_->set_margin_start(10);
    info_bar_->set_margin_end(10);
    info_bar_->set_margin_bottom(8);

    key_info_label_   = Gtk::manage(new Gtk::Label(""));
    countdown_label_  = Gtk::manage(new Gtk::Label(""));

    auto* buttons_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
    repeat_button_ = Gtk::manage(new Gtk::Button("Repetir"));
    action_button_ = Gtk::manage(new Gtk::Button(""));
    buttons_box->set_halign(Gtk::ALIGN_CENTER);
    buttons_box->pack_start(*repeat_button_, false, false);
    buttons_box->pack_start(*action_button_, false, false);

    info_bar_->pack_start(*key_info_label_,  false, false);
    info_bar_->pack_start(*countdown_label_, false, false);
    info_bar_->pack_start(*buttons_box,      false, false);

    panel_box_->pack_start(*grid_,     true,  true);
    panel_box_->pack_start(*info_bar_, false, true);
    panel_box_->show_all();

    repeat_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &SolenoidPanel::OnRepeatButtonClicked));
    action_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &SolenoidPanel::OnActionButtonClicked));
}

// --- Configuración por modo ---

void SolenoidPanel::Setup(std::shared_ptr<kdb::Key> key, Mode mode) {
    StopTimer();
    current_key_       = key;
    current_mode_      = mode;
    active_admin_slot_ = -1;

    // En SELECT la sensibilidad se asigna slot a slot en UpdateGrid (libre=sí, ocupado=no).
    // En ADMIN todos son sensibles; en PICKUP/RETURN ninguno.
    if (mode != Mode::SELECT) {
        for (int i = 0; i < kRows * kCols; i++)
            slot_buttons_[i]->set_sensitive(mode == Mode::ADMIN);
    }

    UpdateGrid();

    switch (mode) {
        case Mode::PICKUP: {
            std::string pos_str = key ? PosToString((int)key->pos) : "?";
            std::string name    = key ? (std::string)key->name    : "";
            key_info_label_->set_text(name + " — " + pos_str);
            repeat_button_->set_label(Tr().solenoid.btn_repeat);
            action_button_->set_label(Tr().solenoid.btn_take_another);
            repeat_button_->show();
            action_button_->show();
            countdown_ = kTimeoutSeconds;
            countdown_label_->set_text(std::string("⏱ ") + std::to_string(countdown_) + "s");
            countdown_label_->show();
            if (key) hw_ctrl->Activate(static_cast<Position>((int)key->pos));
            hw_ctrl->OpenDoor();
            Glib::signal_timeout().connect_once([]{ hw_ctrl->CloseDoor(); }, 2000);
            StartTimer();
            break;
        }
        case Mode::RETURN: {
            std::string pos_str = key ? PosToString((int)key->pos) : "?";
            key_info_label_->set_text(Tr().solenoid.lbl_return + pos_str);
            repeat_button_->set_label(Tr().solenoid.btn_repeat);
            repeat_button_->show();
            action_button_->hide();
            countdown_ = kTimeoutSeconds;
            countdown_label_->set_text(std::string("⏱ ") + std::to_string(countdown_) + "s");
            countdown_label_->show();
            if (key) hw_ctrl->Activate(static_cast<Position>((int)key->pos));
            hw_ctrl->OpenDoor();
            Glib::signal_timeout().connect_once([]{ hw_ctrl->CloseDoor(); }, 2000);
            StartTimer();
            break;
        }
        case Mode::ADMIN:
            key_info_label_->set_text(Tr().solenoid.lbl_admin_title);
            repeat_button_->hide();
            action_button_->hide();
            countdown_label_->set_text("");
            hw_ctrl->OpenDoor();
            Glib::signal_timeout().connect_once([]{ hw_ctrl->CloseDoor(); }, 2000);
            break;
        case Mode::SELECT:
            key_info_label_->set_text(Tr().solenoid.lbl_select_title);
            repeat_button_->hide();
            action_button_->hide();
            countdown_label_->set_text("");
            break;
    }
}

// --- Grid de colores ---

void SolenoidPanel::UpdateGrid() {
    // Posiciones que tienen alguna llave asignada en la BD.
    std::set<int> occupied;
    try {
        for (auto& k : litesql::select<kdb::Key>(*db, kdb::Key::Active == true).all()) {
            int p = (int)k.pos;
            if (p >= 1 && p <= 32)
                occupied.insert(p - 1);   // índice 0-based
        }
    } catch (...) {}

    int target = -1;
    if (current_key_ && (int)current_key_->pos >= 1)
        target = (int)current_key_->pos - 1;

    for (int i = 0; i < kRows * kCols; i++) {
        auto ctx = slot_buttons_[i]->get_style_context();
        ctx->remove_class("slot-target");
        ctx->remove_class("slot-occupied");
        ctx->remove_class("slot-free");

        if (i == target)
            ctx->add_class("slot-target");
        else if (occupied.count(i))
            ctx->add_class("slot-occupied");
        else
            ctx->add_class("slot-free");

        // En SELECT: los slots libres son pulsables, los ocupados no.
        if (current_mode_ == Mode::SELECT)
            slot_buttons_[i]->set_sensitive(!occupied.count(i));
    }
}

// --- Temporizador ---

void SolenoidPanel::StartTimer() {
    timer_conn_ = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &SolenoidPanel::OnTimerTick), 1000);
}

void SolenoidPanel::StopTimer() {
    timer_conn_.disconnect();
}

bool SolenoidPanel::OnTimerTick() {
    --countdown_;
    countdown_label_->set_text(std::string("⏱ ") + std::to_string(countdown_) + "s");
    if (countdown_ <= 0) {
        hw_ctrl->Deactivate();
        signal_logout.emit();
        return false;
    }
    return true;
}

// --- Manejadores de botones ---

void SolenoidPanel::OnBackButtonClicked() {
    StopTimer();
    hw_ctrl->Deactivate();
    if (current_mode_ == Mode::RETURN)
        signal_logout.emit();
    else
        signal_go_home.emit();
}

void SolenoidPanel::OnRepeatButtonClicked() {
    if (current_key_) {
        hw_ctrl->OpenDoor();
        Glib::signal_timeout().connect_once([]{ hw_ctrl->CloseDoor(); }, 2000);
        countdown_ = kTimeoutSeconds;
        countdown_label_->set_text(std::string("⏱ ") + std::to_string(countdown_) + "s");
    }
}

void SolenoidPanel::OnActionButtonClicked() {
    // "Coger otra llave": vuelve al HomeStack manteniendo el estado.
    hw_ctrl->Deactivate();
    StopTimer();
    signal_go_home.emit();
}

void SolenoidPanel::OnSlotButtonClicked(int idx) {
    // En modo SELECT: emite la posición elegida (1-based) y vuelve al HomeStack.
    // Los slots ocupados ya son insensibles, así que idx siempre es una posición libre.
    if (current_mode_ == Mode::SELECT) {
        signal_position_selected.emit(idx + 1);
        signal_go_home.emit();
        return;
    }

    // Determina si un slot tiene llave asignada en la BD, para restaurar su clase CSS.
    auto all_keys = litesql::select<kdb::Key>(*db, kdb::Key::Active == true).all();
    auto IsOccupied = [&](int i) {
        for (auto& k : all_keys)
            if ((int)k.pos - 1 == i) return true;
        return false;
    };

    // Desactiva siempre el slot actual si hay alguno activo.
    if (active_admin_slot_ >= 0) {
        hw_ctrl->Deactivate();
        auto ctx = slot_buttons_[active_admin_slot_]->get_style_context();
        ctx->remove_class("slot-target");
        ctx->add_class(IsOccupied(active_admin_slot_) ? "slot-occupied" : "slot-free");
    }

    if (active_admin_slot_ == idx) {
        // Mismo slot: toggle off, no activar nada nuevo.
        active_admin_slot_ = -1;
        key_info_label_->set_text(Tr().solenoid.lbl_admin_title);
    } else {
        // Slot diferente: activa el nuevo.
        hw_ctrl->Activate(static_cast<Position>(idx + 1));
        auto ctx = slot_buttons_[idx]->get_style_context();
        ctx->remove_class("slot-occupied");
        ctx->remove_class("slot-free");
        ctx->add_class("slot-target");
        active_admin_slot_ = idx;
        key_info_label_->set_text(Tr().solenoid.lbl_activated + PosToString(idx + 1));

        // Buscar si hay una llave asignada a esta posición para el log.
        std::shared_ptr<kdb::Key> slot_key;
        for (auto& k : all_keys)
            if ((int)k.pos - 1 == idx) {
                slot_key = std::make_shared<kdb::Key>(k);
                break;
            }
        signal_slot_activated.emit(idx + 1, slot_key);
    }
}
