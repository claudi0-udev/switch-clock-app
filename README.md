# Switch Clock Suite - Homebrew para Nintendo Switch

Una aplicación nativa para Nintendo Switch que integra **Reloj de sistema (RTC)**, **Cronómetro** con céntimas de segundo y **Temporizador** configurable.

---

## 🚀 Características

1. **Reloj (RTC)**: Muestra la fecha y hora actual leídas en tiempo real desde el hardware de la consola.
2. **Cronómetro**: Medidor de tiempo transcurrido con precisión de milisegundos (inicio, pausa y reinicio).
3. **Temporizador**: Cuenta regresiva programable (ajuste de minutos y segundos) con aviso de finalización.
4. **Navegación por Pestañas**: Cambio fluido de modo con los gatillos `L / R` o las flechas de dirección (`D-Pad`).
5. **Salida Segura**: Retorno al Homebrew Launcher (`hbmenu`) presionando el botón `(+)`.

---

## 📁 Estructura del Proyecto

```text
switch-clock-app/
├── .github/
│   └── workflows/
│       └── build.yml # Compilación automática en la nube mediante GitHub Actions
├── src/
│   └── main.c       # Código fuente C con lógica e interfaz de consola libnx
├── Makefile         # Archivo de compilación para devkitA64 / libnx
└── README.md        # Documentación de uso y compilación
```

---

## ☁️ Opción 1: Compilar en la Nube (Sin instalar nada)

Si no tienes `devkitPro` instalado en tu computadora:

1. Sube este proyecto a un repositorio propio en **GitHub**.
2. Ve a la pestaña **Actions** de tu repositorio.
3. El flujo de trabajo **Build Switch Homebrew** compilará automáticamente el proyecto utilizando el contenedor oficial `devkitpro/devkita64`.
4. Al finalizar el proceso, podrás descargar directamente el archivo listo **`SwitchClock-NRO.zip`** desde la sección **Artifacts** de GitHub.

---

## 🐳 Opción 2: Compilar con Docker en tu PC

Si tienes Docker instalado:

```bash
docker run --rm -v "$PWD":/automation -w /automation devkitpro/devkita64:latest bash -c "dkpro-pacman -Sy --noconfirm switch-dev switch-tools && make"
```

---

## 🛠️ Opción 3: Compilar Localmente con devkitPro

Si instalas **devkitPro** en tu sistema:

```bash
# Instalar los paquetes necesarios en devkitPro
dkpro-pacman -S switch-dev switch-tools

# Compilar
make
```

---

## 📱 Instalación en Nintendo Switch

1. Conecta la tarjeta MicroSD de tu Nintendo Switch a tu computadora (o usa FTP / DBI / Goldleaf).
2. Crea una carpeta llamada `SwitchClock` dentro del directorio `switch` de la tarjeta SD:
   ```text
   sdmc:/switch/SwitchClock/
   ```
3. Copia el archivo **`SwitchClock.nro`** generado en dicha carpeta:
   ```text
   sdmc:/switch/SwitchClock/SwitchClock.nro
   ```
4. Inserta la SD en tu consola, inicia tu Custom Firmware (**Atmosphère**), abre el **Homebrew Launcher** (`hbmenu`) y selecciona **Switch Clock Suite**.

---

## 🎮 Mapeo de Controles

| Botón | Función General |
| --- | --- |
| **L / R** o **D-Pad Izq / Der** | Cambiar pestaña (Reloj / Cronómetro / Temporizador) |
| **(+) PLUS** | Salir limpiamente de la app y volver a `hbmenu` |

### En la pestaña Cronómetro:
* **A**: Iniciar / Pausar cronómetro.
* **X**: Reiniciar a 00:00.00.

### En la pestaña Temporizador:
* **A**: Iniciar / Pausar cuenta regresiva.
* **X**: Reiniciar al tiempo configurado.
* **D-Pad Arriba**: Sumar 1 minuto (cuando está pausado).
* **D-Pad Abajo**: Restar 1 minuto (cuando está pausado).
* **Y**: Sumar 10 segundos (cuando está pausado).

---

## 📜 Licencia y Reconocimientos
Desarrollado con [libnx](https://github.com/switchbrew/libnx) de Switchbrew y devkitPro.
