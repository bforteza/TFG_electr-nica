#ifndef AdminStack_H
#define AdminStack_H

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/stack.h>
#include <gtkmm/label.h>
#include <UsersViewStack.h>
#include "UserCreateStack.h"
#include "KeyCreateStack.h"
#include "KeyViewStack.h"
#include "datos.hpp"


class Window;

class AdminStack
{
    public:
        AdminStack( const Glib::RefPtr<Gtk::Builder>& builder, Window *f);

         void PersonLogg(std::shared_ptr<kdb::Person> InPerson);

    protected:
         Window* father;
        Glib::RefPtr<Gtk::Builder>  builder;

        Gtk::Button*        BackButtonAdmin,
                            *UserCreateButton,
                            *KeyCreateButton,
                            *ViewKeysButton,
                            *ViewUsersButton,
                            *HistoryButton;

        Gtk::Label*         NameLabel;

        Gtk::Stack*         AdminStackO;
        Gtk::Widget*        Back = nullptr;

        void on_UserCreateButton_clicked();
        void on_ViewUsersButton_clicked();
        void on_ViewKeysButton_clicked();
        void on_BackButton_clicked();
        void on_KeyCreateButton_clicked();

        void KeysView(std::vector<kdb::Key> in);
        void UsersView(std::vector<kdb::Person> in);
        void UserEdit(std::shared_ptr<kdb::Person> InPerson);
        void KeyEdit(std::shared_ptr<kdb::Key> InKey);

        void UserLinkKey(std::shared_ptr<kdb::Person> InPerson);
        void UserUnLinkKey(std::shared_ptr<kdb::Person> InPerson);
        void KeyLinkUser(std::shared_ptr<kdb::Key> InKey);
        void KeyUnLinkUser(std::shared_ptr<kdb::Key> InKey);

        void KeyKeeped(std::shared_ptr<kdb::Key> InKey);

        void hide();

        UsersViewStack usersviewstack;
        UserCreateStack usercreatestk;
        KeyCreateStack keycreatestk;
        KeyViewStack keyviewstk;

        //linker auxiliars

        std::shared_ptr<kdb::Person> Logged;

        std::shared_ptr<kdb::Person> AuxPerson;
        std::shared_ptr<kdb::Key> AuxKey;

        sigc::connection KeySelectedConnection;
        sigc::connection UserSelectedConnection;
};

#endif // AdminStack_H
