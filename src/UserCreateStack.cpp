#include "UserCreateStack.h"
#include "datos.hpp"
#include <litesql/selectquery.hpp>
#include "Globals.h"
#include <string>

UserCreateStack::UserCreateStack(const Glib::RefPtr<Gtk::Builder>& builder)
{
    //Entry
    builder->get_widget("UserNameEntry", UserNameEntry);
    if (!UserNameEntry) {
        throw std::runtime_error("No \"UserNameEntry\" object in INI.glade" );
    }

    builder->get_widget("PasswordEntry", PasswordEntry);
    if (!PasswordEntry) {
        throw std::runtime_error("No \"PasswordEntry\" object in INI.glade" );
    }

    builder->get_widget("RPasswordEntry", RPasswordEntry);
    if (!RPasswordEntry) {
        throw std::runtime_error("No \"RPasswordEntry\" object in INI.glade" );
    }
    //Buttons

    builder->get_widget("UserGenerateButton", UserGenerateButton);
    if (!UserGenerateButton) {
        throw std::runtime_error("No \"UserGenerateButton\" object in INI.glade" );
    }
    UserGenerateButton->signal_clicked().connect(sigc::mem_fun(*this, &UserCreateStack::on_UserGenerateButton_clicked));

    builder->get_widget("AddUidUserButton", AddUidUserButton);
    if (!AddUidUserButton) {
        throw std::runtime_error("No \"AddUidUserButton\" object in INI.glade" );
    }
    AddUidUserButton->signal_clicked().connect(sigc::mem_fun(*this, &UserCreateStack::on_AddUidUserButton_clicked));

    builder->get_widget("AddKeyButton", AddKeyButton);
    if (!AddKeyButton) {
        throw std::runtime_error("No \"AddKeyButton\" object in INI.glade" );
    }
    //RadioButtons
     builder->get_widget("A0RadioButton", A0RadioButton);
    if (!A0RadioButton) {
        throw std::runtime_error("No \"A0RadioButton\" object in INI.glade" );
    }
    builder->get_widget("A1RadioButton", A1RadioButton);
    if (!A0RadioButton) {
        throw std::runtime_error("No \"A1RadioButton\" object in INI.glade" );
    }
     builder->get_widget("A2RadioButton", A2RadioButton);
    if (!A0RadioButton) {
        throw std::runtime_error("No \"A2RadioButton\" object in INI.glade" );
    }
    Agroup = A0RadioButton->get_group();
    A1RadioButton->set_group(Agroup);
    A2RadioButton->set_group(Agroup);
    //labels
    builder->get_widget("UserNameErrorLabel", UserNameErrorLabel);
    if (!UserNameErrorLabel) {
        throw std::runtime_error("No \"UserNameErrorLabel\" object in INI.glade" );
    }

    builder->get_widget("PasswordErrorLabel", PasswordErrorLabel);
    if (!PasswordErrorLabel) {
        throw std::runtime_error("No \"PasswordErrorLabel\" object in INI.glade" );
    }

    builder->get_widget("RPasswordErrorLabel", RPasswordErrorLabel);
    if (!RPasswordErrorLabel) {
        throw std::runtime_error("No \"RPasswordErrorLabel\" object in INI.glade" );
    }
    builder->get_widget("NfcErrorLabel", NfcErrorLabel);
    if (!NfcErrorLabel) {
        throw std::runtime_error("No \"NfcErrorLabel\" object in INI.glade" );
    }
    //Text
    builder->get_widget("UserUidText", UserUidText);
    if (!UserUidText) {
        throw std::runtime_error("No \"UserUidText\" object in INI.glade" );
    }


}

