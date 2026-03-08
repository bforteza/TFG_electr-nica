#include <glibmm.h>

#include <iostream>


#include "db_schema.hpp"
#include "litesql.hpp"
#include "nfc_manager.h"
#include "application.h"
#include "globals.h"
#include "mysql.h"


int main(int argc, char** argv) {

    try{
       // litesql::conne conn("mysql://usuario:clau@localhost/miBaseDeDatos");
        static kdb::DbSchema dbt("mysql","user=usuario;password=pene;database=miBaseDeDatos");
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
