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
    padConfigureInput(1, HID_NPAD_STYLE_SET_FULLKEY);
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
        if (kDown & KEY_PLUS) {
            break;
        }

        // --- Navegación entre Pestañas (L / R o D-Pad Izq / Der) ---
        if (kDown & (KEY_R | KEY_DRIGHT)) {
            current_tab = (current_tab + 1) % TAB_COUNT;
            clear_screen();
        }
        if (kDown & (KEY_L | KEY_DLEFT)) {
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
            if (kDown & KEY_A) {
                sw_running = !sw_running;
                if (sw_running) {
                    sw_start_ms = now_ms;
                }
            }
            // X: Reiniciar
            if (kDown & KEY_X) {
                sw_running = false;
                sw_accumulated_ms = 0;
            }
        } else if (current_tab == TAB_TIMER) {
            // A: Iniciar / Pausar
            if (kDown & KEY_A) {
                if (timer_remaining_ms > 0) {
                    timer_running = !timer_running;
                    if (timer_running) {
                        timer_last_update_ms = now_ms;
                        timer_finished = false;
                    }
                }
            }
            // X: Reiniciar al tiempo configurado
            if (kDown & KEY_X) {
                timer_running = false;
                timer_remaining_ms = timer_duration_ms;
                timer_finished = false;
            }
            // Ajustar tiempo cuando está pausado
            if (!timer_running) {
                // D-Pad Arriba: +1 Minuto
                if (kDown & KEY_DUP) {
                    timer_duration_ms += 60 * 1000;
                    timer_remaining_ms = timer_duration_ms;
                    timer_finished = false;
                }
                // D-Pad Abajo: -1 Minuto (mínimo 10 segundos)
                if (kDown & KEY_DDOWN) {
                    if (timer_duration_ms > 60 * 1000) {
                        timer_duration_ms -= 60 * 1000;
                    } else {
                        timer_duration_ms = 10 * 1000;
                    }
                    timer_remaining_ms = timer_duration_ms;
                    timer_finished = false;
                }
                // Y: +10 Segundos
                if (kDown & KEY_Y) {
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
            printf("\x1b[47m\x1b[30m  [1] RELOJ  \x1b[0m  ");
        } else {
            printf("  [1] RELOJ    ");
        }

        if (current_tab == TAB_STOPWATCH) {
            printf("\x1b[47m\x1b[30m  [2] CRONOMETRO  \x1b[0m  ");
        } else {
            printf("  [2] CRONOMETRO    ");
        }

        if (current_tab == TAB_TIMER) {
            printf("\x1b[47m\x1b[30m  [3] TEMPORIZADOR  \x1b[0m");
        } else {
            printf("  [3] TEMPORIZADOR  ");
        }
        printf("\n\n");
        printf("--------------------------------------------------------------------------------\n\n");

        // --- CONTENIDO SEGÚN PESTAÑA ---

        if (current_tab == TAB_CLOCK) {
            time_t rawtime = time(NULL);
            struct tm *timeinfo = localtime(&rawtime);

            char date_str[64];
            char time_str[64];
            strftime(date_str, sizeof(date_str), "%A, %d de %B de %Y", timeinfo);
            strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

            printf("  \x1b[33m+------------------------------------------------------------------+\x1b[0m\n");
            printf("  \x1b[33m|                                                                  |\x1b[0m\n");
            printf("  \x1b[33m|\x1b[0m              HORA ACTUAL DEL SISTEMA (RTC)                      \x1b[33m|\x1b[0m\n");
            printf("  \x1b[33m|                                                                  |\x1b[0m\n");
            printf("  \x1b[33m|\x1b[0m                 \x1b[1m\x1b[32m  %s  \x1b[0m                          \x1b[33m|\x1b[0m\n", time_str);
            printf("  \x1b[33m|                                                                  |\x1b[0m\n");
            printf("  \x1b[33m|\x1b[0m               %s                               \x1b[33m|\x1b[0m\n", date_str);
            printf("  \x1b[33m|                                                                  |\x1b[0m\n");
            printf("  \x1b[33m+------------------------------------------------------------------+\x1b[0m\n\n");
            printf("  Muestra la fecha y hora sintonizada con el chip RTC de tu consola.\n\n");

        } else if (current_tab == TAB_STOPWATCH) {
            u32 total_sec = (u32)(sw_accumulated_ms / 1000);
            u32 min = total_sec / 60;
            u32 sec = total_sec % 60;
            u32 ms = (u32)(sw_accumulated_ms % 1000) / 10; // Céntimas de segundo

            printf("  \x1b[35m+------------------------------------------------------------------+\x1b[0m\n");
            printf("  \x1b[35m|                                                                  |\x1b[0m\n");
            printf("  \x1b[35m|\x1b[0m                       CRONOMETRO                                 \x1b[35m|\x1b[0m\n");
            printf("  \x1b[35m|                                                                  |\x1b[0m\n");
            printf("  \x1b[35m|\x1b[0m                 \x1b[1m\x1b[33m    %02u:%02u.%02u    \x1b[0m                             \x1b[35m|\x1b[0m\n", min, sec, ms);
            printf("  \x1b[35m|                                                                  |\x1b[0m\n");
            printf("  \x1b[35m|\x1b[0m              ESTADO: %-42s \x1b[35m|\x1b[0m\n", sw_running ? "\x1b[32m[ CORRIENDO ]\x1b[0m" : "\x1b[31m[ PAUSADO ]\x1b[0m");
            printf("  \x1b[35m|                                                                  |\x1b[0m\n");
            printf("  \x1b[35m+------------------------------------------------------------------+\x1b[0m\n\n");

        } else if (current_tab == TAB_TIMER) {
            u32 total_sec = (u32)(timer_remaining_ms / 1000);
            u32 min = total_sec / 60;
            u32 sec = total_sec % 60;

            printf("  \x1b[34m+------------------------------------------------------------------+\x1b[0m\n");
            printf("  \x1b[34m|                                                                  |\x1b[0m\n");
            printf("  \x1b[34m|\x1b[0m                       TEMPORIZADOR                                \x1b[34m|\x1b[0m\n");
            printf("  \x1b[34m|                                                                  |\x1b[0m\n");
            printf("  \x1b[34m|\x1b[0m                 \x1b[1m\x1b[36m      %02u:%02u      \x1b[0m                             \x1b[34m|\x1b[0m\n", min, sec);
            printf("  \x1b[34m|                                                                  |\x1b[0m\n");
            if (timer_finished) {
                printf("  \x1b[34m|\x1b[0m           \x1b[5m\x1b[41m\x1b[37m  ¡ TIEMPO FINALIZADO !  \x1b[0m                         \x1b[34m|\x1b[0m\n");
            } else {
                printf("  \x1b[34m|\x1b[0m              ESTADO: %-42s \x1b[34m|\x1b[0m\n", timer_running ? "\x1b[32m[ CONTANDO ]\x1b[0m" : "\x1b[31m[ PAUSADO ]\x1b[0m");
            }
            printf("  \x1b[34m|                                                                  |\x1b[0m\n");
            printf("  \x1b[34m+------------------------------------------------------------------+\x1b[0m\n\n");
        }

        // --- BARRA INFERIOR DE CONTROLES ---
        printf("--------------------------------------------------------------------------------\n");
        printf(" CONTROLES:\n");
        printf("  [L / R / D-Pad] : Cambiar Pestana\n");

        if (current_tab == TAB_STOPWATCH) {
            printf("  [A]             : %s Cronometro\n", sw_running ? "Pausar" : "Iniciar");
            printf("  [X]             : Reiniciar Cronometro\n");
        } else if (current_tab == TAB_TIMER) {
            printf("  [A]             : %s Temporizador\n", timer_running ? "Pausar" : "Iniciar");
            printf("  [X]             : Reiniciar Temporizador\n");
            if (!timer_running) {
                printf("  [D-Pad Arriba/Abajo] : +/- 1 Minuto | [Y] : +10 Segundos\n");
            }
        }
        printf("  [(+) PLUS]      : Salir a hbmenu\n");
        printf("--------------------------------------------------------------------------------\n");

        // Sincronizar actualización a refresco de pantalla
        consoleUpdate(NULL);
    }

    // Salida limpia liberando pantalla y recursos de libnx
    consoleExit(NULL);
    return 0;
}
