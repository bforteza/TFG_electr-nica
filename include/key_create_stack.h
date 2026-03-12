#ifndef KEY_CREATE_STACK_H
#define KEY_CREATE_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <string>
#include "db_schema.hpp"
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

    // Actualiza los textos de botones al idioma activo.
    void RefreshLabels();

private:
    // Campos de entrada del formulario.
    Gtk::Entry* key_name_entry_;
    Gtk::Entry* ubi_entry_;
    Gtk::Entry* commentary_entry_;
    Gtk::Entry* position_entry_;

    // Botón para confirmar la creación o edición de la llave.
    Gtk::Button* generate_button_;

    // Botón para vincular usuarios a la llave (pendiente de implementar).
    Gtk::Button* add_user_button_;

    // Botón para capturar el UID NFC de la llave.
    Gtk::Button* add_uid_button_;

    // Etiquetas descriptivas de cada campo del formulario.
    Gtk::Label* name_label_;
    Gtk::Label* ubi_label_;
    Gtk::Label* commentary_label_;
    Gtk::Label* position_label_;

    // Etiquetas de error asociadas a cada campo.
    Gtk::Label* name_error_label_;
    Gtk::Label* position_error_label_;
    Gtk::Label* uid_error_label_;

    // Área de texto que muestra el UID NFC capturado.
    Gtk::TextView* uid_text_view_;

    // Indicadores de modo activo; solo uno puede ser true a la vez.
    bool edit_mode_   = false;
    bool create_mode_ = true;

    // Llave que se está editando actualmente (nullptr en modo creación).
    std::shared_ptr<kdb::Key> edited_key_;

    // Usuario que inició la creación; se vincula a la llave al crearla.
    std::shared_ptr<kdb::Person> creator_;

    // Limpia el formulario y reinicia los modos.
    void Reset();

    // Valida los campos y crea/actualiza la llave en la base de datos.
    void OnGenerateButtonClicked();

    // Abre la vista de selección de usuarios para vincularlos (pendiente de implementar).
    void OnAddUserButtonClicked();

    // Lanza una lectura NFC puntual y vuelca el UID en uid_text_view_.
    void OnAddUidButtonClicked();
};

#endif // KEY_CREATE_STACK_H
