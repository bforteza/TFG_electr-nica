#include <glibmm.h>

#include <iostream>


#include "datos.hpp"
#include "litesql.hpp"
#include "NfcManager.h"
#include "application.h"
#include "Globals.h"
#include "mysql.h"


int main(int argc, char** argv) {

    try{
       // litesql::conne conn("mysql://usuario:clau@localhost/miBaseDeDatos");
        static kdb::datos dbt("mysql","user=usuario;password=pene;database=miBaseDeDatos");
     //   dbt.create();
        // using SQLite3 as backend
       db = &dbt;

       if (db->needsUpgrade())
        db->upgrade();
        // create tables, sequences and indexes
      db->verbose = true;

    } catch (litesql::Except e){
        std::cerr<< e << std::endl;
        return -1;
    }
    nfcman = std::make_unique<NfcManager>();

     auto app = Application::create();
    return app->run(argc, argv);


}
