#include "application.h"
#include <iostream>

Application::Application()
    : Gtk::Application("main.application")
{}

Application::~Application() {}

Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

Window* Application::CreateWindow() {
    auto window = Window::create();
    add_window(*window);
    window->signal_hide().connect(
        sigc::bind(sigc::mem_fun(*this, &Application::OnHideWindow), window));
    return window;
}

void Application::on_activate() {
    try {
        auto window = CreateWindow();
        window->present();
    } catch (const Glib::Error& ex) {
        std::cerr << "Application::on_activate(): " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Application::on_activate(): " << ex.what() << std::endl;
    }
}

void Application::on_startup() {
    Gtk::Application::on_startup();
    // TODO: registrar acciones globales (Gio::Action) y mover aquí
    // la inicialización de BD y NFC desde main.cpp para poder mostrar
    // errores fatales con un diálogo GTK en lugar de cerr + return -1.
}

void Application::OnHideWindow(Gtk::Window* window) {
    delete window;
}

void Application::OnActionQuit() {
    for (auto window : get_windows())
        window->hide();
    quit();
}

void Application::SetKeySelectWindow() {
    // TODO: navegar a la pantalla de selección/devolución de llave.
}
