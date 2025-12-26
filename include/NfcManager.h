#ifndef NFCMANAGER_H
#define NFCMANAGER_H



#if defined(HAVE_CONFIG_H)
    #include "config.h"
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <nfc/nfc.h>
#include <functional>
//#include <freefare.h>
#include <string>
#include <glibmm/dispatcher.h>

#define LANG_CODE "es"

class NfcManager
{
    public:

        NfcManager();
        ~NfcManager();
        bool verbose = true;
        Glib::Dispatcher aviso;
        void setCallback(std::function<void(const std::string& uid)> callb){
            callback = callb;
        };
        void startPolling();
        void stopPolling();
        std::string Uid;
        std::string NfcDetect(int timeoutMilliseconds = 1000);
    protected:
        nfc_context* context;
        nfc_device* device;

        std::thread worker{};
        std::atomic<bool> polling;
        std::function<void(const std::string& uid)> callback;

        void pollLoop();
    private:
};

#endif // NFCMANAGER_H
