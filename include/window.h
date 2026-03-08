#ifndef WINDOW_H
#define WINDOW_H

#define PATH "./ui/MainWindow.glade"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/stack.h>
#include <gtkmm/button.h>
#include "home_stack.h"
#include "solenoid_panel.h"
#include "login_stack.h"

class Window : public Gtk::ApplicationWindow {
    public:
    Window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~Window();

    static Window* create();

    private:
    Glib::RefPtr<Gtk::Builder>  builder;

    Gtk::Stack*         PANTALLAS;

    friend class HomeStack;
    friend class SolenoidPanel;
    friend class LoginStack;
    LoginStack inistack;
    HomeStack adminstack;
    SolenoidPanel keysstack;

    void UserLogged(std::shared_ptr<kdb::Person> logged);
    void on_BackButton_clicked();
};

#endif // WINDOW_H
