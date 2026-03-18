#include "key_create_stack.h"
#include "dbmanager.hpp"
#include <litesql/selectquery.hpp>
#include "globals.h"
#include "history_logger.h"
#include <string>

KeyCreateStack::KeyCreateStack(const Glib::RefPtr<Gtk::Builder>& builder) {
    builder->get_widget("KeyNameEntry", key_name_entry_);
    if (!key_name_entry_)
        throw std::runtime_error("No \"KeyNameEntry\" object in MainWindow.glade");

    builder->get_widget("UbiKeyEntry", ubi_entry_);
    if (!ubi_entry_)
        throw std::runtime_error("No \"UbiKeyEntry\" object in MainWindow.glade");

    builder->get_widget("ComentaryEntry", commentary_entry_);
    if (!commentary_entry_)
        throw std::runtime_error("No \"ComentaryEntry\" object in MainWindow.glade");

    builder->get_widget("KeyGenerateButton", generate_button_);
    if (!generate_button_)
        throw std::runtime_error("No \"KeyGenerateButton\" object in MainWindow.glade");
    generate_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyCreateStack::OnGenerateButtonClicked));

    builder->get_widget("AddUserButton", add_user_button_);
    if (!add_user_button_)
        throw std::runtime_error("No \"AddUserButton\" object in MainWindow.glade");
    add_user_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyCreateStack::OnAddUserButtonClicked));

    builder->get_widget("AddUidKeyButton", add_uid_button_);
    if (!add_uid_button_)
        throw std::runtime_error("No \"AddUidKeyButton\" object in MainWindow.glade");
    add_uid_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyCreateStack::OnAddUidButtonClicked));

    builder->get_widget("PositionPickerButton", position_picker_button_);
    if (!position_picker_button_)
        throw std::runtime_error("No \"PositionPickerButton\" object in MainWindow.glade");
    position_picker_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyCreateStack::OnPositionPickerButtonClicked));

    builder->get_widget("KeyCreateNameLabel", name_label_);
    if (!name_label_)
        throw std::runtime_error("No \"KeyCreateNameLabel\" object in MainWindow.glade");

    builder->get_widget("KeyCreateUbiLabel", ubi_label_);
    if (!ubi_label_)
        throw std::runtime_error("No \"KeyCreateUbiLabel\" object in MainWindow.glade");

    builder->get_widget("KeyCreateCommentaryLabel", commentary_label_);
    if (!commentary_label_)
        throw std::runtime_error("No \"KeyCreateCommentaryLabel\" object in MainWindow.glade");

    builder->get_widget("KeyNameErrorLabel", name_error_label_);
    if (!name_error_label_)
        throw std::runtime_error("No \"KeyNameErrorLabel\" object in MainWindow.glade");

    builder->get_widget("UidKeyErrorLabel", uid_error_label_);
    if (!uid_error_label_)
        throw std::runtime_error("No \"UidKeyErrorLabel\" object in MainWindow.glade");

    builder->get_widget("UidKeyText", uid_text_view_);
    if (!uid_text_view_)
        throw std::runtime_error("No \"UidKeyText\" object in MainWindow.glade");

    // Suscribe RefreshLabels al cambio de idioma global.
    language_changed.connect(sigc::mem_fun(*this, &KeyCreateStack::RefreshLabels));
    RefreshLabels();
}

void KeyCreateStack::RefreshLabels() {
    // Etiquetas de campo del formulario.
    name_label_->set_label(Tr().key_create.lbl_key_name);
    ubi_label_->set_label(Tr().key_create.lbl_location);
    commentary_label_->set_label(Tr().key_create.lbl_comments);

    // Botones de acción.
    add_user_button_->set_label(Tr().key_create.btn_add_users);
    add_uid_button_->set_label(Tr().key_create.btn_add_nfc);
    position_picker_button_->set_label(Tr().key_create.btn_pick_position);
    // El botón de confirmar tiene etiqueta distinta según el modo activo.
    if (!edited_key_)
        generate_button_->set_label(Tr().key_create.btn_create);
    else
        generate_button_->set_label(Tr().key_create.btn_edit);
}

// --- Iniciadores públicos ---

