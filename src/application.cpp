#include "application.h"
#include <iostream>
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gdkmm/screen.h>

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

    // Carga la hoja de estilos global (fuentes, tamaños para pantalla táctil).
    // Para ajustar tamaños edita ui/style.css — ver comentarios en ese archivo.
    auto css = Gtk::CssProvider::create();
    try {
        css->load_from_path("./ui/style.css");
    } catch (const Gtk::CssProviderError& ex) {
        std::cerr << "CSS error: " << ex.what() << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "CSS load error: " << ex.what() << std::endl;
    }
    Gtk::StyleContext::add_provider_for_screen(
        Gdk::Screen::get_default(),
        css,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

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
