#include <glibmm.h>
#include <iostream>

#include "dbmanager.hpp"
#include "litesql.hpp"
#include "nfc_manager.h"
#include "application.h"
#include "globals.h"
#include "mysql.h"

int main(int argc, char** argv) {
    try {
        // Conecta con MariaDB en la Raspberry Pi.
        // TODO: mover credenciales a un fichero de configuración externo.
        static kdb::DbManager dbt("mysql", "user=usuario;password=CAMBIAR;database=miBaseDeDatos");
        db = &dbt;

        if (db->needsUpgrade())
            db->upgrade();

        db->verbose = true;

    } catch (litesql::Except e) {
        std::cerr << e << std::endl;
        return -1;
    }

    nfcman = std::make_unique<NfcManager>();

    auto app = Application::create();
    return app->run(argc, argv);
}
