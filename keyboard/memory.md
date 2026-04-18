# Teclado Virtual — Documentación

## Uso desde otros programas

El teclado se lanza como subproceso. Imprime el texto introducido por el usuario en `stdout` al pulsar Intro, y sale. El programa llamante lee ese `stdout` para obtener el valor.

### Argumentos

| Argumento | Obligatorio | Descripción |
|-----------|-------------|-------------|
| `--title` | No | Título/etiqueta que se muestra sobre el campo de texto |
| `--text`  | No | Texto inicial (para editar un valor existente) |

### Ejemplos de llamada

**Bash:**
```bash
resultado=$(DISPLAY=:0 python3 /ruta/keyboard/keyboard.py --title "Nombre" --text "Juan")
echo "El usuario escribió: $resultado"
```

**Python:**
```python
import subprocess

result = subprocess.run(
    ['python3', '/ruta/keyboard/keyboard.py', '--title', 'Nombre', '--text', 'Juan'],
    capture_output=True, text=True,
    env={**os.environ, 'DISPLAY': ':0'}
)
texto = result.stdout.strip()
```

**C++ (GTK app):**
```cpp
#include <cstdio>

std::string open_keyboard(const std::string& title, const std::string& initial = "") {
    std::string cmd = "DISPLAY=:0 python3 /ruta/keyboard/keyboard.py"
                      " --title '" + title + "' --text '" + initial + "'";
    FILE* pipe = popen(cmd.c_str(), "r");
    char buffer[512] = {};
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
    // Quitar salto de línea final
    std::string result(buffer);
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}
```

### Comportamiento en casos especiales

- Si el usuario cierra la ventana sin pulsar Intro, `stdout` queda vacío y el proceso sale con código 0.
- Si `--text` contiene el texto anterior, el usuario puede editarlo; el resultado siempre es el texto completo final.

---

## Dependencias

| Dependencia | Versión mínima | Instalación |
|-------------|----------------|-------------|
| Python      | 3.8            | preinstalado en Raspberry Pi OS |
| PyGObject   | 3.x            | `sudo apt install python3-gi python3-gi-cairo gir1.2-gtk-3.0` |
| GTK         | 3.x            | `sudo apt install libgtk-3-0` (normalmente preinstalado) |

No requiere ningún paquete pip. Solo PyGObject del sistema.

**Comprobación rápida:**
```bash
python3 -c "import gi; gi.require_version('Gtk','3.0'); from gi.repository import Gtk; print('OK')"
```

---

## Funcionamiento interno

### Archivos

```
keyboard/
  keyboard.py   — lógica principal (clase KeyboardWindow)
  keyboard.css  — estilos visuales (colores, tamaños de fuente)
  memory.md     — esta documentación
```

### Clase `KeyboardWindow`

Hereda de `Gtk.Window`. Al construirse:
1. Pone la ventana en fullscreen sin decoraciones.
2. Carga `keyboard.css` via `Gtk.CssProvider`.
3. Construye la UI: título opcional → campo `Gtk.Entry` → 3 filas de teclas → fila inferior.

Cada tecla es un `Gtk.Button` con la clase CSS `.key`. Los botones especiales reciben clases adicionales:
- `.key-shift`     — teclas ⇧
- `.key-dead`      — teclas de acento muerto (´ y `)
- `.key-clear`     — botón "Borrar todo"
- `.key-backspace` — botón ⌫ (borra la última letra introducida)
- `.key-enter`     — botón "Intro"

### Estado interno

| Variable | Tipo | Descripción |
|----------|------|-------------|
| `shift_state` | int | 0 = off, 1 = una mayúscula, 2 = caps lock |
| `dead_key` | str\|None | Tecla muerta pendiente (`'´'` o `` '`' ``), `None` si no hay ninguna |
| `shift_buttons` | list | Referencias a los botones ⇧ para actualizar su estilo |
| `letter_buttons` | list | Pares `(button, char_minúscula)` para actualizar etiquetas con shift |
| `dead_buttons` | dict | `char → button` para marcar visualmente la tecla muerta activa |

### Flujo de una pulsación

```
on_key(key)
  ├── '⌫'  → borra carácter anterior al cursor en el Entry
  ├── '⇧'  → avanza shift_state (0→1→2→0), actualiza estilos y etiquetas
  ├── dead  → activa/desactiva dead_key, marca tecla en amarillo
  └── otro → aplica shift si procede
              aplica dead_key si hay una pendiente (busca en tabla o concatena)
              inserta carácter en Entry en la posición del cursor
              si shift_state==1, lo resetea a 0
```

### Teclas muertas

Al pulsar ´ o `, se guarda en `self.dead_key`. La siguiente tecla consulta la tabla correspondiente:

| Dead key | + vocal | resultado |
|----------|---------|-----------|
| ´        | a/e/i/o/u | á/é/í/ó/ú (mayúsculas si shift activo) |
| `        | a/e/i/o/u | à/è/ì/ò/ù (mayúsculas si shift activo) |

Si la vocal no está en la tabla, se insertan los dos caracteres literales.
Una segunda pulsación sobre la misma tecla muerta la cancela.
