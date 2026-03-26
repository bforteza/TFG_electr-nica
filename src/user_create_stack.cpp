#include "user_create_stack.h"
#include "dbmanager.hpp"
#include <litesql/selectquery.hpp>
#include "globals.h"
#include <string>

UserCreateStack::UserCreateStack(const Glib::RefPtr<Gtk::Builder>& builder) {
    builder->get_widget("UserNameEntry", username_entry_);
    if (!username_entry_)
        throw std::runtime_error("No \"UserNameEntry\" object in MainWindow.glade");

    builder->get_widget("PasswordEntry", password_entry_);
    if (!password_entry_)
        throw std::runtime_error("No \"PasswordEntry\" object in MainWindow.glade");

    builder->get_widget("RPasswordEntry", repeat_password_entry_);
    if (!repeat_password_entry_)
        throw std::runtime_error("No \"RPasswordEntry\" object in MainWindow.glade");

    builder->get_widget("UserGenerateButton", generate_button_);
    if (!generate_button_)
        throw std::runtime_error("No \"UserGenerateButton\" object in MainWindow.glade");
    generate_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UserCreateStack::OnGenerateButtonClicked));

    builder->get_widget("AddUidUserButton", add_uid_button_);
    if (!add_uid_button_)
        throw std::runtime_error("No \"AddUidUserButton\" object in MainWindow.glade");
    add_uid_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UserCreateStack::OnAddUidButtonClicked));

    builder->get_widget("AddKeyButton", add_key_button_);
    if (!add_key_button_)
        throw std::runtime_error("No \"AddKeyButton\" object in MainWindow.glade");
    add_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UserCreateStack::OnAddKeyButtonClicked));

    builder->get_widget("A0RadioButton", radio_level0_);
    if (!radio_level0_)
        throw std::runtime_error("No \"A0RadioButton\" object in MainWindow.glade");

    builder->get_widget("A1RadioButton", radio_level1_);
    if (!radio_level1_)
        throw std::runtime_error("No \"A1RadioButton\" object in MainWindow.glade");

    builder->get_widget("A2RadioButton", radio_admin_);
    if (!radio_admin_)
        throw std::runtime_error("No \"A2RadioButton\" object in MainWindow.glade");

    auto radio_group = radio_level0_->get_group();
    radio_level1_->set_group(radio_group);
    radio_admin_->set_group(radio_group);

    builder->get_widget("UserCreateUsernameLabel", username_label_);
    if (!username_label_)
        throw std::runtime_error("No \"UserCreateUsernameLabel\" object in MainWindow.glade");

    builder->get_widget("UserCreatePasswordLabel", password_label_);
    if (!password_label_)
        throw std::runtime_error("No \"UserCreatePasswordLabel\" object in MainWindow.glade");

    builder->get_widget("UserCreateRPasswordLabel", repeat_password_label_);
    if (!repeat_password_label_)
        throw std::runtime_error("No \"UserCreateRPasswordLabel\" object in MainWindow.glade");

    builder->get_widget("UserCreateAccessLabel", access_level_label_);
    if (!access_level_label_)
        throw std::runtime_error("No \"UserCreateAccessLabel\" object in MainWindow.glade");

    builder->get_widget("UserNameErrorLabel", username_error_label_);
    if (!username_error_label_)
        throw std::runtime_error("No \"UserNameErrorLabel\" object in MainWindow.glade");

    builder->get_widget("PasswordErrorLabel", password_error_label_);
    if (!password_error_label_)
        throw std::runtime_error("No \"PasswordErrorLabel\" object in MainWindow.glade");

    builder->get_widget("RPasswordErrorLabel", repeat_password_error_label_);
    if (!repeat_password_error_label_)
        throw std::runtime_error("No \"RPasswordErrorLabel\" object in MainWindow.glade");

    builder->get_widget("NfcErrorLabel", nfc_error_label_);
    if (!nfc_error_label_)
        throw std::runtime_error("No \"NfcErrorLabel\" object in MainWindow.glade");

    builder->get_widget("UserUidText", uid_text_view_);
    if (!uid_text_view_)
        throw std::runtime_error("No \"UserUidText\" object in MainWindow.glade");

    // Suscribe RefreshLabels al cambio de idioma global.
    language_changed.connect(sigc::mem_fun(*this, &UserCreateStack::RefreshLabels));
    RefreshLabels();
}

void UserCreateStack::RefreshLabels() {
    // Etiquetas de campo del formulario.
    username_label_->set_label(Tr().user_create.lbl_username);
    password_label_->set_label(Tr().user_create.lbl_password);
    repeat_password_label_->set_label(Tr().user_create.lbl_repeat_password);
    access_level_label_->set_label(Tr().user_create.lbl_access_level);

    // Radio buttons de nivel de acceso.
    radio_level0_->set_label(Tr().user_create.radio_level_0);
    radio_level1_->set_label(Tr().user_create.radio_level_1);
    radio_admin_->set_label(Tr().user_create.radio_admin);

    // Botones de acción.
    add_key_button_->set_label(Tr().user_create.btn_add_key);
    add_uid_button_->set_label(Tr().user_create.btn_add_nfc);
    // El botón de confirmar tiene etiqueta distinta según el modo activo.
    if (!edited_user_)
        generate_button_->set_label(Tr().user_create.btn_create);
    else
        generate_button_->set_label(Tr().user_create.btn_edit);
}

