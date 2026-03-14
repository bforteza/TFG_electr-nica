#ifndef USERS_VIEW_STACK_H
#define USERS_VIEW_STACK_H

#include <gtkmm/treemodelsort.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>

#include "models.h"
#include "db_schema.hpp"
#include "translations.h"

// Sub-vista de lista de usuarios dentro de HomeInnerStack.
// Permite ver, editar, vincular llaves y seleccionar usuarios.
// Se usa también en modo selección (select()) para flujos de link/unlink.
class UsersViewStack {
public:
    UsersViewStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Señal emitida cuando el usuario selecciona una persona en modo selección.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_selected;

    // Señal para iniciar flujo de vincular una llave al usuario seleccionado.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_link;

    // Señal para iniciar flujo de desvincular una llave del usuario seleccionado.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_unlink;

    // Señal para editar el usuario seleccionado.
    sigc::signal<void, std::shared_ptr<kdb::Person>> user_edit;

    // Señal para mostrar las llaves asignadas al usuario seleccionado.
    sigc::signal<void, std::vector<kdb::Key>> keys_view;

    // Muestra la lista de usuarios en modo normal.
    // access: nivel del usuario logueado; oculta botones de admin si access < 2.
    void view(std::vector<kdb::Person> users, int access = 2);

    // Muestra la lista de usuarios en modo selección:
    // oculta botones de edición y muestra el botón "Seleccionar".
    void select(std::vector<kdb::Person> users);

    // Actualiza los textos de botones y cabeceras de columnas al idioma activo.
    void RefreshLabels();

private:
    // Botón para vincular una llave al usuario seleccionado.
    Gtk::Button* add_key_button_;

    // Botón para desvincular una llave del usuario seleccionado.
    Gtk::Button* remove_key_button_;

    // Botón visible solo en modo selección; emite user_selected.
    Gtk::Button* select_user_button_;

    // Botón para editar los datos del usuario seleccionado.
    Gtk::Button* edit_button_;

    // Botón para ver las llaves asignadas al usuario seleccionado.
    Gtk::Button* view_keys_button_;

    // Widget de lista para mostrar los usuarios.
    Gtk::TreeView* users_tree_view_;

    // Columnas del TreeView (guardadas para modificar títulos).
    Gtk::TreeViewColumn* id_column_;
    Gtk::TreeViewColumn* name_column_;
    Gtk::TreeViewColumn* password_column_;
    Gtk::TreeViewColumn* uid_column_;

    // Modelo de datos enlazado al TreeView.
    Glib::RefPtr<Gtk::ListStore> tree_model_;

    // Definición de columnas del modelo.
    ModelColumns columns_;

    // Nivel de acceso del usuario logueado (para mostrar/ocultar botones de admin).
    int access_ = 2;

    // Lista de usuarios actualmente mostrada en el TreeView.
    std::vector<kdb::Person> current_users_;

    // Recarga el TreeView con el contenido de current_users_.
    void Refresh();

    // Devuelve el ID de la fila seleccionada, o 0 si no hay selección.
    int GetSelectionId();

    // Devuelve un puntero al usuario seleccionado, o nullptr si no hay selección.
    std::shared_ptr<kdb::Person> GetSelectedPerson();

    // Manejadores de los botones de acción.
    void OnAddKeyButtonClicked();
    void OnRemoveKeyButtonClicked();
    void OnViewKeysButtonClicked();
    void OnEditButtonClicked();
    void OnSelectUserButtonClicked();
};

#endif // USERS_VIEW_STACK_H
