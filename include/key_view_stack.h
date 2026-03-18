#ifndef KEY_VIEW_STACK_H
#define KEY_VIEW_STACK_H

#include <gtkmm/treemodelsort.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include "models.h"
#include "dbmanager.hpp"
#include "translations.h"

// Sub-vista de lista de llaves dentro de HomeInnerStack.
// Permite ver, editar, vincular usuarios y registrar la recogida de llaves.
// Se usa también en modo selección (select()) para flujos de link/unlink.
class KeyViewStack {
public:
    KeyViewStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Señal emitida cuando el usuario selecciona una o más llaves en modo selección.
    sigc::signal<void, std::vector<std::shared_ptr<kdb::Key>>> key_selected;

    // Señal para mostrar la lista de usuarios que tienen acceso a una llave.
    sigc::signal<void, std::vector<kdb::Person>> users_view;

    // Señal para iniciar flujo de vincular un usuario a la llave seleccionada.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_link;

    // Señal para iniciar flujo de desvincular un usuario de la llave seleccionada.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_unlink;

    // Señal para editar la llave seleccionada.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_edit;

    // Señal para desactivar la llave seleccionada (HomeStack comprueba permisos y keeper).
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_delete;

    // Señal para registrar que el usuario identificado ha recogido la llave.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_kept;

    // Señal para ver el historial de la llave seleccionada.
    sigc::signal<void, std::shared_ptr<kdb::Key>> key_history;

    // Muestra una lista de llaves. access controla qué botones son visibles.
    void view(std::vector<kdb::Key> keys, int access = 2);

    // Muestra una lista de llaves en modo selección:
    // oculta botones de edición y muestra el botón "Seleccionar".
    void select(std::vector<kdb::Key> keys);

    // Actualiza los textos de botones y cabeceras de columnas al idioma activo.
    void RefreshLabels();

private:
    // Botón para dar acceso a un usuario sobre la llave seleccionada.
    Gtk::Button* add_user_button_;

    // Botón para quitar acceso a un usuario sobre la llave seleccionada.
    Gtk::Button* remove_user_button_;

    // Botón visible solo en modo selección; emite key_selected.
    Gtk::Button* select_key_button_;

    // Botón para editar los datos de la llave seleccionada.
    Gtk::Button* edit_key_button_;

    // Botón para registrar la recogida/devolución de la llave seleccionada.
    Gtk::Button* keep_key_button_;

    // Botón para ver la lista de usuarios con acceso a la llave seleccionada.
    Gtk::Button* view_users_button_;

    // Botón para eliminar la llave seleccionada (pendiente de implementar).
    Gtk::Button* delete_key_button_;

    // Botón para ver el historial de la llave seleccionada.
    Gtk::Button* history_key_button_;

    // Widget de lista para mostrar las llaves.
    Gtk::TreeView* keys_tree_view_;

    // Columnas del TreeView (guardadas para modificar visibilidad y títulos).
    Gtk::TreeViewColumn* id_column_;
    Gtk::TreeViewColumn* name_column_;
    Gtk::TreeViewColumn* ubi_column_;
    Gtk::TreeViewColumn* commentary_column_;
    Gtk::TreeViewColumn* pos_column_;
    Gtk::TreeViewColumn* active_column_;
    Gtk::TreeViewColumn* keeper_column_;

    // Conexión del handler de toggle en modo selección.
    sigc::connection toggle_conn_;

    // Modelo de datos enlazado al TreeView.
    Glib::RefPtr<Gtk::ListStore> tree_model_;

    // Definición de columnas del modelo.
    KeyModelColumns columns_;

    // Lista de llaves actualmente mostrada en el TreeView.
    std::vector<kdb::Key> current_keys_;

    // Recarga el TreeView con el contenido de current_keys_.
    void Refresh();

    // Devuelve un puntero a la llave seleccionada, o nullptr si no hay selección.
    std::shared_ptr<kdb::Key> GetSelectedKey();

    // Devuelve todas las llaves seleccionadas (modo MULTIPLE).
    std::vector<std::shared_ptr<kdb::Key>> GetSelectedKeys();

    // Muestra u oculta botones según el nivel de acceso.
    void Configure(int access);

    // Manejadores de los botones de acción.
    void OnSelectKeyButtonClicked();
    void OnEditKeyButtonClicked();
    void OnAddUserButtonClicked();
    void OnRemoveUserButtonClicked();
    void OnKeepKeyButtonClicked();
    void OnViewUsersButtonClicked();
    void OnDeleteKeyButtonClicked();
    void OnHistoryKeyButtonClicked();
};

#endif // KEY_VIEW_STACK_H
