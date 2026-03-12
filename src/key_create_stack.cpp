#include "key_create_stack.h"
#include "db_schema.hpp"
#include <litesql/selectquery.hpp>
#include "globals.h"
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

    builder->get_widget("PositionEntry", position_entry_);
    if (!position_entry_)
        throw std::runtime_error("No \"PositionEntry\" object in MainWindow.glade");

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

    builder->get_widget("KeyCreateNameLabel", name_label_);
    if (!name_label_)
        throw std::runtime_error("No \"KeyCreateNameLabel\" object in MainWindow.glade");

    builder->get_widget("KeyCreateUbiLabel", ubi_label_);
    if (!ubi_label_)
        throw std::runtime_error("No \"KeyCreateUbiLabel\" object in MainWindow.glade");

    builder->get_widget("KeyCreateCommentaryLabel", commentary_label_);
    if (!commentary_label_)
        throw std::runtime_error("No \"KeyCreateCommentaryLabel\" object in MainWindow.glade");

    builder->get_widget("KeyCreatePositionLabel", position_label_);
    if (!position_label_)
        throw std::runtime_error("No \"KeyCreatePositionLabel\" object in MainWindow.glade");

    builder->get_widget("KeyNameErrorLabel", name_error_label_);
    if (!name_error_label_)
        throw std::runtime_error("No \"KeyNameErrorLabel\" object in MainWindow.glade");

    builder->get_widget("PositionErrorLabel", position_error_label_);
    if (!position_error_label_)
        throw std::runtime_error("No \"PositionErrorLabel\" object in MainWindow.glade");

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
    position_label_->set_label(Tr().key_create.lbl_position);

    // Botones de acción.
    add_user_button_->set_label(Tr().key_create.btn_add_users);
    add_uid_button_->set_label(Tr().key_create.btn_add_nfc);
    // El botón de confirmar tiene etiqueta distinta según el modo activo.
    if (create_mode_)
        generate_button_->set_label(Tr().key_create.btn_create);
    else if (edit_mode_)
        generate_button_->set_label(Tr().key_create.btn_edit);
}

// --- Iniciadores públicos ---

void KeyCreateStack::CreateKey(std::shared_ptr<kdb::Person> creator) {
    Reset();
    creator_     = creator;
    create_mode_ = true;
    RefreshLabels();
}

void KeyCreateStack::KeyEdit(std::shared_ptr<kdb::Key> key) {
    Reset();
    edited_key_ = key;
    key_name_entry_->set_text((std::string)key->name);
    ubi_entry_->set_text((std::string)key->ubi);
    commentary_entry_->set_text((std::string)key->commentary);
    position_entry_->set_text(PosToString(key->pos));
    uid_text_view_->get_buffer()->set_text((std::string)key->uid);
    edit_mode_ = true;
    RefreshLabels();
}

// --- Manejadores de botones ---

