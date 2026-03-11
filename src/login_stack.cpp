#include "login_stack.h"
#include "globals.h"
#include "translations.h"
#include "sound_manager.h"
#include <sigc++/adaptors/bind.h>

LoginStack::LoginStack(const Glib::RefPtr<Gtk::Builder>& builder)
{
    builder->get_widget("IN_IDEN", password_entry_);
    if (!password_entry_) {
        throw std::runtime_error("No \"IN_IDEN\" object in MainWindow.glade");
    }
    password_entry_->signal_activate().connect(
        sigc::mem_fun(*this, &LoginStack::OnPasswordEntered));

    builder->get_widget("LogErrorLabel", error_label_);
    if (!error_label_) {
        throw std::runtime_error("No \"LogErrorLabel\" object in MainWindow.glade");
    }

    builder->get_widget("LoginTitleLabel", title_label_);
    if (!title_label_) {
        throw std::runtime_error("No \"LoginTitleLabel\" object in MainWindow.glade");
    }

    builder->get_widget("LoginDescLabel", desc_label_);
    if (!desc_label_) {
        throw std::runtime_error("No \"LoginDescLabel\" object in MainWindow.glade");
    }

    builder->get_widget("LangCaButton", lang_ca_button_);
    if (!lang_ca_button_)
        throw std::runtime_error("No \"LangCaButton\" object in MainWindow.glade");
    lang_ca_button_->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &LoginStack::OnLangSelected), Language::kCatalan));

    builder->get_widget("LangEsButton", lang_es_button_);
    if (!lang_es_button_)
        throw std::runtime_error("No \"LangEsButton\" object in MainWindow.glade");
    lang_es_button_->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &LoginStack::OnLangSelected), Language::kSpanish));

    builder->get_widget("LangEnButton", lang_en_button_);
    if (!lang_en_button_)
        throw std::runtime_error("No \"LangEnButton\" object in MainWindow.glade");
    lang_en_button_->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &LoginStack::OnLangSelected), Language::kEnglish));

    // Suscribe RefreshLabels a la señal global de cambio de idioma.
    language_changed.connect(sigc::mem_fun(*this, &LoginStack::RefreshLabels));

    // Conecta el dispatcher del NfcManager para recibir notificaciones
    // en el hilo principal de GTK cuando se detecta un dispositivo.
    nfcman->dispatcher.connect([&]() {
        this->OnNfcDetected(nfcman->uid);
    });
}

void LoginStack::Start()
{
    error_label_->set_text("");
    nfcman->StartPolling();
}

void LoginStack::RefreshLabels()
{
    title_label_->set_text(Tr().login.title);
    desc_label_->set_text(Tr().login.description);
}

void LoginStack::OnLangSelected(Language lang)
{
    current_language = lang;
    language_changed.emit();
}

void LoginStack::OnPasswordEntered()
{
    std::string password = password_entry_->get_text();
    try {
        user_logged.emit(std::make_shared<kdb::Person>(
            litesql::select<kdb::Person>(*db, kdb::Person::Password == password).one()));
        error_label_->set_text("");
        SoundManager::Play(SoundEvent::kLoginOk);
    } catch (...) {
        error_label_->set_text(Tr().login.error_invalid_credentials);
        SoundManager::Play(SoundEvent::kLoginError);
    }
}

void LoginStack::OnNfcDetected(const std::string& uid)
{
    // Comprueba si el UID pertenece a un usuario o a una llave activa.
    // Se usa count() para evitar excepciones si no hay resultados.
    int person_count = litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid).count();
    int key_count    = litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid
                                                      && kdb::Key::Active == 1).count();

    if (person_count + key_count == 0) {
        error_label_->set_text(Tr().login.error_device_not_found);
        SoundManager::Play(SoundEvent::kLoginError);
    } else if (person_count + key_count > 1) {
        // Dos dispositivos con el mismo UID indica un error de configuración en la BD.
        error_label_->set_text(Tr().login.error_duplicate_uid);
        SoundManager::Play(SoundEvent::kLoginError);
    } else if (person_count) {
        nfcman->StopPolling();
        SoundManager::Play(SoundEvent::kLoginOk);
        user_logged.emit(std::make_shared<kdb::Person>(
            litesql::select<kdb::Person>(*db, kdb::Person::Uid == uid).one()));
    } else {
        nfcman->StopPolling();
        SoundManager::Play(SoundEvent::kKeyReturn);
        key_logged.emit(std::make_shared<kdb::Key>(
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == uid
                                          && kdb::Key::Active == 1).one()));
    }
}
