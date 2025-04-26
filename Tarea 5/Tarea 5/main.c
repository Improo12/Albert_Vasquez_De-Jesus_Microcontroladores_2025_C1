#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN     GPIO_NUM_2
#define BUTTON_PIN  GPIO_NUM_0

static const char* TAG = "BOTON_APP";

static uint32_t press_time = 0;
static uint32_t duration_ms = 0;

static TimerHandle_t timer_button = NULL;
static TaskHandle_t task_handle = NULL;

static enum {
    MODO_PARPADEO = 0,
    MODO_ESTADO,
    MODO_VARIABLE,
    MODO_MULTIFUNCION
} modo = MODO_PARPADEO;

static void button_timer_callback(TimerHandle_t xTimer) {
    press_time++;
}

void IRAM_ATTR button_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio_get_level(BUTTON_PIN) == 0) {
        press_time = 0;
        gpio_set_level(LED_PIN, 1);
        xTimerStartFromISR(timer_button, &xHigherPriorityTaskWoken);
    } else {
        xTimerStopFromISR(timer_button, &xHigherPriorityTaskWoken);
        duration_ms = press_time;
        gpio_set_level(LED_PIN, 0);
        vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Variable de configuración ajustable
static uint32_t valorConfiguracion = 0;

// Máquina de estados
static enum { ESTADO_IDLE, ESTADO_A, ESTADO_B } estado_actual = ESTADO_IDLE;

void led_task(void* arg) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        switch (modo) {
        case MODO_PARPADEO:
            for (uint32_t i = 0; i < duration_ms / 500; i++) {
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(250));
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(250));
            }
            break;

        case MODO_VARIABLE:
            valorConfiguracion = duration_ms / 100;
            ESP_LOGI(TAG, "Variable ajustada: %d", valorConfiguracion);
            break;

        case MODO_ESTADO:
            if (duration_ms < 1000) estado_actual = ESTADO_A;
            else estado_actual = ESTADO_B;

            if (estado_actual == ESTADO_A) {
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(300));
                gpio_set_level(LED_PIN, 0);
            } else {
                for (int i = 0; i < 3; i++) {
                    gpio_set_level(LED_PIN, 1);
                    vTaskDelay(pdMS_TO_TICKS(150));
                    gpio_set_level(LED_PIN, 0);
                    vTaskDelay(pdMS_TO_TICKS(150));
                }
            }
            break;

        case MODO_MULTIFUNCION:
            if (duration_ms < 1000) {
                ESP_LOGI(TAG, "Acción corta ejecutada");
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_PIN, 0);
            } else if (duration_ms < 3000) {
                ESP_LOGI(TAG, "Acción media ejecutada");
                for (int i = 0; i < 2; i++) {
                    gpio_set_level(LED_PIN, 1);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    gpio_set_level(LED_PIN, 0);
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
            } else {
                ESP_LOGI(TAG, "Acción larga ejecutada");
                for (int i = 0; i < 5; i++) {
                    gpio_set_level(LED_PIN, 1);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    gpio_set_level(LED_PIN, 0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
            break;
        }
    }
}

void setup_gpio_and_timers() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = 1,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_PIN, 0);

    timer_button = xTimerCreate("btn_timer", pdMS_TO_TICKS(1), pdTRUE, NULL, button_timer_callback);
    xTaskCreate(led_task, "LED Task", 2048, NULL, 10, &task_handle);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}

void app_main() {
    setup_gpio_and_timers();

    // Cambia el modo aquí para probar distintas funcionalidades:
    modo = MODO_PARPADEO;
    // modo = MODO_VARIABLE;
    // modo = MODO_ESTADO;
    // modo = MODO_MULTIFUNCION;

    ESP_LOGI(TAG, "Sistema listo. Modo actual: %d", modo);
}
