#ifndef SOLENOID_PANEL_H
#define SOLENOID_PANEL_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>

class Window;

// Panel de apertura de solenoides (pendiente de implementar).
// Actualmente solo contiene el botón de regreso al panel principal.
class SolenoidPanel {
public:
    SolenoidPanel(const Glib::RefPtr<Gtk::Builder>& builder, Window* window);

private:
    // Botón para volver a la pantalla principal (HomeStack).
    Gtk::Button* back_button_;
};

#endif // SOLENOID_PANEL_H
