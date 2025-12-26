#ifndef KEYCREATESTACK_H
#define KEYCREATESTACK_H

#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <string>
#include "datos.hpp"
class KeyCreateStack
{
    public:
        KeyCreateStack(const Glib::RefPtr<Gtk::Builder>& builder);

        void CreateKey();
        void KeyEdit(std::shared_ptr<kdb::Key> InKey);
    protected:
        Gtk::Entry  *KeyNameEntry,
                    *UbiKeyEntry,
                    *ComentaryEntry,
                    *PositionEntry;
        Gtk::Button *KeyGenerateButton,
                    *AddUserButton,
                    *AddUidKeyButton;
        Gtk::Label *KeyNameErrorLabel,
                    *PositionErrorLabel,
                    *UidKeyErrorLabel;
        Gtk::TextView *UidKeyText;
    private:
        bool EditKeyMode = 0;
        bool CreateKeyMode = 1;

        void reset();
        void on_KeyGenerateButton_clicked();
        void on_AddUserButton_clicked();
        void on_AddUidKeyButton_clicked();

        std::shared_ptr<kdb::Key> EditedKey;
};

#endif // KEYCREATESTACK_H
