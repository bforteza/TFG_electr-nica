#include "solenoid_panel.h"
#include "window.h"
#include <stdexcept>

SolenoidPanel::SolenoidPanel(const Glib::RefPtr<Gtk::Builder>& builder, Window* window) {
    builder->get_widget("BackButtonKeySelect", back_button_);
    if (!back_button_)
        throw std::runtime_error("No \"BackButtonKeySelect\" object in MainWindow.glade");
    back_button_->signal_clicked().connect(
        sigc::mem_fun(*window, &Window::OnBackButtonClicked));
}