void KeyCreateStack::OnGenerateButtonClicked() {
    name_error_label_->set_text("");
    bool valid = true;

    if (create_mode_) {
        // Validar longitud del nombre.
        if (key_name_entry_->get_text_length() < 3) {
            name_error_label_->set_text(Tr().key_create.error_name_too_short);
            valid = false;
        }
        // Validar nombre único.
        if (litesql::select<kdb::Key>(*db, kdb::Key::Name == key_name_entry_->get_text()).count()) {
            name_error_label_->set_text(Tr().key_create.error_name_exists);
            valid = false;
        }
        // Validar UID único (no usado ni en persona ni en llave).
        std::string uid = uid_text_view_->get_buffer()->get_text();
        if (litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid).count()) {
            uid_error_label_->set_text(Tr().key_create.error_card_in_use);
            valid = false;
        }
        // Validar posición.
        std::string pos_str = position_entry_->get_text();
        if (PosFromString(pos_str)) {
            if (litesql::select<kdb::Key>(*db, kdb::Key::Pos == PosFromString(pos_str)).count()) {
                std::string msg = Tr().key_create.error_position_unavailable;
                for (auto& k : litesql::select<kdb::Key>(*db, kdb::Key::Pos > 0)
                                    .orderBy(kdb::Key::Pos).all())
                    msg += " " + PosToString((int)k.pos);
                position_error_label_->set_text(msg);
                valid = false;
            }
        } else {
            position_error_label_->set_text(Tr().key_create.error_position_invalid);
            valid = false;
        }

        if (valid) {
            kdb::Key new_key(*db);
            new_key.name        = (std::string)key_name_entry_->get_text();
            new_key.ubi         = (std::string)ubi_entry_->get_text();
            new_key.commentary  = (std::string)commentary_entry_->get_text();
            new_key.uid         = (std::string)uid_text_view_->get_buffer()->get_text();
            new_key.pos         = (int)PosFromString(position_entry_->get_text());
            new_key.update();
            auto key_ptr = std::make_shared<kdb::Key>(new_key);
            if (creator_) {
                creator_->keys().link(*key_ptr);
                creator_->keepkeys().link(*key_ptr);
            }
            // Entra en modo edición con la llave recién creada.
            KeyEdit(key_ptr);
        }

    } else if (edit_mode_) {
        // Validar longitud del nombre.
        if (key_name_entry_->get_text_length() < 3) {
            name_error_label_->set_text(Tr().key_create.error_name_too_short);
            valid = false;
        }
        // Validar nombre único (excluyendo la llave actual).
        if (litesql::select<kdb::Key>(*db, kdb::Key::Name == key_name_entry_->get_text()
                                          && kdb::Key::Id != edited_key_->id).count()) {
            name_error_label_->set_text(Tr().key_create.error_name_exists);
            valid = false;
        }
        // Validar UID único (excluyendo la llave actual).
        std::string uid = uid_text_view_->get_buffer()->get_text();
        if (litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid
                                          && kdb::Key::Id != edited_key_->id).count()) {
            uid_error_label_->set_text(Tr().key_create.error_card_in_use);
            valid = false;
        }
        // Validar posición (excluyendo la posición actual de la misma llave).
        std::string pos_str = position_entry_->get_text();
        if (PosFromString(pos_str)) {
            if (litesql::select<kdb::Key>(*db, kdb::Key::Pos == PosFromString(pos_str)
                                              && kdb::Key::Id != edited_key_->id).count()) {
                std::string msg = Tr().key_create.error_position_unavailable;
                for (auto& k : litesql::select<kdb::Key>(*db, kdb::Key::Pos > 0)
                                    .orderBy(kdb::Key::Pos).all())
                    msg += " " + PosToString((int)k.pos);
                position_error_label_->set_text(msg);
                valid = false;
            }
        } else {
            position_error_label_->set_text(Tr().key_create.error_position_invalid);
            valid = false;
        }

        if (valid) {
            edited_key_->name       = (std::string)key_name_entry_->get_text();
            edited_key_->ubi        = (std::string)ubi_entry_->get_text();
            edited_key_->commentary = (std::string)commentary_entry_->get_text();
            edited_key_->uid        = (std::string)uid_text_view_->get_buffer()->get_text();
            edited_key_->pos        = (int)PosFromString(position_entry_->get_text());
            edited_key_->update();
        }
    }
}

void KeyCreateStack::OnAddUserButtonClicked() {
    // TODO: abrir vista de selección de usuarios para vincularlos a la llave.
}

void KeyCreateStack::OnAddUidButtonClicked() {
    std::string uid = nfcman->NfcDetect(1);
    if (!uid.empty())
        uid_text_view_->get_buffer()->set_text(uid);
    else
        uid_text_view_->get_buffer()->set_text(Tr().key_create.prompt_scan_card);
}

// --- Auxiliares privados ---

void KeyCreateStack::Reset() {
    key_name_entry_->set_text("");
    ubi_entry_->set_text("");
    commentary_entry_->set_text("");
    position_entry_->set_text("");
    uid_text_view_->get_buffer()->set_text("");

    name_error_label_->set_text("");
    position_error_label_->set_text("");
    uid_error_label_->set_text("");

    edit_mode_   = false;
    create_mode_ = false;
    edited_key_  = nullptr;
    creator_     = nullptr;
}
