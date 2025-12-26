#ifndef WINDOW_H
#define WINDOW_H

#define PATH "./ui/MainWindow.glade"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/stack.h>
#include <gtkmm/button.h>
#include "AdminStack.h"
#include "KeySelectStack.h"
#include "IniStack.h"

class Window : public Gtk::ApplicationWindow {
    public:
    Window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~Window();

    static Window* create();

    private:
    Glib::RefPtr<Gtk::Builder>  builder;


    Gtk::Stack*         PANTALLAS;

    friend class AdminStack;
    friend class KeySelectStack;
    friend class IniStack;
    IniStack inistack;
    AdminStack adminstack;
    KeySelectStack keysstack;

    void UserLogged(std::shared_ptr<kdb::Person> logged);
    void on_BackButton_clicked();

  //  void setHeaderBar();
};

#endif // WINDOW_H
