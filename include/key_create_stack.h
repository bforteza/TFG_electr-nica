#ifndef KEY_CREATE_STACK_H
#define KEY_CREATE_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <sigc++/signal.h>
#include <string>
#include "dbmanager.hpp"
#include "translations.h"

// Sub-vista de creación y edición de llaves dentro de HomeInnerStack.
// En modo creación (CreateKey()) valida y persiste una llave nueva.
// En modo edición (KeyEdit()) carga los datos de una llave existente y los actualiza.
class KeyCreateStack {
public:
    KeyCreateStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Inicializa el formulario en modo creación (campos en blanco).
    // El creador queda vinculado automáticamente a la llave (acceso + portador).
    void CreateKey(std::shared_ptr<kdb::Person> creator);

    // Inicializa el formulario en modo edición con los datos de la llave dada.
    void KeyEdit(std::shared_ptr<kdb::Key> key);

    // Modo recuperación: carga datos de una llave inactiva para validar antes de reactivarla.
    // Al confirmar, además de guardar los cambios, pone active=true.
    void RecoverKey(std::shared_ptr<kdb::Key> key);

    // Rellena el campo de posición con la posición seleccionada en el picker (1-based).
    void SetPosition(int pos);

    // Emitida cuando el usuario pulsa el botón de selección de posición.
    sigc::signal<void> position_select_requested;

    // Emitida cuando el usuario pulsa "Añadir usuario" en modo edición.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_link_user;

private:
    // Campos de entrada del formulario.
    Gtk::Entry* key_name_entry_;
    Gtk::Entry* ubi_entry_;
    Gtk::Entry* commentary_entry_;

    // Botón para confirmar la creación o edición de la llave.
    Gtk::Button* generate_button_;

    // Toggle para marcar la llave como pública (accesible por todos los usuarios).
    Gtk::CheckButton* public_check_button_;

    // Botón para vincular usuarios a la llave.
    Gtk::Button* add_user_button_;

    // Botón para capturar el UID NFC de la llave.
    Gtk::Button* add_uid_button_;

    // Botón para abrir el selector visual de posición.
    Gtk::Button* position_picker_button_;

    // Etiquetas descriptivas de cada campo del formulario.
    Gtk::Label* name_label_;
    Gtk::Label* ubi_label_;
    Gtk::Label* commentary_label_;

    // Etiquetas de error asociadas a cada campo.
    Gtk::Label* name_error_label_;
    Gtk::Label* uid_error_label_;

    // Posición seleccionada mediante el picker (0 = sin seleccionar).
    int position_ = 0;

    // Área de texto que muestra el UID NFC capturado.
    Gtk::TextView* uid_text_view_;

    // Llave que se está editando actualmente (nullptr en modo creación).
    std::shared_ptr<kdb::Key> edited_key_;

    // true cuando estamos en modo recuperación (llave inactiva que se va a reactivar).
    bool recover_mode_ = false;

    // Usuario que inició la creación; se vincula a la llave al crearla.
    std::shared_ptr<kdb::Person> creator_;

    // Actualiza los textos de botones al idioma activo.
    void RefreshLabels();

    // Limpia el formulario y reinicia los modos.
    void Reset();

    // Valida los campos y crea/actualiza la llave en la base de datos.
    void OnGenerateButtonClicked();

    // Abre la vista de selección de usuarios para vincularlos (pendiente de implementar).
    void OnAddUserButtonClicked();

    // Lanza una lectura NFC puntual y vuelca el UID en uid_text_view_.
    void OnAddUidButtonClicked();

    // Emite position_select_requested para abrir el selector de posición.
    void OnPositionPickerButtonClicked();
};

#endif // KEY_CREATE_STACK_H
