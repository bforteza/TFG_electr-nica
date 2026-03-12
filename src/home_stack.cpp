#include "home_stack.h"
#include "window.h"
#include "globals.h"
#include <gtkmm/messagedialog.h>

HomeStack::HomeStack(const Glib::RefPtr<Gtk::Builder>& builder, Window* window)
    : window_(window),
      users_view_stack_(builder),
      user_create_stack_(builder),
      key_create_stack_(builder),
      key_view_stack_(builder)
{
    builder->get_widget("BackButtonAdmin", back_button_admin_);
    if (!back_button_admin_)
        throw std::runtime_error("No \"BackButtonAdmin\" object in MainWindow.glade");
    back_button_admin_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnBackButtonClicked));

    builder->get_widget("UserCreateButton", user_create_button_);
    if (!user_create_button_)
        throw std::runtime_error("No \"UserCreateButton\" object in MainWindow.glade");
    user_create_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnUserCreateButtonClicked));

    builder->get_widget("KeyCreateButton", key_create_button_);
    if (!key_create_button_)
        throw std::runtime_error("No \"KeyCreateButton\" object in MainWindow.glade");
    key_create_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnKeyCreateButtonClicked));

    builder->get_widget("ViewUsersButton", view_users_button_);
    if (!view_users_button_)
        throw std::runtime_error("No \"ViewUsersButton\" object in MainWindow.glade");
    view_users_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnViewUsersButtonClicked));

    builder->get_widget("ViewKeysButton", view_keys_button_);
    if (!view_keys_button_)
        throw std::runtime_error("No \"ViewKeysButton\" object in MainWindow.glade");
    view_keys_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnViewKeysButtonClicked));

    builder->get_widget("HistoryButton", history_button_);
    if (!history_button_)
        throw std::runtime_error("No \"HistoryButton\" object in MainWindow.glade");
    // TODO: conectar HistoryButton cuando el historial esté implementado.

    builder->get_widget("SolenoidPanelButton", solenoid_panel_button_);
    if (!solenoid_panel_button_)
        throw std::runtime_error("No \"SolenoidPanelButton\" object in MainWindow.glade");
    solenoid_panel_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &HomeStack::OnSolenoidPanelButtonClicked));

    builder->get_widget("HomeInnerStack", inner_stack_);
    if (!inner_stack_)
        throw std::runtime_error("No \"HomeInnerStack\" object in MainWindow.glade");

    builder->get_widget("NameLabel", name_label_);
    if (!name_label_)
        throw std::runtime_error("No \"NameLabel\" object in MainWindow.glade");

    // Conecta las señales de las sub-vistas con los manejadores de este panel.
    users_view_stack_.user_link.connect(sigc::mem_fun(*this, &HomeStack::OnUserLinkKey));
    users_view_stack_.user_unlink.connect(sigc::mem_fun(*this, &HomeStack::OnUserUnlinkKey));
    users_view_stack_.keys_view.connect(sigc::mem_fun(*this, &HomeStack::ShowKeysView));
    users_view_stack_.user_edit.connect(sigc::mem_fun(*this, &HomeStack::OnUserEdit));

    key_view_stack_.key_link.connect(sigc::mem_fun(*this, &HomeStack::OnKeyLinkUser));
    key_view_stack_.key_unlink.connect(sigc::mem_fun(*this, &HomeStack::OnKeyUnlinkUser));
    key_view_stack_.users_view.connect(sigc::mem_fun(*this, &HomeStack::ShowUsersView));
    key_view_stack_.key_edit.connect(sigc::mem_fun(*this, &HomeStack::OnKeyEdit));
    key_view_stack_.key_kept.connect(sigc::mem_fun(*this, &HomeStack::OnKeyKept));
    key_view_stack_.key_delete.connect(sigc::mem_fun(*this, &HomeStack::OnKeyDelete));

    key_create_stack_.position_select_requested.connect(
        sigc::mem_fun(*this, &HomeStack::OnPositionSelectRequested));

    // Suscribe RefreshLabels al cambio de idioma global.
    language_changed.connect(sigc::mem_fun(*this, &HomeStack::RefreshLabels));
    RefreshLabels();
}

