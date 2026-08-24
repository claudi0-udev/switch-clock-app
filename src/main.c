#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <switch.h>

// Definición de pestañas
typedef enum {
    TAB_CLOCK = 0,
    TAB_STOPWATCH = 1,
    TAB_TIMER = 2,
    TAB_COUNT
} TabMode;

// Función para obtener tiempo en milisegundos desde el sistema ARM
static u64 get_time_ms(void) {
    u64 freq = armGetSystemTickFreq();
    if (freq == 0) return 0;
    return (armGetSystemTick() * 1000) / freq;
}

// Limpiar la pantalla de la consola
static void clear_screen(void) {
    printf("\x1b[2J\x1b[1;1H");
}

int main(int argc, char **argv) {
    // Inicializar la consola en modo texto (80x45 caracteres aprox.)
    consoleInit(NULL);

    // Configurar entrada de Joy-Cons / Pro Controller
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    TabMode current_tab = TAB_CLOCK;

    // --- Estado del Cronómetro ---
    bool sw_running = false;
    u64 sw_start_ms = 0;
    u64 sw_accumulated_ms = 0;

    // --- Estado del Temporizador ---
    bool timer_running = false;
    u64 timer_duration_ms = 5 * 60 * 1000; // Por defecto: 5 minutos
    u64 timer_remaining_ms = 5 * 60 * 1000;
    u64 timer_last_update_ms = 0;
    bool timer_finished = false;

    clear_screen();

    // Bucle principal de la aplicación (mientras el SO permita ejecutarse)
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        // Salir de la aplicación con (+)
        if (kDown & HidNpadButton_Plus) {
            break;
        }

        // --- Navegación entre Pestañas (L / R o D-Pad Izq / Der) ---
        if (kDown & (HidNpadButton_R | HidNpadButton_Right)) {
            current_tab = (current_tab + 1) % TAB_COUNT;
            clear_screen();
        }
        if (kDown & (HidNpadButton_L | HidNpadButton_Left)) {
            current_tab = (current_tab + TAB_COUNT - 1) % TAB_COUNT;
            clear_screen();
        }

        u64 now_ms = get_time_ms();

        // --- Lógica del Cronómetro ---
        if (sw_running) {
            sw_accumulated_ms += (now_ms - sw_start_ms);
            sw_start_ms = now_ms;
        }

        // --- Lógica del Temporizador ---
        if (timer_running) {
            u64 delta = now_ms - timer_last_update_ms;
            timer_last_update_ms = now_ms;

            if (timer_remaining_ms > delta) {
                timer_remaining_ms -= delta;
            } else {
                timer_remaining_ms = 0;
                timer_running = false;
                timer_finished = true;
            }
        }

        // --- Manejo de Inputs según la pestaña activa ---
        if (current_tab == TAB_STOPWATCH) {
            // A: Iniciar / Pausar
            if (kDown & HidNpadButton_A) {
                sw_running = !sw_running;
                if (sw_running) {
                    sw_start_ms = now_ms;
                }
            }
            // X: Reiniciar
            if (kDown & HidNpadButton_X) {
                sw_running = false;
                sw_accumulated_ms = 0;
            }
        } else if (current_tab == TAB_TIMER) {
            // A: Iniciar / Pausar
            if (kDown & HidNpadButton_A) {
                if (timer_remaining_ms > 0) {
                    timer_running = !timer_running;
                    if (timer_running) {
                        timer_last_update_ms = now_ms;
                        timer_finished = false;
                    }
                }
            }
            // X: Reiniciar al tiempo configurado
            if (kDown & HidNpadButton_X) {
                timer_running = false;
                timer_remaining_ms = timer_duration_ms;
                timer_finished = false;
            }
            // Ajustar tiempo cuando está pausado
            if (!timer_running) {
                // D-Pad Arriba: +1 Minuto
                if (kDown & HidNpadButton_Up) {
                    timer_duration_ms += 60 * 1000;
                    timer_remaining_ms = timer_duration_ms;
                    timer_finished = false;
                }
                // D-Pad Abajo: -1 Minuto (mínimo 10 segundos)
                if (kDown & HidNpadButton_Down) {
                    if (timer_duration_ms > 60 * 1000) {
                        timer_duration_ms -= 60 * 1000;
                    } else {
                        timer_duration_ms = 10 * 1000;
                    }
                    timer_remaining_ms = timer_duration_ms;
                    timer_finished = false;
                }
                // Y: +10 Segundos
                if (kDown & HidNpadButton_Y) {
                    timer_duration_ms += 10 * 1000;
                    timer_remaining_ms = timer_duration_ms;
                    timer_finished = false;
                }
            }
        }

        // --- RENDERIZADO DE LA INTERFAZ ---

        // Posicionar cursor al inicio sin borrar toda la pantalla para evitar parpadeos
        printf("\x1b[1;1H");

        // Encabezado
        printf("\x1b[36m================================================================================\x1b[0m\n");
        printf("\x1b[1m                 NINTENDO SWITCH HOMEBREW - CLOCK SUITE                         \x1b[0m\n");
        printf("\x1b[36m================================================================================\x1b[0m\n\n");

        // Barra de Pestañas
        printf("   ");
        if (current_tab == TAB_CLOCK) {
            printf("\x1b[47m\x1b[30m [1] RELOJ (RTC) \x1b[0m   ");
        } else {
            printf(" [1] RELOJ (RTC)    ");
        }

        if (current_tab == TAB_STOPWATCH) {
            printf("\x1b[47m\x1b[30m [2] CRONOMETRO \x1b[0m   ");
        } else {
            printf(" [2] CRONOMETRO    ");
        }

        if (current_tab == TAB_TIMER) {
            printf("\x1b[47m\x1b[30m [3] TEMPORIZADOR \x1b[0m\n\n");
        } else {
            printf(" [3] TEMPORIZADOR\n\n");
        }

        printf("--------------------------------------------------------------------------------\n\n");

        // --- Contenido según la pestaña activa ---
        if (current_tab == TAB_CLOCK) {
            time_t unix_time = time(NULL);
            struct tm *time_info = localtime(&unix_time);

            char time_str[64];
            char date_str[64];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
            strftime(date_str, sizeof(date_str), "%A, %d de %B de %Y", time_info);

            printf("  \x1b[33mCONTENIDO: RELOJ EN TIEMPO REAL\x1b[0m\n\n");
            printf("  Hora Actual:  \x1b[1m\x1b[32m%s\x1b[0m\n", time_str);
            printf("  Fecha:        %s\n\n", date_str);
            printf("  * El reloj utiliza el tiempo real (RTC) de la consola Nintendo Switch.\n\n");

        } else if (current_tab == TAB_STOPWATCH) {
            u64 total_sec = sw_accumulated_ms / 1000;
            u64 min = total_sec / 60;
            u64 sec = total_sec % 60;
            u64 ms = sw_accumulated_ms % 1000;

            printf("  \x1b[33mCONTENIDO: CRONOMETRO\x1b[0m\n\n");
            printf("  Tiempo Transcurrido: \x1b[1m\x1b[32m%02lu:%02lu.%03lu\x1b[0m\n\n", min, sec, ms);
            printf("  Estado: %s\n\n", sw_running ? "\x1b[32m[CORRIENDO]\x1b[0m" : "\x1b[33m[PAUSADO]\x1b[0m");
            printf("  Controles Cronómetro:\n");
            printf("    - \x1b[1mA\x1b[0m: Iniciar / Pausar\n");
            printf("    - \x1b[1mX\x1b[0m: Reiniciar a 00:00.000\n\n");

        } else if (current_tab == TAB_TIMER) {
            u64 total_sec = timer_remaining_ms / 1000;
            u64 min = total_sec / 60;
            u64 sec = total_sec % 60;

            printf("  \x1b[33mCONTENIDO: TEMPORIZADOR DE CUENTA REGRESIVA\x1b[0m\n\n");
            if (timer_finished) {
                printf("  Tiempo Restante: \x1b[1m\x1b[31m00:00 (¡TIEMPO AGOTADO!)\x1b[0m\n\n");
            } else {
                printf("  Tiempo Restante: \x1b[1m\x1b[32m%02lu:%02lu\x1b[0m\n\n", min, sec);
            }

            printf("  Estado: %s\n\n", timer_running ? "\x1b[32m[EN CURSO]\x1b[0m" : (timer_finished ? "\x1b[31m[FINALIZADO]\x1b[0m" : "\x1b[33m[PAUSADO / CONFIGURACION]\x1b[0m"));
            printf("  Controles Temporizador:\n");
            printf("    - \x1b[1mA\x1b[0m: Iniciar / Pausar\n");
            printf("    - \x1b[1mX\x1b[0m: Reiniciar tiempo\n");
            printf("    - \x1b[1mD-Pad Arriba / Abajo\x1b[0m: +/- 1 Minuto (en pausa)\n");
            printf("    - \x1b[1mY\x1b[0m: +10 Segundos (en pausa)\n\n");
        }

        // Pie de página de controles globales
        printf("\x1b[36m--------------------------------------------------------------------------------\x1b[0m\n");
        printf(" Controles Globales: \x1b[1mL / R\x1b[0m o \x1b[1mD-Pad Izq/Der\x1b[0m: Cambiar Pestaña | \x1b[1m(+)\x1b[0m: Salir\n");
        printf("\x1b[36m================================================================================\x1b[0m\n");

        consoleUpdate(NULL);
    }

    // Salir limpiando la consola
    clear_screen();
    consoleExit(NULL);
    return 0;
}
