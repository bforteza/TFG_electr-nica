#ifndef HOME_STACK_H
#define HOME_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/stack.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include "users_view_stack.h"
#include "user_create_stack.h"
#include "key_create_stack.h"
#include "key_view_stack.h"
#include "solenoid_panel.h"
#include "db_schema.hpp"
#include "translations.h"

class Window;

// Panel de administración, visible tras identificar un usuario.
// Muestra los botones de acción disponibles según el nivel de acceso
// y gestiona la navegación entre sus sub-vistas internas (HomeInnerStack).
class HomeStack {
public:
    HomeStack(const Glib::RefPtr<Gtk::Builder>& builder, Window* window);

    // Inicializa el panel con el usuario identificado.
    // Oculta todos los botones y muestra solo los que corresponden
    // al nivel de acceso del usuario.
    void PersonLogged(std::shared_ptr<kdb::Person> person);

    // Actualiza los textos de los botones al idioma activo.
    // Llamar tras cambiar current_language.
    void RefreshLabels();

    // Navega atrás: si estamos en AdminMainStack vuelve al login;
    // si no, retorna al panel anterior guardado en back_widget_.
    void OnBackButtonClicked();

    // Señal para abrir el SolenoidPanel (PICKUP al recoger llave, ADMIN desde botón).
    sigc::signal<void, std::shared_ptr<kdb::Key>, SolenoidPanel::Mode> signal_open_solenoid;

private:
    // Ventana principal; se usa para volver al login desde el panel raíz.
    Window* window_;

    // Stack interno con las sub-vistas (UsersView, KeyView, UserCreate, KeyCreate).
    Gtk::Stack* inner_stack_;

    // Vista a la que volver con el botón atrás cuando hay historial de navegación.
    Gtk::Widget* back_widget_ = nullptr;

    // Botón de volver del panel de administración.
    Gtk::Button* back_button_admin_;

    // Botón para ir a la sub-vista de creación de usuario.
    Gtk::Button* user_create_button_;

    // Botón para ir a la sub-vista de creación de llave.
    Gtk::Button* key_create_button_;

    // Botón para ir a la sub-vista de lista de llaves.
    Gtk::Button* view_keys_button_;

    // Botón para ir a la sub-vista de lista de usuarios.
    Gtk::Button* view_users_button_;

    // Botón para ir al historial de acciones (pendiente de implementar).
    Gtk::Button* history_button_;

    // Botón para acceder al panel de control de solenoides (solo admin).
    Gtk::Button* solenoid_panel_button_;

    // Etiqueta que muestra el nombre del usuario identificado.
    Gtk::Label* name_label_;

    // Sub-vistas embebidas dentro de inner_stack_.
    UsersViewStack  users_view_stack_;
    UserCreateStack user_create_stack_;
    KeyCreateStack  key_create_stack_;
    KeyViewStack    key_view_stack_;

    // Usuario actualmente identificado en el sistema.
    std::shared_ptr<kdb::Person> logged_person_;

    // Auxiliares para operaciones de vinculación: se preservan mientras se navega
    // entre sub-vistas para completar el flujo de link/unlink en dos pasos.
    std::shared_ptr<kdb::Person> aux_person_;
    std::shared_ptr<kdb::Key>    aux_key_;

    // Conexiones temporales de señales activas durante el modo selección.
    // Se desconectan al pulsar atrás.
    sigc::connection key_selected_connection_;
    sigc::connection user_selected_connection_;

    // Oculta todos los botones de acción antes de mostrar los que corresponden
    // al nivel de acceso del usuario.
    void HideButtons();

    // Manejadores de los botones del panel principal.
    void OnUserCreateButtonClicked();
    void OnViewUsersButtonClicked();
    void OnViewKeysButtonClicked();
    void OnKeyCreateButtonClicked();

    // Muestra la sub-vista de llaves/usuarios guardando la vista actual como
    // destino de vuelta para el botón atrás.
    void ShowKeysView(std::vector<kdb::Key> keys);
    void ShowUsersView(std::vector<kdb::Person> users);

    // Navega a la sub-vista de edición del elemento dado.
    void OnUserEdit(std::shared_ptr<kdb::Person> person);
    void OnKeyEdit(std::shared_ptr<kdb::Key> key);

    // Flujo de vinculación en dos pasos:
    // Si ya hay un aux_ guardado, vincula ambos elementos y vuelve atrás.
    // Si no, abre la vista de selección del elemento que falta.
    void OnUserLinkKey(std::shared_ptr<kdb::Person> person);
    void OnUserUnlinkKey(std::shared_ptr<kdb::Person> person);
    void OnKeyLinkUser(std::shared_ptr<kdb::Key> key);
    void OnKeyUnlinkUser(std::shared_ptr<kdb::Key> key);

    // Registra que el usuario identificado ha recogido la llave dada.
    void OnKeyKept(std::shared_ptr<kdb::Key> key);

    // Abre el SolenoidPanel en modo ADMIN al pulsar el botón de solenoides.
    void OnSolenoidPanelButtonClicked();
};

#endif // HOME_STACK_H
