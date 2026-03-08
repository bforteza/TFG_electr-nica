#ifndef SOLENOID_PANEL_H
#define SOLENOID_PANEL_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>

class Window;
class SolenoidPanel
{
    public:
        SolenoidPanel( const Glib::RefPtr<Gtk::Builder>& builder, Window *f);

    protected:
        Gtk::Button*        BackButtonKeySelect;
    private:
};

#endif // SOLENOID_PANEL_H
