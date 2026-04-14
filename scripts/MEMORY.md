# Scripts de Arranque — Armario de Llaves Inteligente

Conjunto de scripts y unidades systemd para que la Raspberry Pi arranque
automáticamente en modo producción: app GTK a pantalla completa, webserver
Flask activo y Raspi Connect disponible para mantenimiento remoto.

---

## Arquitectura de arranque

```
Boot (systemd)
 ├── mariadb.service              ← BD lista (habilitado en install.sh §4)
 ├── armario-webserver.service    ← Flask en :5000  (After: mariadb + network)
 ├── rpi-connect-update.service   ← actualiza rpi-connect si hay versión nueva
 ├── rpi-connect (user service)   ← acceso remoto vía Raspberry Pi Connect
 └── lightdm.service              ← autologin usuario configurado en install.sh
      └── LXDE desktop
           └── ~/.config/autostart/armario-kiosk.desktop
                └── armario-kiosk-session.sh
                     ├── lanza bin/Debug/Pruebas2 (fullscreen, sin decoración)
                     └── si crash (exit≠0): espera 2s y relanza
                         si ESC   (exit=0): sale → escritorio LXDE accesible
```

---

## Ficheros

### `armario-webserver.service`

Unidad systemd para el webserver Flask.

- **Tipo**: `simple`, reinicio automático en fallo (`Restart=on-failure`)
- **Dependencias**: `mariadb.service` + `network.target`
- **Contiene marcadores** sustituidos por `install.sh`:
  - `__USER__` → usuario que ejecutó el script
  - `__INSTALL_DIR__` → path del repositorio (por defecto `$HOME/TFG_electr-nica`)
- **Instalado en**: `/etc/systemd/system/armario-webserver.service`
- **Comandos útiles**:
  ```bash
  sudo systemctl status armario-webserver
  sudo journalctl -u armario-webserver -f
  sudo systemctl restart armario-webserver
  ```

### `rpi-connect-update.service`

Unidad systemd `oneshot` que comprueba y aplica actualizaciones de Raspi Connect
en cada arranque, una vez que hay red disponible.

- **Tipo**: `oneshot` (corre una vez al arrancar y termina)
- **Dependencias**: `network-online.target`
- **Lógica**: `apt-get update -qq && apt-get install --only-upgrade -y rpi-connect`
- Si no hay actualización disponible, termina en segundos sin efecto.
- La nueva versión de rpi-connect se activa en el siguiente reinicio del servicio.
- **Instalado en**: `/etc/systemd/system/rpi-connect-update.service`

### `kiosk-session.sh`

Wrapper de arranque de la app GTK. Lógica de reintento basada en exit code:

| Exit code | Causa | Comportamiento |
|-----------|-------|----------------|
| `0` | ESC → `get_application()->quit()` | Sale del loop → escritorio LXDE |
| `≠ 0` | Crash inesperado | Espera 2 s y relanza |

- **Contiene marcador**: `__INSTALL_DIR__` → sustituido por `install.sh`
- **Instalado en**: `/usr/local/bin/armario-kiosk-session.sh`

### `armario-kiosk.desktop`

Entrada de autostart para LXDE/Pi OS. Se copia a `~/.config/autostart/` y hace
que el escritorio lance `armario-kiosk-session.sh` al iniciar sesión gráfica.

- **Instalado en**: `~/.config/autostart/armario-kiosk.desktop`
- No requiere sustitución de marcadores (referencia path fijo `/usr/local/bin/`).

---

## Configuración de sistema generada por `install.sh`

### `/etc/lightdm/lightdm.conf.d/50-armario-autologin.conf`

Drop-in de lightdm que configura el autologin sin tocar la configuración existente:
```ini
[Seat:*]
autologin-user=<usuario>
autologin-user-timeout=0
```
Usa la sesión gráfica por defecto de Pi OS (LXDE). La app ya gestiona
fullscreen/sin decoración por sí misma (`set_decorated(false)` + `fullscreen()`
en `src/window.cpp`).

---

## Mantenimiento

### Con teclado físico (sin red)
```
ESC  →  app cierra con exit 0  →  escritorio LXDE accesible con cursor
```
Para volver a la app: `sudo reboot` o ejecutar manualmente
`/usr/local/bin/armario-kiosk-session.sh`.

### Remoto con Raspi Connect / SSH
```bash
pkill Pruebas2          # cierra la app (exit≠0 → el wrapper la relanza)
# o bien:
pkill -TERM Pruebas2    # envía SIGTERM → GTK lo trata como exit 0 → no relanza
sudo reboot             # para un ciclo completo limpio
```

### Raspi Connect — paso manual obligatorio (solo una vez)
Tras ejecutar `install.sh`, vincular el dispositivo a la cuenta:
```bash
rpi-connect signin
```

---

## WiFi

La configuración WiFi se hace **manualmente** fuera de `install.sh`.
La red de producción es **SALJUB** con IP estática `192.168.1.9`.

```bash
# 1. Conectarse primero a la red (si no está conectada ya)
sudo nmcli device wifi connect "SALJUB" password "<CONTRASEÑA>"

# 2. Configurar IP estática
sudo nmcli connection modify "SALJUB" \
    ipv4.method      manual \
    ipv4.addresses   "192.168.1.9/24" \
    ipv4.gateway     "192.168.1.1" \
    ipv4.dns         "8.8.8.8,1.1.1.1" \
    connection.autoconnect yes \
    connection.autoconnect-retries -1

# 3. Reiniciar para que la IP estática tome efecto
sudo reboot
```

> `nmcli connection up` no siempre es suficiente en Pi OS — el reboot
> garantiza que arranca limpio sin restos de la sesión DHCP anterior.

---

## Instalación completa desde cero

```bash
git clone -b Implementacion https://github.com/bforteza/TFG_electr-nica
cd TFG_electr-nica
chmod +x install.sh && ./install.sh
# Al terminar: compilar el proyecto con Code::Blocks y hacer sudo reboot
rpi-connect signin   # una sola vez
```
