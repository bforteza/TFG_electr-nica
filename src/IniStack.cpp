#include "IniStack.h"
#include "window.h"
#include "Globals.h"
IniStack::IniStack(const Glib::RefPtr<Gtk::Builder>& builder, Window *f):father(f)
{
    //Entrada de usuario
    builder->get_widget("IN_IDEN", IN_IDEN);
    if (!IN_IDEN) {
        throw std::runtime_error("No \"IN_IDEN\" object in INI.glade" );
    }

    IN_IDEN->signal_activate().connect(sigc::mem_fun(*this, &IniStack::on_IN_IDEN_activate));

    builder->get_widget("LogErrorLabel", LogErrorLabel);
    if (!LogErrorLabel) {
        throw std::runtime_error("No \"LogErrorLabel\" object in INI.glade" );
    }

    nfcman->aviso.connect([&](){
                          this->NfcDetected(nfcman->Uid);
                          });
    //ctor
}

void IniStack::on_IN_IDEN_activate()
{
    std::string in= IN_IDEN->get_text();
    try{
        UserLogg.emit(std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db, kdb::Person::Password == in).one()));
        LogErrorLabel->set_text("");
    } catch (...) {
        LogErrorLabel->set_text("Atenció: credencials no identificades");
    }
}

void IniStack::start(){
    LogErrorLabel->set_text("");

    nfcman->startPolling();

}

void IniStack::NfcDetected(const std::string  in){

    std::cout<< in<< std::endl;

    uint8_t Pcounter = litesql::select<kdb::Person>(*db, kdb::Person::Uid == in).count();
    uint8_t Kcounter = litesql::select<kdb::Key>(*db, kdb::Key::Uid == in && kdb::Key::Active == 1).count();
    if(!(Pcounter+Kcounter)){
        LogErrorLabel->set_text("Atenció: dispositiu no reconegut");
    } else if(Pcounter+Kcounter>1){
        LogErrorLabel->set_text("ERROR: dos dipositius amb el mateix uid, contacta amb l'administrado");
    } else if (Pcounter){
        UserLogg.emit(std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db, kdb::Person::Uid == in).one()));
         nfcman->stopPolling();
    } else if (Kcounter){
        KeyLogg.emit(std::make_shared<kdb::Key> (litesql::select<kdb::Key>(*db, kdb::Key::Uid== in
                                                                           && kdb::Key::Active==1).one()));
         nfcman->stopPolling();
    }
}



