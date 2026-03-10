#include "nfc_manager.h"
#include <iostream>
#include <iomanip>
#include <sstream>

NfcManager::NfcManager() : context_(nullptr), device_(nullptr), polling_(false) {
    nfc_init(&context_);
    if (!context_) {
        std::cerr << "NfcManager: no se pudo inicializar libnfc\n";
        return;
    }
    device_ = nfc_open(context_, nullptr);
    if (!device_) {
        std::cerr << "NfcManager: no se ha encontrado dispositivo NFC\n";
        return;
    }
    if (nfc_initiator_init(device_) < 0) {
        std::cerr << "NfcManager: no se pudo inicializar el dispositivo\n";
        nfc_close(device_);
        nfc_exit(context_);
        device_  = nullptr;
        context_ = nullptr;
        return;
    }
    std::cout << "NfcManager: lector NFC inicializado correctamente\n";
}

NfcManager::~NfcManager() {
    if (device_)  nfc_close(device_);
    if (context_) nfc_exit(context_);
}

std::string NfcManager::NfcDetect(int timeout_ms) {
    auto start_time = std::chrono::steady_clock::now();
    nfc_target target;
    nfc_modulation mod[] = {{.nmt = NMT_ISO14443A, .nbr = NBR_106}};

    while (true) {
        int res = nfc_initiator_poll_target(device_, mod, 1, 2, 2, &target);
        if (res) {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setfill('0');
            for (size_t i = 0; i < target.nti.nai.szUidLen; i++)
                oss << std::setw(2) << (int)target.nti.nai.abtUid[i];
            return oss.str();
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time).count();
        if (elapsed >= timeout_ms)
            return "";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void NfcManager::PollLoop() {
    while (polling_) {
        std::string detected = NfcDetect(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!detected.empty()) {
            uid = detected;
            dispatcher.emit();
            // Pausa extra para evitar detecciones repetidas de la misma tarjeta.
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void NfcManager::StartPolling() {
    if (!device_ || polling_ || worker_.joinable()) return;
    polling_ = true;
    worker_ = std::thread(&NfcManager::PollLoop, this);
}

void NfcManager::StopPolling() {
    polling_ = false;
    worker_.detach();
}
