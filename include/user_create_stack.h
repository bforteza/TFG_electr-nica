#ifndef USER_CREATE_STACK_H
#define USER_CREATE_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/radiobutton.h>
#include "dbmanager.hpp"
#include "translations.h"

// Sub-vista de creación y edición de usuarios dentro de HomeInnerStack.
// En modo creación (CreateUser()) valida y persiste un usuario nuevo.
// En modo edición (UserEdit()) carga los datos de un usuario existente y los actualiza.
class UserCreateStack {
public:
    UserCreateStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Inicializa el formulario en modo edición con los datos del usuario dado.
    void UserEdit(std::shared_ptr<kdb::Person> person);

    // Inicializa el formulario en modo creación (campos en blanco).
    void CreateUser();

    // Emitida cuando el usuario pulsa "Añadir llave" en modo edición.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_link_key;

    // Actualiza los textos de etiquetas y botones al idioma activo.
    void RefreshLabels();

private:
    // Usuario que se está editando actualmente (nullptr en modo creación).
    std::shared_ptr<kdb::Person> edited_user_;

    // Campos de entrada del formulario.
    Gtk::Entry* username_entry_;
    Gtk::Entry* password_entry_;
    Gtk::Entry* repeat_password_entry_;

    // Botón para confirmar la creación o edición del usuario.
    Gtk::Button* generate_button_;

    // Botón para vincular llaves al usuario (pendiente de implementar).
    Gtk::Button* add_key_button_;

    // Botón para capturar el UID NFC del usuario.
    Gtk::Button* add_uid_button_;

    // Radio buttons para seleccionar el nivel de acceso.
    Gtk::RadioButton* radio_level0_;   // Sin acceso especial (a1=false, a2=false)
    Gtk::RadioButton* radio_level1_;   // Acceso a gestión de llaves (a1=true)
    Gtk::RadioButton* radio_admin_;    // Administrador completo (a2=true)

    // Etiquetas descriptivas de cada campo del formulario.
    Gtk::Label* username_label_;
    Gtk::Label* password_label_;
    Gtk::Label* repeat_password_label_;
    Gtk::Label* access_level_label_;

    // Etiquetas de error asociadas a cada campo.
    Gtk::Label* username_error_label_;
    Gtk::Label* password_error_label_;
    Gtk::Label* repeat_password_error_label_;
    Gtk::Label* nfc_error_label_;

    // Área de texto que muestra el UID NFC capturado.
    Gtk::TextView* uid_text_view_;

    // Limpia el formulario y reinicia los modos.
    void Reset();

    // Valida los campos y crea/actualiza el usuario en la base de datos.
    void OnGenerateButtonClicked();

    // Lanza una lectura NFC puntual y vuelca el UID en uid_text_view_.
    void OnAddUidButtonClicked();

    // Emite user_link_key con el usuario en edición.
    void OnAddKeyButtonClicked();
};

#endif // USER_CREATE_STACK_H