void KeyCreateStack::CreateKey(std::shared_ptr<kdb::Person> creator) {
    Reset();
    creator_ = creator;
    add_user_button_->hide();
    RefreshLabels();
}

void KeyCreateStack::KeyEdit(std::shared_ptr<kdb::Key> key) {
    Reset();
    edited_key_ = key;
    key_name_entry_->set_text((std::string)key->name);
    ubi_entry_->set_text((std::string)key->ubi);
    commentary_entry_->set_text((std::string)key->commentary);
    position_ = (int)key->pos;
    uid_text_view_->get_buffer()->set_text((std::string)key->uid);
    add_user_button_->show();
    RefreshLabels();
}

// --- Manejadores de botones ---

void KeyCreateStack::OnGenerateButtonClicked() {
    name_error_label_->set_text("");
    uid_error_label_->set_text("");

    int exclude_id      = edited_key_ ? (int)edited_key_->id : 0;
    std::string uid     = uid_text_view_->get_buffer()->get_text();
    std::string name    = (std::string)key_name_entry_->get_text();
    bool valid          = true;

    // Validar longitud del nombre.
    if (key_name_entry_->get_text_length() < 3) {
        name_error_label_->set_text(Tr().key_create.error_name_too_short);
        valid = false;
    }
    // Validar nombre único (excluyendo la llave actual en modo edición).
    if (litesql::select<kdb::Key>(*db, kdb::Key::Name == name
                                      && kdb::Key::Id != exclude_id).count()) {
        name_error_label_->set_text(Tr().key_create.error_name_exists);
        valid = false;
    }
    // Validar UID único (excluyendo la llave actual en modo edición).
    if (litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid).count() +
        litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid
                                      && kdb::Key::Id != exclude_id).count()) {
        uid_error_label_->set_text(Tr().key_create.error_card_in_use);
        valid = false;
    }
    // En modo creación, validar también que hay posición seleccionada.
    if (!edited_key_ && position_ == 0) {
        position_picker_button_->set_label(Tr().key_create.error_position_invalid);
        valid = false;
    }

    if (!valid) return;

    if (!edited_key_) {
        kdb::Key new_key(*db);
        new_key.name        = name;
        new_key.ubi         = (std::string)ubi_entry_->get_text();
        new_key.commentary  = (std::string)commentary_entry_->get_text();
        new_key.uid         = uid;
        new_key.pos         = position_;
        new_key.active      = true;
        new_key.update();
        auto key_ptr = std::make_shared<kdb::Key>(new_key);
        history::LogKeyCreated(creator_, key_ptr);
        if (creator_) {
            creator_->keys().link(*key_ptr);
            creator_->keepkeys().link(*key_ptr);
        }
        KeyEdit(key_ptr);
    } else {
        edited_key_->name       = name;
        edited_key_->ubi        = (std::string)ubi_entry_->get_text();
        edited_key_->commentary = (std::string)commentary_entry_->get_text();
        edited_key_->uid        = uid;
        edited_key_->pos        = position_;
        edited_key_->update();
    }
}

void KeyCreateStack::OnAddUserButtonClicked() {
    if (edited_key_)
        key_link_user.emit(edited_key_);
}

void KeyCreateStack::OnAddUidButtonClicked() {
    std::string uid = nfcman->NfcDetect(1);
    if (!uid.empty())
        uid_text_view_->get_buffer()->set_text(uid);
    else
        uid_text_view_->get_buffer()->set_text(Tr().key_create.prompt_scan_card);
}

void KeyCreateStack::OnPositionPickerButtonClicked() {
    position_select_requested.emit();
}

void KeyCreateStack::SetPosition(int pos) {
    position_ = pos;
    position_picker_button_->set_label(Tr().key_create.btn_pick_position);
}

// --- Auxiliares privados ---

void KeyCreateStack::Reset() {
    key_name_entry_->set_text("");
    ubi_entry_->set_text("");
    commentary_entry_->set_text("");
    uid_text_view_->get_buffer()->set_text("");

    name_error_label_->set_text("");
    uid_error_label_->set_text("");

    position_   = 0;
    edited_key_ = nullptr;
    creator_    = nullptr;
}
