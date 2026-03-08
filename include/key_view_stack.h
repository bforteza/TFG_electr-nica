#ifndef KEY_VIEW_STACK_H
#define KEY_VIEW_STACK_H

#include <gtkmm/treemodelsort.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>

#include "models.h"
#include "db_schema.hpp"

class KeyViewStack
{
     public:
        KeyViewStack(const Glib::RefPtr<Gtk::Builder>& builder);

        sigc::signal<void,std::shared_ptr<kdb::Key>> KeySelected;
        sigc::signal<void,std::vector<kdb::Person>> UsersView;
        sigc::signal<void,std::shared_ptr<kdb::Key>> KeyLink;
        sigc::signal<void,std::shared_ptr<kdb::Key>> KeyUnLink;
        sigc::signal<void,std::shared_ptr<kdb::Key>> KeyEdit;
        sigc::signal<void,std::shared_ptr<kdb::Key>> KeyKeeped;

        void view(std::vector<kdb::Key> i);
        void view(std::shared_ptr<kdb::Person> InPerson);
        void select(std::vector<kdb::Key> i);
        bool configure();

    protected:

        uint8_t acces = 0;
        Gtk::Button *AddUserToKeyButton,
                    *RemoveUserToKeyButton,
                    *SelectKeyButton,
                    *EditKeyButton,
                    *KeepKeyButton,
                    *ViewUsersOfKeyButton,
                    *DeleteKeyButton;

        Gtk::TreeView* ViewKeysTree;
        Gtk::TreeViewColumn     *IdColumn,
                                *NameColumn,
                                *UbiColumn,
                                *CommentaryColumn,
                                *PosColumn,
                                *ActiveColumn,
                                *KeeperColumn;
        Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

        KeyModelColumns m_Columns;

        std::vector<kdb::Key> actual;

        void actualizar();
        int get_selection();
        std::shared_ptr<kdb::Key> get_selection_ptr();

        void on_SelectKeyButton_clicked();
        void on_EditKeyButton_clicked();
        void on_AddUserToKeyButton_clicked();
        void on_RemoveUserToKeyButton_clicked();
        void on_KeepKeyButton_clicked();
        void on_ViewUsersOfKeyButton_clicked();
        void on_DeleteKeyButton_clicked();
};

#endif // KEY_VIEW_STACK_H
