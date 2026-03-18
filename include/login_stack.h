#ifndef LOGIN_STACK_H
#define LOGIN_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <string>
#include "dbmanager.hpp"
#include "translations.h"

// Gestiona la pantalla de inicio de sesión.
// El usuario puede identificarse mediante contraseña o tarjeta NFC.
// Emite user_logged o key_logged según el dispositivo detectado.
class LoginStack
{
public:
    LoginStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Señal emitida cuando se identifica un usuario válido.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_logged;

    // Señal emitida cuando se identifica una llave válida.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_logged;

    // Limpia el mensaje de error e inicia el polling NFC.
    void Start();

private:
    // Campo de entrada de contraseña/identificación manual.
    Gtk::Entry* password_entry_;

    // Etiqueta para mostrar mensajes de error al usuario.
    Gtk::Label* error_label_;

    // Etiqueta con el título de la pantalla ("Introdueix credencials").
    Gtk::Label* title_label_;

    // Etiqueta con el texto descriptivo sobre cómo identificarse.
    Gtk::Label* desc_label_;

    // Botones de selección de idioma.
    Gtk::Button* lang_ca_button_;
    Gtk::Button* lang_es_button_;
    Gtk::Button* lang_en_button_;

    // Actualiza los textos de la pantalla al idioma activo.
    void RefreshLabels();

    // Llamado cuando el usuario pulsa Enter en el campo de contraseña.
    void OnPasswordEntered();

    // Llamado cuando el NfcManager detecta un dispositivo.
    void OnNfcDetected(const std::string& uid);

    // Cambia el idioma activo y notifica a todos los stacks.
    void OnLangSelected(Language lang);
};

#endif // LOGIN_STACK_H
