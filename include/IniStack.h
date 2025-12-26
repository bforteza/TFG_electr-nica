#ifndef INISTACK_H
#define INISTACK_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <string>
#include "datos.hpp"

class Window;
class IniStack
{
    public:
        IniStack(const Glib::RefPtr<Gtk::Builder>& builder, Window *f);
        sigc::signal<void,std::shared_ptr<kdb::Person>> UserLogg;
        sigc::signal<void,std::shared_ptr<kdb::Key>> KeyLogg;

        void start();
    protected:
        Gtk::Entry*         IN_IDEN;
        Gtk::Label*     LogErrorLabel;

        Window* father;

        void on_IN_IDEN_activate();
        void NfcDetected(const std::string in);

};

#endif // INISTACK_H
