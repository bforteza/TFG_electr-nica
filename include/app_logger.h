#ifndef APP_LOGGER_H
#define APP_LOGGER_H

#include <string>

// Logger persistente de sistema. Escribe en ../../logs/armario.log (relativo al
// ejecutable en bin/Debug/). Thread-safe: usa mutex interno.
// Rota el archivo al superar 5 MB (renombra a armario.log.bak).
class AppLogger {
public:
    enum class ShutdownReason { kEsc, kShutdown, kReboot };

    // Abre el archivo de log, crea el directorio si no existe, y registra START.
    // Si la sesión anterior no terminó con STOP, registra un aviso de crash.
    static void Init();

    // Registra APPLICATION STOP con el motivo y cierra el stream.
    static void Shutdown(ShutdownReason reason);

    static void Info (const char* source, const std::string& msg);
    static void Error(const char* source, const std::string& msg);

    AppLogger() = delete;
};

#endif // APP_LOGGER_H
