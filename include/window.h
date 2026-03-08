#ifndef WINDOW_H
#define WINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/stack.h>
#include "home_stack.h"
#include "login_stack.h"

// Ventana principal de la aplicación.
// Gestiona la navegación entre las pantallas principales:
//   LoginStack → HomeStack  (cuando se identifica un usuario)
//   LoginStack → KeyLoginStack (pendiente; cuando se acerca una llave para devolución)
class Window : public Gtk::ApplicationWindow {
public:
    Window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~Window();

    // Carga la ventana desde MainWindow.glade y devuelve el puntero.
    static Window* create();

    // Vuelve a la pantalla de login y reinicia el polling NFC.
    void OnBackButtonClicked();

private:
    Glib::RefPtr<Gtk::Builder> builder_;

    // Stack raíz que contiene LoginStack y HomeStack.
    Gtk::Stack* main_stack_;

    LoginStack login_stack_;
    HomeStack  home_stack_;

    // Llamado cuando LoginStack identifica un usuario válido.
    void OnUserLogged(std::shared_ptr<kdb::Person> person);

    // Llamado cuando LoginStack identifica una llave válida (devolución).
    // TODO: navegar a KeyLoginStack cuando esté implementado.
    void OnKeyLogged(std::shared_ptr<kdb::Key> key);
};

#endif // WINDOW_H
