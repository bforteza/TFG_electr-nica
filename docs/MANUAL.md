# MANUAL.md — Documento de planificación del manual de usuario

> **Propósito de este archivo**
> Este documento es una referencia interna de planificación. Recoge el inventario de
> recursos disponibles, la estructura propuesta del manual, las decisiones de estilo y
> el estado de redacción de cada sección. No es el manual en sí.

---

## 1. Descripción del producto

**Armario Inteligente de Llaves** — sistema embebido de gestión física de llaves para
entornos empresariales. Corre sobre Raspberry Pi 4 con interfaz táctil de tipo kiosco.
Permite identificar usuarios y llaves mediante NFC o contraseña numérica, controlar
cerraduras solenoides, y mantener un historial de auditoría completo.

**Hardware principal:**
- Raspberry Pi 4 (2 GB) — unidad central
- Pantalla táctil IPS 7" 1024×600 — interacción
- Lector NFC PN532 (I2C) — identificación
- Expansor I2C XL9535 + módulo 16 relés — control de solenoides
- Teclado numérico USB — entrada alternativa
- Armario 4 filas × 8 columnas = 32 posiciones (A1–D8)

**Niveles de usuario:**

| Nivel | Nombre | Capacidades |
|-------|--------|-------------|
| 0 | Usuario básico | Ver sus llaves asignadas, retirar, devolver |
| 1 | Usuario medio | + crear/editar llaves, gestionar accesos de sus llaves |
| 2 | Administrador local | Control total: usuarios, todas las llaves, historial, solenoides directos |

---

## 2. Inventario de diagramas disponibles

Todos los PDFs están en `TFG_memoria/figuras/` (unificados — los que estaban en la
raíz se han movido y las referencias LaTeX se han actualizado en consecuencia).

| Archivo | Contenido | Útil para sección |
|---------|-----------|-------------------|
| `caso de uso.pdf` | Diagrama de casos de uso por actor y herencia | Introducción / Roles |
| `navegacion.drawio.pdf` | Todas las pantallas y transiciones, coloreadas por nivel de acceso | Descripción de la interfaz |
| `mtriusca-diagrama de flujo identificación.drawio.pdf` | Flujo completo de autenticación (NFC y contraseña) | Guía de uso — identificación |
| `Diagrama de clases.drawio.pdf` | Diagrama de clases del software (miembros públicos) | (técnico, opcional en apéndice) |
| `mtriusca-secuencia nfc.drawio.pdf` | Diagrama de secuencia del módulo NFC | (técnico, opcional en apéndice) |
| `ERD.drawio.pdf` | Entidad-Relación de la BD (inglés) | (técnico, opcional en apéndice) |
| `Copia de erd.drawio.pdf` | ERD variante multimáquina | (técnico, no incluir) |
| `diagrama comunicaciones.pdf` | Conexiones entre componentes HW | Descripción del sistema HW |
| `diagrama conexiones.pdf` | Esquema de comunicaciones alternativo | (revisar si es diferente al anterior) |
| `diagrama potencia.pdf` | Circuito de alimentación | (técnico, opcional en apéndice) |
| `Esquema.pdf` | Esquemático completo de distribución de solenoides (3F×6C simplificado) | Descripción HW / apéndice |
| `Esquema-pagina proximidad.pdf` | Módulo individual solenoide+LED+diodo | Descripción HW |
| `keybox.png` / `keycafe.png` | Fotos de productos similares del mercado | (no incluir, son de estado del arte) |

**Capturas de pantalla:** aún no disponibles — hay un TODO en `software.tex §Resultado`.
Se deben tomar cuando el sistema esté completamente terminado.

---

## 3. Estructura propuesta del manual

