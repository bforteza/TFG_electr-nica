#include "window.h"

#include <gtkmm/settings.h>

#include <iostream>

void Window::on_BackButton_clicked(){
    PANTALLAS->set_visible_child("INI");
    inistack.Start();
}



Window::Window(Gtk::ApplicationWindow::BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject),
      builder(builder),
      adminstack(builder, this),
      keysstack(builder, this),
      inistack(builder){

    builder->get_widget("PantallasStack", PANTALLAS);
    if (!PANTALLAS) {
        throw std::runtime_error("No \"PANTALLAS\" object in INI.glade");
    }

    inistack.user_logged.connect(sigc::mem_fun(*this, &Window::UserLogged));

    inistack.Start();


}

Window::~Window() {
}

Window* Window::create() {
    auto builder = Gtk::Builder::create_from_file(PATH);

    Window* window = nullptr;
    builder->get_widget_derived("MainWindow", window);
    if (!window) {
        throw std::runtime_error("No \"window\" object in MainWindow.glade");
    }
    return window;
}


void Window::UserLogged(std::shared_ptr<kdb::Person> logged){
    PANTALLAS->set_visible_child("AdminView");
    adminstack.PersonLogg(logged);
}
