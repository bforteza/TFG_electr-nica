#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <array>
#include <cstdint>

// --- Bus I2C ---
constexpr int     kI2cBus          = 1;     // /dev/i2c-1
constexpr uint8_t kI2cAddress      = 0x20;  // XL9535 (A0=A1=A2=GND)

// --- Timing ---
constexpr int kRelayOffDelayMs     = 50;    // delay al cerrar (protección flyback)
constexpr int kRelayTestDwellMs    = 200;   // tiempo activo por relé durante el test
constexpr int kI2cRetries          = 5;     // reintentos por escritura I2C
constexpr int kI2cRetryDelayMs     = 8;     // delay entre reintentos (ms)
constexpr int kDoorOpenMs          = 200;   // tiempo puerta abierta (ms)
constexpr int kDoorWaitMs          = 300;   // espera entre puerta y solenoide (ms)
constexpr int kPickupSolenoidMs    = 3000;  // tiempo solenoide en PICKUP (ms)
constexpr int kReturnSolenoidMs    = 2000;  // tiempo solenoide en RETURN (ms)
constexpr int kRepeatWaitSecs      = 10;    // segundos de espera para repetir

// --- Asignación de pines XL9535 (0–15) ---
// Ajustar según el cableado real de la PCB.
constexpr std::array<int, 4> kRowPins = {9, 10, 11, 12};              // filas A, B, C, D
constexpr std::array<int, 8> kColPins = {4, 3, 2, 1, 0, 15, 14, 13}; // columnas 1–8
constexpr int kDoorPin               = 8;  // relé cerradura de acceso

// --- Registros del XL9535 (compatible PCA9535) ---
constexpr uint8_t kRegOutputPort0  = 0x02;
constexpr uint8_t kRegOutputPort1  = 0x03;
constexpr uint8_t kRegConfigPort0  = 0x06;
constexpr uint8_t kRegConfigPort1  = 0x07;

#endif // HARDWARE_CONFIG_H
