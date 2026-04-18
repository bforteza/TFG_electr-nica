#include "app_logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace {

constexpr const char* kLogPath    = "../../logs/armario.log";
constexpr const char* kLogBakPath = "../../logs/armario.log.bak";
constexpr uintmax_t   kMaxBytes   = 5u * 1024u * 1024u;

std::mutex    g_mutex;
std::ofstream g_file;

std::string Timestamp() {
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Debe llamarse con g_mutex ya adquirido.
void RotateIfNeeded() {
    try {
        if (std::filesystem::file_size(kLogPath) < kMaxBytes)
            return;
    } catch (...) {
        return;
    }
    g_file.close();
    std::error_code ec;
    std::filesystem::rename(kLogPath, kLogBakPath, ec);
    g_file.open(kLogPath, std::ios::app);
}

void Write(const char* level, const char* source, const std::string& msg) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_file.is_open())
        return;
    RotateIfNeeded();
    g_file << Timestamp()
           << " [" << std::left << std::setw(5) << level << "] "
           << "[" << std::left << std::setw(7) << source << "] "
           << msg << "\n";
    g_file.flush();
}

// Devuelve true si la última línea de [APP] del log contiene "APPLICATION STOP".
// Ignora líneas de otras fuentes (WEB, I2C, etc.) que pueden haberse escrito
// después del cierre del C++ mientras el webserver seguía activo.
// Llamar antes de abrir g_file.
bool LastLineIsStop() {
    std::ifstream f(kLogPath);
    if (!f.is_open())
        return true;  // primera ejecución, no hay crash previo
    std::string last_app_line, line;
    while (std::getline(f, line))
        if (line.find("[APP    ]") != std::string::npos)
            last_app_line = line;
    if (last_app_line.empty())
        return true;  // nunca hubo línea APP → no hay crash previo
    return last_app_line.find("APPLICATION STOP") != std::string::npos;
}

}  // namespace

void AppLogger::Init() {
    try {
        std::filesystem::create_directories("../../logs");
    } catch (...) {}

    bool crash = !LastLineIsStop();

    std::lock_guard<std::mutex> lock(g_mutex);
    g_file.open(kLogPath, std::ios::app);
    if (!g_file.is_open())
        return;

    g_file << Timestamp() << " [INFO ] [APP    ] APPLICATION START\n";
    if (crash)
        g_file << Timestamp() << " [WARN ] [APP    ] Previous session ended unexpectedly\n";
    g_file.flush();
}

void AppLogger::Shutdown(ShutdownReason reason) {
    const char* label = nullptr;
    switch (reason) {
        case ShutdownReason::kEsc:      label = "ESC (maintenance)"; break;
        case ShutdownReason::kShutdown: label = "SHUTDOWN";          break;
        case ShutdownReason::kReboot:   label = "REBOOT";            break;
    }
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_file.is_open())
        return;
    g_file << Timestamp() << " [INFO ] [APP    ] APPLICATION STOP: " << label << "\n";
    g_file.flush();
    g_file.close();
}

void AppLogger::Info(const char* source, const std::string& msg) {
    Write("INFO", source, msg);
}

void AppLogger::Error(const char* source, const std::string& msg) {
    Write("ERROR", source, msg);
}
