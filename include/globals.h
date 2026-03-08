#ifndef GLOBALS_H
#define GLOBALS_H

#include "db_schema.hpp"
#include "nfc_manager.h"
#include <sigc++/signal.h>
#include <string>
#include <unordered_map>
#include <stdexcept>

// Señal global emitida tras cambiar current_language.
// Cada stack se suscribe en su constructor para refrescar sus textos.
inline sigc::signal<void> language_changed;


extern kdb::DbSchema* db;
inline std::unique_ptr<NfcManager> nfcman = nullptr;

enum class Position {
    N0=0,A1=1,A2,A3,A4,A5,A6,A7,A8,
    B1,B2,B3,B4,B5,B6,B7,B8,
    C1,C2,C3,C4,C5,C6,C7,C8,
    D1,D2,D3,D4,D5,D6,D7,D8};


// --- Conversión enum -> string ---
inline std::string to_string(int pos) {
    if(pos == 0) return "NULL";
    if(pos >32) return "OOR";
    int idx = pos -1;
    char letter = 'A' + (idx/8);
    int number = (idx % 8) +1;
    return std::string(1, letter) + std::to_string(number);

}

// --- Conversión string -> enum ---
inline int to_position(const std::string& s) {
    if(s.size()< 2) return 0;

    char letter = std::toupper(s[0]);

    if (letter < 'A'|| letter > 'D') return 0;

    letter -= 'A';

    char number = s[1] -'0';

    if (number < 0 || number > 9) return 0;

    return letter * 8 + number;
}


#endif // GLOBALS_H