void HomeStack::RefreshLabels() {
    solenoid_panel_button_->set_label(Tr().home.btn_solenoid_panel);
    user_create_button_->set_label(Tr().home.btn_create_user);
    key_create_button_->set_label(Tr().home.btn_create_key);
    view_keys_button_->set_label(Tr().home.btn_view_keys);
    view_users_button_->set_label(Tr().home.btn_view_users);
    history_button_->set_label(Tr().home.btn_history);
}

// --- Iniciador ---

void HomeStack::PersonLogged(std::shared_ptr<kdb::Person> person) {
    name_label_->set_text((std::string)person->name);
    HideButtons();

    int access = (int)person->a1 + 2 * (int)person->a2;
    if (access >= 0)
        view_keys_button_->show();
    if (access >= 1)
        key_create_button_->show();
    if (access >= 2) {
        view_users_button_->show();
        user_create_button_->show();
        solenoid_panel_button_->show();
    }

    logged_person_ = person;
}

// --- Navegación ---

void HomeStack::OnBackButtonClicked() {
    key_selected_connection_.disconnect();
    user_selected_connection_.disconnect();
    aux_key_    = nullptr;
    aux_person_ = nullptr;

    if (inner_stack_->get_visible_child_name() == "AdminMainStack") {
        window_->OnBackButtonClicked();
    } else if (back_widget_ == nullptr) {
        inner_stack_->set_visible_child("AdminMainStack");
    } else {
        inner_stack_->set_visible_child(*back_widget_);
        back_widget_ = nullptr;
    }
}

void HomeStack::OnUserCreateButtonClicked() {
    user_create_stack_.CreateUser();
    inner_stack_->set_visible_child("UserCreateStack");
}

void HomeStack::OnViewUsersButtonClicked() {
    users_view_stack_.view(litesql::select<kdb::Person>(*db).all());
    inner_stack_->set_visible_child("ViewUsersStack");
}

void HomeStack::OnViewKeysButtonClicked() {
    key_view_stack_.view(logged_person_);
    inner_stack_->set_visible_child("ViewKeyStack");
}

void HomeStack::OnKeyCreateButtonClicked() {
    key_create_stack_.CreateKey(logged_person_);
    inner_stack_->set_visible_child("KeyCreateStack");
}

// --- Auxiliares internos ---

void HomeStack::ShowKeysView(std::vector<kdb::Key> keys) {
    key_view_stack_.view(keys);
    back_widget_ = inner_stack_->get_visible_child();
    inner_stack_->set_visible_child("ViewKeyStack");
}

void HomeStack::ShowUsersView(std::vector<kdb::Person> users) {
    users_view_stack_.view(users);
    back_widget_ = inner_stack_->get_visible_child();
    inner_stack_->set_visible_child("ViewUsersStack");
}

void HomeStack::OnUserEdit(std::shared_ptr<kdb::Person> person) {
    user_create_stack_.UserEdit(person);
    back_widget_ = inner_stack_->get_visible_child();
    inner_stack_->set_visible_child("UserCreateStack");
}

void HomeStack::OnKeyEdit(std::shared_ptr<kdb::Key> key) {
    key_create_stack_.KeyEdit(key);
    back_widget_ = inner_stack_->get_visible_child();
    inner_stack_->set_visible_child("KeyCreateStack");
}

void HomeStack::OnUserLinkKey(std::shared_ptr<kdb::Person> person) {
    aux_person_ = person;

    if (aux_key_ == nullptr) {
        key_view_stack_.select(
            (litesql::except(litesql::select<kdb::Key>(*db), aux_person_->keys().get())).all());
        back_widget_ = inner_stack_->get_visible_child();
        key_selected_connection_ = key_view_stack_.key_selected.connect(
            sigc::mem_fun(*this, &HomeStack::OnKeyLinkUser));
        inner_stack_->set_visible_child("ViewKeyStack");
    } else {
        aux_person_->keys().link(*aux_key_);
        OnBackButtonClicked();
    }
}