// Button methods
void UserCreateStack::on_UserGenerateButton_clicked(){
    UserNameErrorLabel->set_text("");
    PasswordErrorLabel->set_text("");
    RPasswordErrorLabel->set_text("");
    bool ValidCreation = 1;
    if (CreateUserMode){


        //Test Name repetition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Name == UserNameEntry->get_text()).count() ){
            UserNameErrorLabel->set_text("Atenció: nom de usuari existent");
            ValidCreation *= 0;
        }
        //Test repete password
        if( RPasswordEntry->get_text() != PasswordEntry->get_text() ){
            RPasswordErrorLabel->set_text("Atenció: contrasenya de no coincideix");
            ValidCreation *= 0;
        }
        //Test Password repettion
        if(litesql::select<kdb::Person>(*db, kdb::Person::Password == PasswordEntry->get_text()).count() ){
            PasswordErrorLabel->set_text("Atenció: contrasenya de usuari repetit");
            ValidCreation *= 0;
        }
        //Test Name lenght
        if(UserNameEntry->get_text_length()< 2){
            UserNameErrorLabel->set_text("Atenció: nom de usuari massa curt");
            ValidCreation *= 0;
        }
        //Test Password lenght
        if(PasswordEntry->get_text_length()< 2){
            PasswordErrorLabel->set_text("Atenció: contrasenya de usuari massa curt");
            ValidCreation *= 0;
        }
        //Test Uid repettition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Uid == UserUidText->get_buffer()->get_text()).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == UserUidText->get_buffer()->get_text()).count()){
            NfcErrorLabel->set_text("Atenció: tarjeta en us");
            ValidCreation *= 0;
        }

        if( ValidCreation){
            kdb::Person AddedPerson(*db);
            AddedPerson.uid = (std::string) UserUidText->get_buffer()->get_text();
            AddedPerson.name = (std::string) UserNameEntry->get_text();
            AddedPerson.password = (std::string) PasswordEntry->get_text();
            AddedPerson.a1 = A1RadioButton->get_active();
            AddedPerson.a2 = A2RadioButton->get_active();
            AddedPerson.update();
            reset();
            UserEdit(std::make_shared<kdb::Person>(AddedPerson));
        }

    } else if (UserEditMode){
        //Test Name repetition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Name == UserNameEntry->get_text()
                                         && kdb::Person::Id != EditedUser->id).count() ){
            UserNameErrorLabel->set_text("Atenció: nom de usuari existent");
            ValidCreation *= 0;
        }
        //Test repete password
        if( RPasswordEntry->get_text() != PasswordEntry->get_text() ){
            RPasswordErrorLabel->set_text("Atenció: contrasenya de no coincideix");
            ValidCreation *= 0;
        }
        //Test Password repettion
        if(litesql::select<kdb::Person>(*db, kdb::Person::Password == PasswordEntry->get_text()
                                        && kdb::Person::Id != EditedUser->id).count() ){
            PasswordErrorLabel->set_text("Atenció: contrasenya de usuari repetit");
            ValidCreation *= 0;
        }
        //Test Name lenght
        if(UserNameEntry->get_text_length()< 2){
            UserNameErrorLabel->set_text("Atenció: nom de usuari massa curt");
            ValidCreation *= 0;
        }
        //Test Password lenght
        if(PasswordEntry->get_text_length()< 2){
            PasswordErrorLabel->set_text("Atenció: contrasenya de usuari massa curt");
            ValidCreation *= 0;
        }
        //Test Uid repettition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Uid == UserUidText->get_buffer()->get_text()
                                        && kdb::Person::Id != EditedUser->id).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == UserUidText->get_buffer()->get_text()).count()){
            NfcErrorLabel->set_text("Atenció: tarjeta en us");
            ValidCreation *= 0;
        }

        if( ValidCreation){

            EditedUser->uid = (std::string) UserUidText->get_buffer()->get_text();
            EditedUser->name = (std::string) UserNameEntry->get_text();
            EditedUser->password = (std::string) PasswordEntry->get_text();
            EditedUser->a1 = A1RadioButton->get_active();
            EditedUser->a2 = A2RadioButton->get_active();
            EditedUser->update();
            UserEdit(EditedUser);
        }
    }

}


void UserCreateStack::on_AddUidUserButton_clicked(){
    std::string uid = nfcman->NfcDetect(1);
    if(!uid.empty()){
        UserUidText->get_buffer()->set_text(uid);
    } else {
       UserUidText->get_buffer()->set_text("Introdueix la tarjeta desitjada");
    }
}

void UserCreateStack::UserEdit(std::shared_ptr<kdb::Person> InPerson){
    reset();
    UserEditMode = 1;
    UserGenerateButton->set_label("Editar usuari");
    UserNameEntry->set_text((std::string)InPerson->name);
    PasswordEntry->set_text((std::string)InPerson->password);
    RPasswordEntry->set_text((std::string)InPerson->password);
    UserUidText->get_buffer()->set_text((std::string) InPerson->uid);
    EditedUser = InPerson;
}

void UserCreateStack::reset(){

   UserNameErrorLabel->set_text("");
   PasswordErrorLabel->set_text("");
   RPasswordErrorLabel->set_text("");
   NfcErrorLabel->set_text("");
   CreateUserMode = 0;
   UserEditMode = 0;

    UserNameEntry->set_text("");
    PasswordEntry->set_text("");
    RPasswordEntry->set_text("");
    UserUidText->get_buffer()->set_text("");
    EditedUser= nullptr;
}

void UserCreateStack::CreateUser(){
    reset();
    CreateUserMode = 1;
    UserGenerateButton->set_label("Crear usuari");
}
