#ifndef HISTORY_LOGGER_H
#define HISTORY_LOGGER_H

#include <memory>
#include "dbmanager.hpp"

// Tipos de evento registrados en el historial.
// Los valores numéricos se almacenan directamente en HistoryEvent::etype.
enum class HistoryEventType {
    PICKUP          = 0,  // Usuario recoge llave
    RETURN          = 1,  // Llave devuelta por NFC (persona desconocida)
    ADMIN_OPEN      = 2,  // Admin abre slot (key=nullptr si slot vacío)
    KEY_CREATED     = 3,  // Admin crea llave
    KEY_DEACTIVATED = 4,  // Admin desactiva llave
};

// Funciones de registro del historial.
// Cada función crea y persiste un HistoryEvent en la base de datos global.
// Los argumentos opcionales (key, person) pueden ser nullptr cuando no apliquen.
namespace history {

// Registra que 'person' ha recogido 'key'.
void LogPickup(std::shared_ptr<kdb::Person> person,
               std::shared_ptr<kdb::Key>    key);

// Registra la devolución de 'key' (persona desconocida).
void LogReturn(std::shared_ptr<kdb::Key> key);

// Registra que el admin 'person' ha activado el slot 'pos'.
// Si no hay llave asignada en ese slot, key puede ser nullptr.
void LogAdminOpen(std::shared_ptr<kdb::Person> person,
                  std::shared_ptr<kdb::Key>    key,
                  int                          pos);

// Registra la creación de 'key' por 'creator' (puede ser nullptr si se desconoce).
void LogKeyCreated(std::shared_ptr<kdb::Person> creator,
                   std::shared_ptr<kdb::Key>    key);

// Registra la desactivación de 'key' por 'admin'.
void LogKeyDeactivated(std::shared_ptr<kdb::Person> admin,
                       std::shared_ptr<kdb::Key>    key);

} // namespace history

#endif // HISTORY_LOGGER_H