```
MANUAL DE USUARIO — Armario Inteligente de Llaves
│
├── 0. Portada / Cabecera
│     Logo, nombre del producto, versión, fecha
│
├── 1. Introducción
│     - Qué es el sistema y para qué sirve
│     - Componentes físicos (foto/diagrama del armario)
│     - Niveles de acceso (tabla resumen)
│     - Formas de identificación (NFC y contraseña)
│
├── 2. Primeros pasos
│     - Encendido del sistema
│     - Pantalla de inicio (LoginStack)
│     - Cómo identificarse con NFC
│     - Cómo identificarse con contraseña numérica
│     - Sesión automática / cierre por inactividad (30 s)
│
├── 3. Uso diario — Usuario básico
│     3.1 Ver mis llaves asignadas
│     3.2 Retirar una llave
│         - Seleccionar llave → solenoide abre → retirar
│         - Qué ocurre si la llave ya la tiene otro usuario
│     3.3 Devolver una llave
│         - Acercar el llavero NFC → solenoide abre → depositar
│
├── 4. Gestión de llaves — Usuario medio
│     4.1 Crear una nueva llave
│         - Datos: nombre, ubicación, comentario, posición en armario
│         - Asignar tag NFC a la llave
│         - Seleccionar posición física (panel de solenoides modo SELECT)
│     4.2 Editar una llave
│     4.3 Gestionar quién tiene acceso a una llave
│     4.4 Activar / desactivar una llave
│
├── 5. Administración — Administrador local
│     5.1 Gestión de usuarios
│         - Crear usuario (nombre, contraseña, NFC, nivel)
│         - Editar / desactivar usuario
│         - Ver historial de un usuario
│     5.2 Gestión completa de llaves
│         (igual que §4 pero sobre todas las llaves del sistema)
│     5.3 Historial de acciones
│         - Historial global
│         - Filtrado por llave o por usuario
│     5.4 Control directo de solenoides (modo ADMIN)
│         - Abrir / cerrar posiciones manualmente
│         - Código de colores: verde = abierto, rojo = ocupado, gris = vacío
│
├── 6. Configuración
│     6.1 Cambio de idioma (Español / Català / English)
│
├── 7. Solución de problemas
│     - NFC no reconoce la tarjeta
│     - Contraseña incorrecta
│     - Llave ya en posesión de otro usuario
│     - Solenoide no responde
│
└── 8. Apéndice (opcional)
      - Tabla de posiciones del armario (A1–D8)
      - Glosario
```

---

## 4. Estilo y tono

- **Idioma:** Español (versiones en catalán/inglés fuera de alcance por ahora).
- **Destinatario:** Administrador del sistema — conocimiento completo: flujos de uso,
  estructura de la base de datos, niveles de acceso, hardware. Sin código fuente.
- **Tono:** técnico pero claro; el lector conoce sistemas informáticos pero no
  necesariamente el código. Términos del dominio (solenoide, NFC, historial) se usan
  con normalidad; se explican en el glosario.
- **Persona gramatical:** segunda persona formal ("Seleccione", "Introduzca").
- **Formato de pasos:** siempre numerados, una acción por paso.
- **Notas y advertencias:** bloques diferenciados (`> **Nota:**` / `> **Aviso:**`).
- **Imágenes:** capturas de pantalla a añadir progresivamente. Las mismas capturas
  se reutilizarán en la memoria del TFG (§ Interfaz de usuario). El manual vive en
  el repositorio `TFG_electrónica`, no en `TFG_memoria`.
- **Longitud objetivo:** 20–30 páginas en PDF final.

---

## 5. Formato de salida — DECIDIDO

**Fuente:** Markdown (`.md`) en este repositorio (`TFG_electrónica/docs/`).
**Salida:** PDF generado con la extensión **Markdown PDF** de VS Code (o Pandoc CLI).

El Markdown es editable aquí colaborativamente; el PDF se genera en un clic cuando
se quiera una versión entregable. Las imágenes se referencian con rutas relativas y
se comparten con la memoria LaTeX del TFG.

---

## 6. Estado de redacción

| Sección | Estado |
|---------|--------|
| 0. Portada | ⬜ Pendiente |
| 1. Introducción | ⬜ Pendiente |
| 2. Primeros pasos | ⬜ Pendiente |
| 3. Uso diario (básico) | ⬜ Pendiente |
| 4. Gestión de llaves (medio) | ⬜ Pendiente |
| 5. Administración | ⬜ Pendiente |
| 6. Configuración | ⬜ Pendiente |
| 7. Solución de problemas | ⬜ Pendiente |
| 8. Apéndice | ⬜ Pendiente |

> Leyenda: ⬜ Pendiente · 🟡 En progreso · ✅ Completado

---

## 7. Pendientes antes de escribir

- [ ] Confirmar formato de salida (PDF / HTML / ambos)
- [ ] Confirmar idioma(s) del manual
- [ ] Tomar capturas de pantalla de todas las pantallas principales
- [ ] Decidir si incluir sección técnica/apéndice o solo guía de usuario
- [ ] Confirmar nombre oficial del producto (aparece como "Armario Inteligente de Llaves"
      en el TFG, pero puede haber un nombre comercial)
