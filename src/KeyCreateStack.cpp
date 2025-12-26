#include "KeyCreateStack.h"
#include "datos.hpp"
#include <litesql/selectquery.hpp>
#include "Globals.h"
#include <string>


KeyCreateStack::KeyCreateStack(const Glib::RefPtr<Gtk::Builder>& builder)
{
       //Entry
    builder->get_widget("KeyNameEntry", KeyNameEntry);
    if (!KeyNameEntry) {
        throw std::runtime_error("No \"KeyNameEntry\" object in INI.glade" );
    }

    builder->get_widget("UbiKeyEntry", UbiKeyEntry);
    if (!UbiKeyEntry) {
        throw std::runtime_error("No \"UbiKeyEntry\" object in INI.glade" );
    }

    builder->get_widget("ComentaryEntry", ComentaryEntry);
    if (!ComentaryEntry) {
        throw std::runtime_error("No \"ComentaryEntry\" object in INI.glade" );
    }
    builder->get_widget("PositionEntry", PositionEntry);
    if (!PositionEntry) {
        throw std::runtime_error("No \"PositionEntry\" object in INI.glade" );
    }

    //Buttons
    builder->get_widget("KeyGenerateButton", KeyGenerateButton);
    if (!KeyGenerateButton) {
        throw std::runtime_error("No \"KeyGenerateButton\" object in INI.glade" );
    }
    KeyGenerateButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyCreateStack::on_KeyGenerateButton_clicked));

    builder->get_widget("AddUserButton", AddUserButton);
    if (!AddUserButton) {
        throw std::runtime_error("No \"AddUserButton\" object in INI.glade" );
    }
    AddUserButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyCreateStack::on_AddUserButton_clicked));

    builder->get_widget("AddUidKeyButton", AddUidKeyButton);
    if (!AddUidKeyButton) {
        throw std::runtime_error("No \"AddUidKeyButton\" object in INI.glade" );
    }
    AddUidKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyCreateStack::on_AddUidKeyButton_clicked));


    //labels
    builder->get_widget("KeyNameErrorLabel", KeyNameErrorLabel);
    if (!KeyNameErrorLabel) {
        throw std::runtime_error("No \"KeyNameErrorLabel\" object in INI.glade" );
    }
    builder->get_widget("PositionErrorLabel", PositionErrorLabel);
    if (!PositionErrorLabel) {
        throw std::runtime_error("No \"PositionErrorLabel\" object in INI.glade" );
    }
    builder->get_widget("UidKeyErrorLabel", UidKeyErrorLabel);
    if (!UidKeyErrorLabel) {
        throw std::runtime_error("No \"UidKeyErrorLabel\" object in INI.glade" );
    }
    //text
     builder->get_widget("UidKeyText", UidKeyText);
    if (!UidKeyText) {
        throw std::runtime_error("No \"UidKeyText\" object in INI.glade" );
    }
}

