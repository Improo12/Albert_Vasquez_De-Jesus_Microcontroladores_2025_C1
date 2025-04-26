#include <stdio.h>

// Activar/desactivar modo debug
#define DEBUG 1

// Enumeración de estados
typedef enum {
    ABIERTO,
    CERRANDO,
    CERRADO,
    ABRIENDO,
    DETENIDO,
    DESCARRILADO
} Estado;

// Variables globales
Estado estado_actual = ABIERTO;
Estado estado_anterior = ABIERTO;

// Función para imprimir nombre del estado
const char* nombre_estado(Estado e) {
    switch (e) {
        case ABIERTO: return "ABIERTO";
        case CERRANDO: return "CERRANDO";
        case CERRADO: return "CERRADO";
        case ABRIENDO: return "ABRIENDO";
        case DETENIDO: return "DETENIDO";
        case DESCARRILADO: return "DESCARRILADO";
        default: return "DESCONOCIDO";
    }
}

// Función de transición
void transicion_estados(int PP, int LSA, int LSC, int FTC, int RESET) {
    estado_anterior = estado_actual;

    if (RESET) {
        estado_actual = ABIERTO;
        if (DEBUG) printf("[RESET] Estado -> ABIERTO\n");
        return;
    }

    switch (estado_actual) {
        case ABIERTO:
            if (PP) estado_actual = CERRANDO;
            break;

        case CERRANDO:
            if (FTC) {
                estado_actual = DESCARRILADO;
            } else if (LSC) {
                estado_actual = CERRADO;
            } else if (PP) {
                estado_actual = DETENIDO;
            }
            break;

        case CERRADO:
            if (PP) estado_actual = ABRIENDO;
            break;

        case ABRIENDO:
            if (FTC) {
                estado_actual = DESCARRILADO;
            } else if (LSA) {
                estado_actual = ABIERTO;
            } else if (PP) {
                estado_actual = DETENIDO;
            }
            break;

        case DETENIDO:
            if (PP) estado_actual = estado_anterior;
            break;

        case DESCARRILADO:
            // Esperar RESET
            break;
    }

    if (DEBUG) {
        printf("Estado Actual: %s\n", nombre_estado(estado_actual));
    }
}

// Simulación simple
int main() {
    // Simulación de eventos: PP, LSA, LSC, FTC, RESET
    transicion_estados(1, 0, 0, 0, 0); // ABIERTO -> CERRANDO
    transicion_estados(0, 0, 1, 0, 0); // CERRANDO -> CERRADO
    transicion_estados(1, 0, 0, 0, 0); // CERRADO -> ABRIENDO
    transicion_estados(0, 1, 0, 0, 0); // ABRIENDO -> ABIERTO
    transicion_estados(1, 0, 0, 0, 0); // ABIERTO -> CERRANDO
    transicion_estados(0, 0, 0, 1, 0); // CERRANDO -> DESCARRILADO
    transicion_estados(0, 0, 0, 0, 1); // RESET -> ABIERTO

    return 0;
}
