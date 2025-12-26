#include "NfcManager.h"

NfcManager::NfcManager() : context(nullptr), device(nullptr), polling(false)
{
    nfc_init(&context);
    if(!context){
        std::cerr << "Error: no se pudo inicializar libnfc\n";
        return;
    }
    device = nfc_open(context, nullptr);
    if(!device){
        std::cerr <<"Error: no se ha encontrado dispositivo nfc \n";
    }
    if(nfc_initiator_init(device)< 0){
        std::cerr <<"Error: no se pudo inicializar dispositivo \n";
        nfc_close(device);
        nfc_exit(context);
    }
    if (verbose){        std::cout <<"Inicializado lector nfc correctamente\n";
    }


}

NfcManager::~NfcManager()
{
    if(device) nfc_close(device);
    if(context) nfc_exit(context);
}

std::string NfcManager::NfcDetect(int timeoutMilliseconds){
    auto StartTime = std::chrono::steady_clock::now();
    nfc_target target;
    nfc_modulation mod[2]={
    {.nmt= NMT_ISO14443A, .nbr= NBR_106}};
    uint8_t uiPolNr = 2;
    uint8_t uiPeriod = 2;
    size_t Mod = 1;
    while (true){

        int res = nfc_initiator_poll_target(device, mod,Mod,uiPolNr,uiPeriod,&target);
        std::ostringstream uid;
        uid << std::hex<<std::uppercase << std::setfill('0');
        if (res){
            for (size_t i = 0; i < target.nti.nai.szUidLen; i++){
                uid << std::setw(2) << (int) target.nti.nai.abtUid[i];
            }
            return uid.str();
        }
        auto NowTime = std::chrono::steady_clock::now();

        if(std::chrono::duration_cast<std::chrono::milliseconds>(NowTime-StartTime).count() >= timeoutMilliseconds){
            return "";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
// Polling loop for thread
void NfcManager::pollLoop(){
    std::string aux = NfcDetect(1);
    while(polling){
        aux = NfcDetect(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(aux.size() > 0  ){
             Uid = aux;
             aviso.emit();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }

}

//Polling starting thread

void NfcManager::startPolling(){
    if(polling || worker.joinable()) return;
    polling = true;
    worker = std::thread(&NfcManager::pollLoop, this);
}

//Stop polling thread

void NfcManager::stopPolling(){
    polling = false;
    worker.detach();
}
