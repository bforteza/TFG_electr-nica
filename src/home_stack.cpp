#include "home_stack.h"
#include "window.h"
#include "globals.h"
HomeStack::HomeStack(const Glib::RefPtr<Gtk::Builder>& builder, Window* f):father(f),builder(builder),
usersviewstack(builder),
usercreatestk(builder),
keycreatestk(builder),
keyviewstk(builder)
{
    builder->get_widget("BackButtonAdmin", BackButtonAdmin);
    if (!BackButtonAdmin) {
        throw std::runtime_error("No \"BackButtonAdmin\" object in INI.glade" );
    }
      BackButtonAdmin->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_BackButton_clicked));


    builder->get_widget("UserCreateButton", UserCreateButton);
    if (!UserCreateButton) {
        throw std::runtime_error("No \"UserCreateButton\" object in INI.glade" );
    }
    UserCreateButton->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_UserCreateButton_clicked));


    builder->get_widget("KeyCreateButton", KeyCreateButton);
    if (!KeyCreateButton) {
        throw std::runtime_error("No \"KeyCreateButton\" object in INI.glade" );
    }
    KeyCreateButton->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_KeyCreateButton_clicked));

    builder->get_widget("ViewUsersButton", ViewUsersButton);
    if (!ViewUsersButton) {
        throw std::runtime_error("No \"ViewUsersButton\" object in INI.glade" );
    }
    ViewUsersButton->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_ViewUsersButton_clicked));


    builder->get_widget("ViewKeysButton", ViewKeysButton);
    if (!ViewKeysButton) {
        throw std::runtime_error("No \"ViewKeysButton\" object in INI.glade" );
    }
    ViewKeysButton->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_ViewKeysButton_clicked));

    builder->get_widget("HistoryButton", HistoryButton);
    if (!HistoryButton) {
        throw std::runtime_error("No \"HistoryButton\" object in INI.glade" );
    }
    //HistoryButton->signal_clicked().connect(sigc::mem_fun(*this, &HomeStack::on_HistoryButton_clicked));


    builder->get_widget("AdminStacks", AdminStackO);
    if (!AdminStackO)
        throw std::runtime_error("No \"AdminStackO\" object in INI.glade" );

    //Labels
    builder->get_widget("NameLabel", NameLabel);
    if(!NameLabel){
        throw std::runtime_error("No NameLabel object in INI.glade");
    }

    //connectar señales
    usersviewstack.UserLink.connect(sigc::mem_fun(*this, &HomeStack::UserLinkKey));
    usersviewstack.UserUnLink.connect(sigc::mem_fun(*this, &HomeStack::UserUnLinkKey));
    usersviewstack.KeysView.connect(sigc::mem_fun(*this, &HomeStack::KeysView));
    usersviewstack.UserEdit.connect(sigc::mem_fun(*this, &HomeStack::UserEdit));

    keyviewstk.KeyLink.connect(sigc::mem_fun(*this, &HomeStack::KeyLinkUser));
    keyviewstk.KeyUnLink.connect(sigc::mem_fun(*this, &HomeStack::KeyUnLinkUser));
    keyviewstk.UsersView.connect(sigc::mem_fun(*this, &HomeStack::UsersView));
    keyviewstk.KeyEdit.connect(sigc::mem_fun(*this, &HomeStack::KeyEdit));
    keyviewstk.KeyKeeped.connect(sigc::mem_fun(*this, &HomeStack::KeyKeeped));


}

//Initiator

void HomeStack::PersonLogg(std::shared_ptr<kdb::Person> InPerson){
    NameLabel->set_text((std::string)InPerson->name);
    hide();
    int acces = (int) InPerson->a1 + 2 * (int)InPerson->a2;
    if (acces >= 0){
        ViewKeysButton->show();
    }
     if (acces>=1){
        KeyCreateButton->show();
    }
     if (acces==2){

        ViewUsersButton->show();
        UserCreateButton->show();
    }
    Logged = InPerson;
}

//Button actions

void HomeStack::on_UserCreateButton_clicked(){
    usercreatestk.CreateUser();
   AdminStackO->set_visible_child("UserCreateStack");
}

void HomeStack::on_ViewUsersButton_clicked(){
    usersviewstack.view(litesql::select<kdb::Person>(*db).all());
AdminStackO->set_visible_child("ViewUsersStack");
}

