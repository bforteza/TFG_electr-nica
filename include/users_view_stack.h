#ifndef USERS_VIEW_STACK_H
#define USERS_VIEW_STACK_H

#include <gtkmm/treemodelsort.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>

#include "models.h"
#include "db_schema.hpp"

class HomeStack;

class UsersViewStack
{
    public:
        UsersViewStack(const Glib::RefPtr<Gtk::Builder>& builder);

        sigc::signal<void,std::shared_ptr<kdb::Person>> UserSelected;
        sigc::signal<void,std::shared_ptr<kdb::Person>> UserLink;
        sigc::signal<void,std::shared_ptr<kdb::Person>> UserUnLink;
        sigc::signal<void,std::shared_ptr<kdb::Person>> UserEdit;
        sigc::signal<void,std::vector<kdb::Key>> KeysView;

        void view(std::vector<kdb::Person> i);
        void select(std::vector<kdb::Person> i);
    protected:
        Gtk::Button *AddKeyToUserButton,
                    *RemoveKeyToUserButton,
                    *SelectUserButton,
                    *UserEditButton,
                    *ViewKeysOfUserButton;

        Gtk::TreeView* ViewUsersTree;
        Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

        ModelColumns m_Columns;

        std::vector<kdb::Person> actual;

        void actualizar();
        int get_selection();
        std::shared_ptr<kdb::Person> get_selection_ptr();

        void on_UserEditButton_clicked();
        void on_AddKeyToUserButton_clicked();
        void on_RemoveKeyToUserButton_clicked();
        void on_ViewKeysOfUserButton_clicked();
        void on_SelectUserButton_clicked();

};

#endif // USERS_VIEW_STACK_H