// --- Iniciadores públicos ---

void UserCreateStack::CreateUser() {
    Reset();
    add_key_button_->hide();
    RefreshLabels();
}

void UserCreateStack::UserEdit(std::shared_ptr<kdb::Person> person) {
    Reset();
    edited_user_ = person;
    username_entry_->set_text((std::string)person->name);
    password_entry_->set_text((std::string)person->password);
    repeat_password_entry_->set_text((std::string)person->password);
    uid_text_view_->get_buffer()->set_text((std::string)person->uid);
    int lv = (int)person->level;
    if (lv >= 2)      radio_admin_->set_active(true);
    else if (lv == 1) radio_level1_->set_active(true);
    else              radio_level0_->set_active(true);
    add_key_button_->show();
    RefreshLabels();
}

void UserCreateStack::SelfEdit(std::shared_ptr<kdb::Person> person) {
    UserEdit(person);          // configura el formulario (Reset() pone self_edit_mode_=false)
    self_edit_mode_ = true;    // activar DESPUÉS para que OnGenerateButtonClicked lo vea
    add_key_button_->hide();
    access_level_label_->hide();
    radio_level0_->hide();
    radio_level1_->hide();
    radio_admin_->hide();
}

// --- Manejadores de botones ---

void UserCreateStack::OnGenerateButtonClicked() {
    username_error_label_->set_text("");
    password_error_label_->set_text("");
    repeat_password_error_label_->set_text("");
    bool valid = true;

    auto username = username_entry_->get_text();
    auto password = password_entry_->get_text();
    std::string uid = uid_text_view_->get_buffer()->get_text();

    // Validaciones comunes a ambos modos.
    if (username_entry_->get_text_length() < 2) {
        username_error_label_->set_text(Tr().user_create.error_username_too_short);
        valid = false;
    }
    if (password_entry_->get_text_length() < 2) {
        password_error_label_->set_text(Tr().user_create.error_password_too_short);
        valid = false;
    }
    if (repeat_password_entry_->get_text() != password) {
        repeat_password_error_label_->set_text(Tr().user_create.error_password_mismatch);
        valid = false;
    }

    int exclude_id = edited_user_ ? (int)edited_user_->id : 0;

    if (litesql::select<kdb::Person>(*db, kdb::Person::Name == username
                                         && kdb::Person::Id != exclude_id).count()) {
        username_error_label_->set_text(Tr().user_create.error_username_exists);
        valid = false;
    }
    if (litesql::select<kdb::Person>(*db, kdb::Person::Password == password
                                         && kdb::Person::Id != exclude_id).count()) {
        password_error_label_->set_text(Tr().user_create.error_password_exists);
        valid = false;
    }
    if (litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid
                                         && kdb::Person::Id != exclude_id).count() +
        litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid).count()) {
        nfc_error_label_->set_text(Tr().user_create.error_card_in_use);
        valid = false;
    }

    if (!valid) return;

    if (!edited_user_) {
        kdb::Person new_person(*db);
        new_person.uid      = uid;
        new_person.name     = (std::string)username;
        new_person.password = (std::string)password;
        new_person.level    = radio_admin_->get_active() ? 2 : (radio_level1_->get_active() ? 1 : 0);
        new_person.update();
        UserEdit(std::make_shared<kdb::Person>(new_person));
    } else {
        edited_user_->uid      = uid;
        edited_user_->name     = (std::string)username;
        edited_user_->password = (std::string)password;
        edited_user_->level    = radio_admin_->get_active() ? 2 : (radio_level1_->get_active() ? 1 : 0);
        edited_user_->update();
        if (self_edit_mode_)
            SelfEdit(edited_user_);
        else
            UserEdit(edited_user_);
    }
}

void UserCreateStack::OnAddUidButtonClicked() {
    std::string uid = nfcman->NfcDetect(1);
    if (!uid.empty())
        uid_text_view_->get_buffer()->set_text(uid);
    else
        uid_text_view_->get_buffer()->set_text(Tr().user_create.prompt_scan_card);
}

void UserCreateStack::OnAddKeyButtonClicked() {
    if (edited_user_)
        user_link_key.emit(edited_user_);
}

// --- Auxiliares privados ---

void UserCreateStack::Reset() {
    username_error_label_->set_text("");
    password_error_label_->set_text("");
    repeat_password_error_label_->set_text("");
    nfc_error_label_->set_text("");

    username_entry_->set_text("");
    password_entry_->set_text("");
    repeat_password_entry_->set_text("");
    uid_text_view_->get_buffer()->set_text("");

    edited_user_    = nullptr;
    self_edit_mode_ = false;

    // Restaurar visibilidad de controles que SelfEdit oculta.
    access_level_label_->show();
    radio_level0_->show();
    radio_level1_->show();
    radio_admin_->show();
}
