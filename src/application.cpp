#include "application.h"
#include "globals.h"
#include "litesql.hpp"
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

    // Inicializa la base de datos. Si falla, termina la aplicación.
    try {
        // TODO: mover credenciales a un fichero de configuración externo.
        db = std::make_unique<kdb::DbManager>("mysql", "user=usuario;password=CAMBIAR;database=miBaseDeDatos");
        if (db->needsUpgrade())
            db->upgrade();
        db->verbose = false;
    } catch (litesql::Except& e) {
        std::cerr << "DB error: " << e << std::endl;
        quit();
        return;
    }

    nfcman = std::make_unique<NfcManager>();

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
}

void Application::OnHideWindow(Gtk::Window* window) {
    delete window;
}

void Application::OnActionQuit() {
    for (auto window : get_windows())
        window->hide();
    quit();
}
