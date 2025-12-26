#ifndef KEYSELECTSTACK_H
#define KEYSELECTSTACK_H


#include <gtkmm/builder.h>
#include <gtkmm/button.h>
class Window;
class KeySelectStack
{
    public:
        KeySelectStack( const Glib::RefPtr<Gtk::Builder>& builder, Window *f);

    protected:
        Gtk::Button*        BackButtonKeySelect;
    private:
};

#endif // KEYSELECTSTACK_H
