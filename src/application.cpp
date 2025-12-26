#include "application.h"
#include <iostream>
void Application::set_KeySelectWindow(){
//    get_active_window().hide()


}
Application::Application():
    Gtk::Application("main.application")
{
    //ctor
}

Application::~Application()
{
    //dtor
}
Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

Window *Application::createWindow() {
    auto window = Window::create();
    add_window(*window);
    window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &Application::on_hide_window), window));
    return window;
}

void Application::on_activate() {
    try {
         auto window = Window::create();
        add_window(*window);
        window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &Application::on_hide_window), window));
        window->present();
    } catch (const Glib::Error &ex) {
        std::cerr << "Application::on_activate(): " << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "Application::on_activate(): " << ex.what() << std::endl;
    }

}

void Application::on_startup() {
    Gtk::Application::on_startup();

    //añadir acciones
/*
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource("ui/menu.glade");
    } catch (const Glib::Error &ex) {
        std::cerr << "Application::on_startup(): " << ex.what() << std::endl;
        return;
    }

   */
}

void Application::on_hide_window(Gtk::Window *window) {
    delete window;
}
/*
void Application::on_action_preferences() {
    try {
        auto prefsDialog = Preferences::create(*get_active_window());
        prefsDialog->present();
        prefsDialog->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &Application::on_hide_window), prefsDialog));
    } catch (const Glib::Error &ex) {
        std::cerr << "Application::on_action_preferences(): " << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "Application::on_action_preferences(): " << ex.what() << std::endl;
    }
}
*/
void Application::on_action_quit() {
    auto windows = get_windows();
    for (auto window : windows) {
        window->hide();
    }
    quit();
}