void KeyCreateStack::on_KeyGenerateButton_clicked(){
    KeyNameErrorLabel->set_text("");
     bool ValidCreation = 1;

    if (CreateKeyMode){

        //Test Name repetition
        if( KeyNameEntry->get_text_length()< 3){
            KeyNameErrorLabel->set_text("Atenció: nom molt breu");
            ValidCreation *= 0;
        }
        if(litesql::select<kdb::Key>(*db, kdb::Key::Name == KeyNameEntry->get_text()).count() ){
            KeyNameErrorLabel->set_text("Atenció nom de clau existent");
            ValidCreation *= 0;
        }
        //Test Uid repetition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Uid == UidKeyText->get_buffer()->get_text()).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == UidKeyText->get_buffer()->get_text()).count()){
            UidKeyErrorLabel->set_text("Atenció: tarjeta en us");
            ValidCreation *= 0;
        }
        //Test Key position
        std::string AuxPos = PositionEntry->get_text();
        if (to_position(AuxPos)) {
            if(litesql::select<kdb::Key>(*db, kdb::Key::Pos == to_position(AuxPos)).count() ){
                PositionErrorLabel->set_text("Posicións no disponibles:");
                for(auto iter: (litesql::select<kdb::Key>(*db, kdb::Key::Pos > 0).orderBy(kdb::Key::Pos).all())){
                        PositionErrorLabel->set_text(PositionErrorLabel->get_text() + " " + to_string((int)iter.pos));
                }
                ValidCreation *= 0;
            }
        } else {
            PositionErrorLabel->set_text("Atenció: posició no válida");
            ValidCreation *= 0;
        }


        if( ValidCreation){
            kdb::Key AddedKey(*db);
            AddedKey.name = (std::string) KeyNameEntry->get_text();
            AddedKey.ubi = (std::string) UbiKeyEntry->get_text();
            AddedKey.commentary = (std::string) ComentaryEntry->get_text();
            AddedKey.uid = (std::string) UidKeyText->get_buffer()->get_text();
            AddedKey.pos = (int) to_position(PositionEntry->get_text());
            AddedKey.update();
            KeyEdit(std::make_shared<kdb::Key>(AddedKey));
        }

    } else if (EditKeyMode){

        //Test Name repetition
        if( KeyNameEntry->get_text_length()< 3){
            KeyNameErrorLabel->set_text("Atenció: nom molt breu");
            ValidCreation *= 0;
        }
        if(litesql::select<kdb::Key>(*db, kdb::Key::Name == KeyNameEntry->get_text()
                                     && kdb::Key::Id != EditedKey->id).count() ){
            KeyNameErrorLabel->set_text("Atenció nom de clau existent");
            ValidCreation *= 0;
        }
        //Test Uid repetition
        if(litesql::select<kdb::Person>(*db, kdb::Person::Uid == UidKeyText->get_buffer()->get_text()).count() +
            litesql::select<kdb::Key>(*db, kdb::Key::Uid == UidKeyText->get_buffer()->get_text()
                                      && kdb::Key::Id != EditedKey->id).count()){
            UidKeyErrorLabel->set_text("Atenció: tarjeta en us");
            ValidCreation *= 0;
        }
        //Test Key position
        std::string AuxPos = PositionEntry->get_text();
        if (to_position(AuxPos)) {
            if(litesql::select<kdb::Key>(*db, kdb::Key::Pos == to_position(AuxPos) && kdb::Key::Id != EditedKey->id).count() ){
                PositionErrorLabel->set_text("Posicións no disponibles:");
                for(auto iter: (litesql::select<kdb::Key>(*db, kdb::Key::Pos > 0).orderBy(kdb::Key::Pos).all())){
                        PositionErrorLabel->set_text(PositionErrorLabel->get_text() + " " + to_string((int)iter.pos));
                }
                ValidCreation *= 0;
            }
        } else {
            PositionErrorLabel->set_text("Atenció: posició no válida");
            ValidCreation *= 0;
        }

        if( ValidCreation){

            EditedKey->name = (std::string) KeyNameEntry->get_text();
            EditedKey->ubi = (std::string) UbiKeyEntry->get_text();
            EditedKey->commentary = (std::string) ComentaryEntry->get_text();
            EditedKey->uid = (std::string) UidKeyText->get_buffer()->get_text();
            EditedKey->pos = (int) to_position(PositionEntry->get_text());
            EditedKey->update();

        }
    }
}

void KeyCreateStack::on_AddUserButton_clicked(){

}

void KeyCreateStack::on_AddUidKeyButton_clicked(){
     std::string uid = nfcman->NfcDetect(1);
    if(!uid.empty()){
        UidKeyText->get_buffer()->set_text(uid);
    } else {
       UidKeyText->get_buffer()->set_text("Introdueix la tarjeta desitjada");
    }
}

void KeyCreateStack::CreateKey(){
    reset();

    KeyGenerateButton->set_label("Crear clau");
    CreateKeyMode = 1;
}

void KeyCreateStack::KeyEdit(std::shared_ptr<kdb::Key> InKey){
    reset();
    EditedKey = InKey;
    KeyNameEntry->set_text((std::string) InKey->name);
    UbiKeyEntry->set_text((std::string) InKey->ubi);
    ComentaryEntry->set_text((std::string) InKey->commentary);
    PositionEntry->set_text(to_string(InKey->pos));
    UidKeyText->get_buffer()->set_text((std::string) InKey->uid);

    EditKeyMode = 1;
}

void KeyCreateStack::reset(){
    KeyNameEntry->set_text("");
    UbiKeyEntry->set_text("");
    ComentaryEntry->set_text("");
    PositionEntry->set_text("");
    UidKeyText->get_buffer()->set_text("");

    KeyNameErrorLabel->set_text("");
    PositionErrorLabel->set_text("");
    UidKeyErrorLabel->set_text("");

    EditKeyMode = 0;
    CreateKeyMode = 0;
    EditedKey = nullptr;
}