void HomeStack::OnKeyLinkUser(std::shared_ptr<kdb::Key> key) {
    aux_key_ = key;

    if (aux_person_ == nullptr) {
        users_view_stack_.select(
            (litesql::except(litesql::select<kdb::Person>(*db), aux_key_->owners().get())).all());
        back_widget_ = inner_stack_->get_visible_child();
        user_selected_connection_ = users_view_stack_.user_selected.connect(
            sigc::mem_fun(*this, &HomeStack::OnUserLinkKey));
        inner_stack_->set_visible_child("ViewUsersStack");
    } else {
        aux_person_->keys().link(*aux_key_);
        OnBackButtonClicked();
    }
}

void HomeStack::OnUserUnlinkKey(std::shared_ptr<kdb::Person> person) {
    aux_person_ = person;

    if (aux_key_ == nullptr) {
        key_view_stack_.select(person->keys().get().all());
        back_widget_ = inner_stack_->get_visible_child();
        key_selected_connection_ = key_view_stack_.key_selected.connect(
            sigc::mem_fun(*this, &HomeStack::OnKeyUnlinkUser));
        inner_stack_->set_visible_child("ViewKeyStack");
    } else {
        aux_person_->keys().unlink(*aux_key_);
        OnBackButtonClicked();
    }
}

void HomeStack::OnKeyUnlinkUser(std::shared_ptr<kdb::Key> key) {
    aux_key_ = key;

    if (aux_person_ == nullptr) {
        users_view_stack_.select(key->owners().get().all());
        back_widget_ = inner_stack_->get_visible_child();
        user_selected_connection_ = users_view_stack_.user_selected.connect(
            sigc::mem_fun(*this, &HomeStack::OnUserUnlinkKey));
        inner_stack_->set_visible_child("ViewUsersStack");
    } else {
        aux_person_->keys().unlink(*aux_key_);
        OnBackButtonClicked();
    }
}

void HomeStack::OnKeyKept(std::shared_ptr<kdb::Key> key) {
    // Si la llave ya tiene portador, muestra aviso y cancela.
    if (key->keeper().get().count() > 0) {
        auto* dlg = new Gtk::MessageDialog(
            *window_, Tr().key_view.err_key_in_use,
            false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        auto closed = std::make_shared<bool>(false);
        dlg->signal_response().connect([dlg, closed](int) {
            *closed = true;
            dlg->hide();
            delete dlg;
        });
        Glib::signal_timeout().connect_once([dlg, closed]() {
            if (!*closed)
                dlg->response(Gtk::RESPONSE_OK);
        }, 2000);
        dlg->show();
        return;
    }

    logged_person_->keepkeys().link(*key);
    signal_open_solenoid.emit(key, SolenoidPanel::Mode::PICKUP);
}

void HomeStack::OnKeyDelete(std::shared_ptr<kdb::Key> key) {
    if (key->keeper().get().count() == 0) {
        auto* dlg = new Gtk::MessageDialog(
            *window_, Tr().key_view.err_key_in_cabinet,
            false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        auto closed = std::make_shared<bool>(false);
        dlg->signal_response().connect([dlg, closed](int) {
            *closed = true; dlg->hide(); delete dlg;
        });
        Glib::signal_timeout().connect_once([dlg, closed]() {
            if (!*closed) dlg->response(Gtk::RESPONSE_OK);
        }, 2000);
        dlg->show();
        return;
    }
    key->active = false;
    key->update();
    key_view_stack_.view(logged_person_);
}

void HomeStack::OnSolenoidPanelButtonClicked() {
    signal_open_solenoid.emit(nullptr, SolenoidPanel::Mode::ADMIN);
}

void HomeStack::OnPositionSelectRequested() {
    signal_open_solenoid.emit(nullptr, SolenoidPanel::Mode::SELECT);
}

void HomeStack::OnPositionSelected(int pos) {
    key_create_stack_.SetPosition(pos);
    inner_stack_->set_visible_child("KeyCreateStack");
}

void HomeStack::HideButtons() {
    solenoid_panel_button_->hide();
    user_create_button_->hide();
    key_create_button_->hide();
    view_keys_button_->hide();
    view_users_button_->hide();
    history_button_->hide();
}
