#ifndef APPLICATION_H
#define APPLICATION_H

#include <gtkmm/application.h>
#include "window.h"

// Punto de entrada de la aplicación GTK.
// Gestiona el ciclo de vida: arranque, creación de ventana y cierre limpio.
class Application : public Gtk::Application {
public:
    virtual ~Application() override;

    // Crea la instancia singleton de la aplicación.
    static Glib::RefPtr<Application> create();

private:
    Application();

    // Crea y registra una ventana principal en la aplicación.
    Window* CreateWindow();

    // Llamado por GTK al activar la aplicación (primera ejecución).
    // Crea la ventana principal y la muestra.
    void on_activate() override;

    // Llamado por GTK antes de on_activate(). Punto de inicialización
    // de recursos globales (acciones, menús, etc.).
    void on_startup() override;

    // Libera la ventana cuando se cierra (recibe señal hide).
    void OnHideWindow(Gtk::Window* window);

    // Oculta todas las ventanas y termina el bucle principal.
    void OnActionQuit();

};

#endif // APPLICATION_H
