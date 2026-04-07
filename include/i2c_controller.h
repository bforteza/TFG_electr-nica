#ifndef I2C_CONTROLLER_H
#define I2C_CONTROLLER_H

#include "globals.h"
#include <functional>
#include <memory>

// Controlador de hardware del armario de llaves.
// Gestiona el chip XL9535 (expansor I2C de 16 bits) que acciona los relés:
//   - Matriz 4×8 de solenoides (32 posiciones)
//   - 1 relé de cerradura de acceso
//
// Restricción hardware: solo un solenoide puede estar activo simultáneamente.
// Se inicializa desde main() mediante hw_ctrl->Init().
class I2cController {
public:
    I2cController();
    ~I2cController();

    // Abre /dev/i2c-X, selecciona el esclavo XL9535 y ejecuta la secuencia
    // segura de arranque (pre-set outputs a LOW antes de cambiar a output mode).
    // Devuelve false si el bus no puede abrirse o falla alguna escritura.
    bool Init();

    // Activa el solenoide en pos (fila + columna simultáneamente).
    // No hace nada si ya hay otra posición activa (restricción hardware).
    void Activate(Position pos);

    // Desactiva el solenoide activo: fila off → delay kRelayOffDelayMs → columna off.
    // No hace nada si no hay ninguna posición activa.
    void Deactivate();

    // Abre/cierra el relé de la cerradura de acceso al armario.
    void OpenDoor();
    void CloseDoor();

    // Recorre los 16 pines del XL9535 uno a uno activando y desactivando cada relé.
    // on_step(relay_idx) se llama justo antes de activar el relé relay_idx (0–15).
    // Bloquea hasta completar el ciclo. Solo para uso en modo administrador.
    void RunRelayTest();

private:
    int      fd_;          // file descriptor de /dev/i2c-X
    bool     door_open_;   // estado actual de la cerradura
    Position active_pos_;  // posición actualmente abierta (N0 = ninguna)

    // Escribe el estado de los 16 pines al XL9535 en una sola transacción I2C.
    void WriteState(uint16_t state);

    // Devuelve el bit correspondiente al pin de cerradura si door_open_, 0 si no.
    uint16_t DoorBit() const;
};

// Controlador de hardware global. Se inicializa en main().
inline std::unique_ptr<I2cController> hw_ctrl = nullptr;

#endif // I2C_CONTROLLER_H
