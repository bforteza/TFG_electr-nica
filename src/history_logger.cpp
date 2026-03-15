#include "history_logger.h"
#include "globals.h"
#include <chrono>
#include <iomanip>
#include <sstream>

// Devuelve el timestamp actual en formato ISO 8601: "YYYY-MM-DD HH:MM:SS".
static std::string NowTimestamp() {
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Crea y persiste un HistoryEvent con los campos dados.
static void WriteEvent(int etype, int keyid, const std::string& keyname,
                       int personid, const std::string& personname, int pos) {
    kdb::HistoryEvent ev(*db);
    ev.etype      = etype;
    ev.timestamp  = NowTimestamp();
    ev.keyid      = keyid;
    ev.keyname    = keyname;
    ev.personid   = personid;
    ev.personname = personname;
    ev.pos        = pos;
    ev.update();
}

namespace history {

void LogPickup(std::shared_ptr<kdb::Person> person,
               std::shared_ptr<kdb::Key>    key) {
    WriteEvent(
        static_cast<int>(HistoryEventType::PICKUP),
        key    ? (int)key->id       : 0,
        key    ? (std::string)key->name    : "",
        person ? (int)person->id    : 0,
        person ? (std::string)person->name : "",
        key    ? (int)key->pos      : 0
    );
}

void LogReturn(std::shared_ptr<kdb::Key> key) {
    WriteEvent(
        static_cast<int>(HistoryEventType::RETURN),
        key ? (int)key->id    : 0,
        key ? (std::string)key->name : "",
        0, "",
        key ? (int)key->pos   : 0
    );
}

void LogAdminOpen(std::shared_ptr<kdb::Person> person,
                  std::shared_ptr<kdb::Key>    key,
                  int                          pos) {
    WriteEvent(
        static_cast<int>(HistoryEventType::ADMIN_OPEN),
        key    ? (int)key->id       : 0,
        key    ? (std::string)key->name    : "",
        person ? (int)person->id    : 0,
        person ? (std::string)person->name : "",
        pos
    );
}

void LogKeyCreated(std::shared_ptr<kdb::Person> creator,
                   std::shared_ptr<kdb::Key>    key) {
    WriteEvent(
        static_cast<int>(HistoryEventType::KEY_CREATED),
        key     ? (int)key->id        : 0,
        key     ? (std::string)key->name     : "",
        creator ? (int)creator->id    : 0,
        creator ? (std::string)creator->name : "",
        key     ? (int)key->pos       : 0
    );
}

void LogKeyDeactivated(std::shared_ptr<kdb::Person> admin,
                       std::shared_ptr<kdb::Key>    key) {
    WriteEvent(
        static_cast<int>(HistoryEventType::KEY_DEACTIVATED),
        key   ? (int)key->id     : 0,
        key   ? (std::string)key->name  : "",
        admin ? (int)admin->id   : 0,
        admin ? (std::string)admin->name : "",
        key   ? (int)key->pos    : 0
    );
}

} // namespace history
