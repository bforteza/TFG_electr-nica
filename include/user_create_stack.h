#ifndef USER_CREATE_STACK_H
#define USER_CREATE_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/radiobutton.h>
#include "db_schema.hpp"

class UserCreateStack
{
    public:
        UserCreateStack(const Glib::RefPtr<Gtk::Builder>& builder);

        void UserEdit(std::shared_ptr<kdb::Person> InPerson);
        void CreateUser();
    protected:
        std::shared_ptr<kdb::Person> EditedUser;
        bool CreateUserMode = 1;
        bool UserEditMode= 0;
        Gtk::Entry* UserNameEntry, *PasswordEntry, *RPasswordEntry;
        Gtk::Button* UserGenerateButton, *AddKeyButton, *AddUidUserButton;
        Gtk::RadioButton::Group Agroup;
        Gtk::RadioButton *A0RadioButton,
                         *A1RadioButton,
                         *A2RadioButton;
        Gtk::Label* UserNameErrorLabel, *PasswordErrorLabel, *RPasswordErrorLabel, *NfcErrorLabel;
        Gtk::TextView* UserUidText;
        void on_UserGenerateButton_clicked();
        void on_AddUidUserButton_clicked();
        void reset();
    private:
};

#endif // USER_CREATE_STACK_H
