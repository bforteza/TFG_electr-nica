#include "KeySelectStack.h"
#include "window.h"
KeySelectStack::KeySelectStack(const Glib::RefPtr<Gtk::Builder>& builder, Window *f)
{
    builder->get_widget("BackButtonKeySelect", BackButtonKeySelect);
    if (!BackButtonKeySelect) {
        throw std::runtime_error("No \"BackButtonKeySelect\" object in INI.glade" );
    }
      BackButtonKeySelect->signal_clicked().connect(sigc::mem_fun(*f, &Window::on_BackButton_clicked));
}
