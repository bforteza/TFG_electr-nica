#include "application.h"
#include "app_logger.h"
#include "globals.h"
#include "i2c_controller.h"
#include "litesql.hpp"
#include <iostream>
#include <sstream>
#include <gtkmm/cssprovider.h>
#include <gtkmm/settings.h>
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
    AppLogger::Init();

    Gtk::Application::on_startup();

    // Tema GTK — cambiar "Arc" por el tema instalado en el sistema.
    auto gtk_settings = Gtk::Settings::get_default();
    gtk_settings->property_gtk_theme_name() = "Raleigh";

    // Inicializa la base de datos. Si falla, termina la aplicación.
    try {
        // TODO: mover credenciales a un fichero de configuración externo.
        db = std::make_unique<kdb::DbManager>("mysql", "user=KEYSISTEM;password=KeySistem;database=miBaseDeDatos");
        if (db->needsUpgrade())
            db->upgrade();
        db->verbose = false;
        AppLogger::Info("DB", "Connection OK");
    } catch (litesql::Except& e) {
        std::ostringstream oss;
        oss << e;
        AppLogger::Error("DB", "Connection error: " + oss.str());
        quit();
        return;
    }

    nfcman = std::make_unique<NfcManager>();

    hw_ctrl = std::make_unique<I2cController>();
    if (!hw_ctrl->Init())
        AppLogger::Error("I2C", "Init failed: no se pudo abrir /dev/i2c");
    else
        AppLogger::Info("I2C", "Init OK");

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
