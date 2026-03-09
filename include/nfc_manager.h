#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#if defined(HAVE_CONFIG_H)
    #include "config.h"
#endif

#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <nfc/nfc.h>
#include <glibmm/dispatcher.h>

// Gestiona la comunicación con el lector NFC (PN532 vía libnfc).
// Expone dos modos de uso:
//  - Polling en hilo separado: StartPolling()/StopPolling() + señal dispatcher.
//  - Detección síncrona: NfcDetect() bloqueante con timeout.
class NfcManager {
public:
    NfcManager();
    ~NfcManager();

    // Dispatcher GTK-safe: se emite desde el hilo de polling cuando se
    // detecta una tarjeta. El suscriptor debe leer uid para obtener el UID.
    Glib::Dispatcher dispatcher;

    // UID de la última tarjeta detectada en el hilo de polling.
    // Solo es válido en el instante en que dispatcher emite la señal.
    std::string uid;

    // Registra un callback alternativo al dispatcher (opcional).
    void SetCallback(std::function<void(const std::string&)> callback) {
        callback_ = callback;
    }

    // Arranca el hilo de polling NFC en segundo plano.
    // No hace nada si ya hay un hilo activo.
    void StartPolling();

    // Detiene el hilo de polling poniendo polling_ a false.
    // El hilo se desvincula (detach) y termina en la siguiente iteración.
    void StopPolling();

    // Detección síncrona bloqueante: espera hasta timeout_ms a que aparezca
    // una tarjeta ISO14443A. Devuelve el UID en hex mayúsculas, o "" si timeout.
    std::string NfcDetect(int timeout_ms = 1000);

private:
    // Contexto y dispositivo libnfc. Se inicializan en el constructor.
    nfc_context* context_;
    nfc_device*  device_;

    // Hilo de polling y flag de control. polling_ es atomic para
    // permitir escritura segura desde el hilo GTK y lectura desde worker_.
    std::thread       worker_;
    std::atomic<bool> polling_;

    // Callback alternativo al dispatcher (no usado actualmente).
    std::function<void(const std::string&)> callback_;

    // Bucle interno ejecutado por worker_. Llama a NfcDetect() repetidamente
    // y emite dispatcher cuando detecta una tarjeta nueva.
    void PollLoop();
};

#endif // NFC_MANAGER_H