void HomeStack::on_ViewKeysButton_clicked(){
    keyviewstk.view(Logged);
    AdminStackO->set_visible_child("ViewKeyStack");
}

void HomeStack::on_BackButton_clicked(){
    KeySelectedConnection.disconnect();
    UserSelectedConnection.disconnect();
    AuxKey= nullptr;
    AuxPerson =nullptr;

    if (AdminStackO->get_visible_child_name()=="AdminMainStack")
        father->on_BackButton_clicked();
    else if (Back == nullptr){
        AdminStackO->set_visible_child("AdminMainStack");
    } else {
        AdminStackO->set_visible_child(*Back);
        Back = nullptr;
    }

}

void HomeStack::on_KeyCreateButton_clicked(){
    AdminStackO->set_visible_child("KeyCreateStack");
}

//Internal auxiliars

void HomeStack::KeysView(std::vector<kdb::Key> in){
    keyviewstk.view(in);
    Back = AdminStackO->get_visible_child();
    AdminStackO->set_visible_child("ViewKeyStack");
}

void HomeStack::UsersView(std::vector<kdb::Person> in){
    usersviewstack.view(in);
    Back = AdminStackO->get_visible_child();
    AdminStackO->set_visible_child("ViewUsersStack");
}

void HomeStack::UserEdit(std::shared_ptr<kdb::Person> InPerson){
    usercreatestk.UserEdit(InPerson);
    Back = AdminStackO->get_visible_child();
    AdminStackO->set_visible_child("UserCreateStack");
}

void HomeStack::KeyEdit(std::shared_ptr<kdb::Key> InKey){
    keycreatestk.KeyEdit(InKey);
    Back = AdminStackO->get_visible_child();
    AdminStackO->set_visible_child("KeyCreateStack");
}

void HomeStack::UserLinkKey(std::shared_ptr<kdb::Person> InPerson){
    AuxPerson = InPerson;

    if(AuxKey == nullptr){
        keyviewstk.select((litesql::except(litesql::select<kdb::Key>(*db), AuxPerson->keys().get())).all());
        Back = AdminStackO->get_visible_child();
        KeySelectedConnection = keyviewstk.KeySelected.connect(sigc::mem_fun(*this, &HomeStack::KeyLinkUser));
        AdminStackO->set_visible_child("ViewKeyStack");
    } else {
        AuxPerson->keys().link(*AuxKey);
        on_BackButton_clicked();
    }

}

void HomeStack::KeyLinkUser(std::shared_ptr<kdb::Key> InKey){
    AuxKey = InKey;
    if(AuxPerson == nullptr){
        usersviewstack.select((litesql::except(litesql::select<kdb::Person>(*db), AuxKey->owners().get())).all());
        Back = AdminStackO->get_visible_child();
        UserSelectedConnection = usersviewstack.UserSelected.connect(sigc::mem_fun(*this, &HomeStack::UserLinkKey));
        AdminStackO->set_visible_child("ViewUsersStack");
    } else {
        AuxPerson->keys().link(*AuxKey);
         on_BackButton_clicked();
    }

}

void HomeStack::UserUnLinkKey(std::shared_ptr<kdb::Person> InPerson){
    AuxPerson = InPerson;

    if(AuxKey == nullptr){
        keyviewstk.select(InPerson->keys().get().all());
        Back = AdminStackO->get_visible_child();
        KeySelectedConnection = keyviewstk.KeySelected.connect(sigc::mem_fun(*this, &HomeStack::KeyUnLinkUser));
        AdminStackO->set_visible_child("ViewKeyStack");
    } else {
        AuxPerson->keys().unlink(*AuxKey);
         on_BackButton_clicked();
    }

}

void HomeStack::KeyUnLinkUser(std::shared_ptr<kdb::Key> InKey){
    AuxKey = InKey;

    if(AuxPerson == nullptr){
        usersviewstack.select(InKey->owners().get().all());
        Back = AdminStackO->get_visible_child();
        AdminStackO->set_visible_child("ViewUsersStack");
    } else {
        AuxPerson->keys().unlink(*AuxKey);
        on_BackButton_clicked();
    }

}

void HomeStack::KeyKeeped(std::shared_ptr<kdb::Key> InKey){
    Logged->keepkeys().link(*InKey);
}

void HomeStack::hide(){
    UserCreateButton->hide();
    KeyCreateButton->hide();
    ViewKeysButton->hide();
    ViewUsersButton->hide();
    HistoryButton->hide();
}
