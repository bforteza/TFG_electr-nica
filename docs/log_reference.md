# Referencia del Log del Sistema — armario.log

Archivo: `~/TFG_electr-nica/logs/armario.log`
Backup:  `~/TFG_electr-nica/logs/armario.log.bak` (se crea al superar 5 MB)

## Formato de línea

```
2026-04-18 10:32:45 [INFO ] [APP    ] APPLICATION START
└─── timestamp ────┘ └lvl┘  └source┘  └─── mensaje ────────────────
```

- **LEVEL**: `INFO ` · `ERROR` · `WARN `
- **SOURCE**: `APP    ` · `DB     ` · `I2C    ` · `NFC    ` · `WEB    `

---

## Eventos por fuente

### `[APP]` — Ciclo de vida de la aplicación C++

| Nivel  | Mensaje | Cuándo ocurre |
|--------|---------|---------------|
| INFO   | `APPLICATION START` | Cada vez que arranca el programa (inicio de `on_startup`) |
| WARN   | `Previous session ended unexpectedly` | El arranque detecta que la sesión anterior no tiene `APPLICATION STOP`. Indica crash o apagado brusco de la Pi |
| INFO   | `APPLICATION STOP: ESC (maintenance)` | El operador pulsó ESC para volver al escritorio (mantenimiento local) |
| INFO   | `APPLICATION STOP: SHUTDOWN` | El operador confirmó el apagado desde `ShutdownButton` en la pantalla de login (`LoginStack::OnShutdownClicked`) |
| INFO   | `APPLICATION STOP: REBOOT` | El operador confirmó el reinicio desde `RebootButton` en la pantalla de login (`LoginStack::OnRebootClicked`) |

---

### `[DB]` — Base de datos MariaDB

| Nivel  | Mensaje | Cuándo ocurre |
|--------|---------|---------------|
| INFO   | `Connection OK` | LiteSQL se conectó correctamente a MariaDB al arrancar |
| ERROR  | `Connection error: <detalle>` | LiteSQL lanzó `litesql::Except` al intentar conectar. La app termina. Causas típicas: MariaDB no está corriendo, credenciales incorrectas, base de datos no existe |

---

### `[I2C]` — Controlador hardware XL9535

| Nivel  | Mensaje | Cuándo ocurre |
|--------|---------|---------------|
| INFO   | `Init OK` | `I2cController::Init()` abrió `/dev/i2c-1`, configuró el XL9535 y puso todos los relés a LOW |
| ERROR  | `Init failed: no se pudo abrir /dev/i2c-1` | `open("/dev/i2c-1")` o `ioctl(I2C_SLAVE)` fallaron. Causas: bus I2C deshabilitado en `raspi-config`, cable suelto, módulo `i2c-dev` no cargado |
| ERROR  | `WriteReg failed: reg=0x<XX> tras N intentos` | Fallo de escritura I2C tras `kI2cRetries` reintentos. Ocurre durante `Init`, `Activate`, `Deactivate`, `OpenDoor` o `CloseDoor`. Causas: XL9535 sin alimentación, fallo de bus I2C en caliente |

---

### `[NFC]` — Lector NFC PN532

| Nivel  | Mensaje | Cuándo ocurre |
|--------|---------|---------------|
| INFO   | `lector NFC inicializado correctamente` | `nfc_initiator_init` OK. El lector está listo para detectar tarjetas |
| ERROR  | `no se pudo inicializar libnfc` | `nfc_init` devolvió `nullptr`. libnfc no pudo obtener contexto. Causa: librería no instalada o permisos insuficientes |
| ERROR  | `no se ha encontrado dispositivo NFC` | `nfc_open` devolvió `nullptr`. El PN532 no responde en I2C. Causas: cable suelto, dirección I2C incorrecta, driver no cargado |
| ERROR  | `no se pudo inicializar el dispositivo` | `nfc_initiator_init` devolvió < 0. El dispositivo se abrió pero no responde a comandos. Causa: firmware del PN532 en estado incorrecto, reset necesario |

---

### `[WEB]` — Webserver Flask

| Nivel  | Mensaje | Cuándo ocurre |
|--------|---------|---------------|
| INFO   | `SERVER START` | Flask arranca y empieza a escuchar en el puerto 5000 |
| INFO   | `SERVER STOP` | Flask se detiene limpiamente (systemd stop, SIGTERM) |
| INFO   | `Network OK` | Al arrancar el webserver, se detecta al menos una interfaz de red activa (`operstate = up`) |
| ERROR  | `Network DOWN at startup` | El webserver arrancó pero ninguna interfaz de red está activa. Los clientes no podrán conectarse |
| INFO   | `Network UP` | La red se recuperó después de haber estado caída (transición DOWN→UP detectada por el hilo monitor) |
| ERROR  | `Network DOWN` | La red cayó mientras el webserver estaba corriendo (transición UP→DOWN detectada por el hilo monitor cada 30 s) |
| ERROR  | `DB connection error: <detalle>` | PyMySQL no pudo conectar a MariaDB al atender una petición HTTP. Causas: MariaDB caído, credenciales incorrectas en `config.py` |

---

## Ejemplo de sesión normal

```
2026-04-18 08:00:01 [INFO ] [APP    ] APPLICATION START
2026-04-18 08:00:01 [INFO ] [DB     ] Connection OK
2026-04-18 08:00:01 [INFO ] [NFC    ] lector NFC inicializado correctamente
2026-04-18 08:00:01 [INFO ] [I2C    ] Init OK
2026-04-18 08:00:03 [INFO ] [WEB    ] SERVER START
2026-04-18 08:00:03 [INFO ] [WEB    ] Network OK
2026-04-18 17:45:22 [INFO ] [APP    ] APPLICATION STOP: ESC (maintenance)
2026-04-18 17:45:22 [INFO ] [WEB    ] SERVER STOP
```

## Ejemplo con incidencias

```
2026-04-18 08:00:01 [INFO ] [APP    ] APPLICATION START
2026-04-18 08:00:01 [WARN ] [APP    ] Previous session ended unexpectedly
2026-04-18 08:00:01 [INFO ] [DB     ] Connection OK
2026-04-18 08:00:01 [ERROR] [NFC    ] no se ha encontrado dispositivo NFC
2026-04-18 08:00:01 [INFO ] [I2C    ] Init OK
2026-04-18 08:00:03 [INFO ] [WEB    ] SERVER START
2026-04-18 08:00:03 [ERROR] [WEB    ] Network DOWN at startup
2026-04-18 08:01:15 [INFO ] [WEB    ] Network UP
2026-04-18 10:32:44 [ERROR] [I2C    ] WriteReg failed: reg=0x02 tras 3 intentos
```
