#ifndef GLOBALS_H
#define GLOBALS_H

#include "dbmanager.hpp"
#include "nfc_manager.h"
#include <sigc++/signal.h>
#include <string>

// Señal global emitida cada vez que cambia current_language.
// Cada stack se suscribe en su constructor para refrescar sus textos de UI.
inline sigc::signal<void> language_changed;

// Puntero a la base de datos activa. Se inicializa en main() y se usa en
// todos los stacks para consultar y modificar registros.
extern kdb::DbManager* db;

// Gestor NFC. Se inicializa en main() y se usa desde LoginStack y UserCreateStack
// para detectar tarjetas NFC en segundo plano.
inline std::unique_ptr<NfcManager> nfcman = nullptr;

// Enumera las 32 posiciones físicas del armario (4 filas × 8 columnas).
// N0 representa «sin posición asignada».
enum class Position {
    N0 = 0,
    A1 = 1, A2, A3, A4, A5, A6, A7, A8,
    B1,     B2, B3, B4, B5, B6, B7, B8,
    C1,     C2, C3, C4, C5, C6, C7, C8,
    D1,     D2, D3, D4, D5, D6, D7, D8,
};

// Convierte un índice numérico de posición (1–32) en su cadena legible
// (p.ej. 1→"A1", 9→"B1"). Devuelve "NULL" para 0 y "OOR" si está fuera
// de rango.
inline std::string PosToString(int pos) {
    if (pos == 0) return "NULL";
    if (pos > 32) return "OOR";
    int idx     = pos - 1;
    char letter = 'A' + (idx / 8);
    int number  = (idx % 8) + 1;
    return std::string(1, letter) + std::to_string(number);
}

// Convierte una cadena de posición (p.ej. "A1", "D8") en su índice numérico
// (1–32). Devuelve 0 si la cadena no representa una posición válida.
inline int PosFromString(const std::string& s) {
    if (s.size() < 2) return 0;
    char letter = std::toupper(s[0]);
    if (letter < 'A' || letter > 'D') return 0;
    letter -= 'A';
    int number = s[1] - '0';
    if (number < 1 || number > 8) return 0;
    return letter * 8 + number;
}

#endif // GLOBALS_H
