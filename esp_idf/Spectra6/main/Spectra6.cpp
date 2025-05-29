#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_system.h"

#include <bb_epaper.h>
#include "Roboto_Black_40.h"
BBEPAPER bbep(EP81_SPECTRA_1024x576);

#define DC_PIN 14
#define BUSY_PIN 13
#define RESET_PIN 9
#define CS_PIN 10
#define CS_PIN2 8
#define SCK_PIN 12
#define MOSI_PIN 11

static const char *TAG = "MAIN";

void main_application_task(void *pvParameters) {
    // register the task with the watchdog
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    esp_task_wdt_reset();

    bbep.initIO(DC_PIN, RESET_PIN, BUSY_PIN, CS_PIN, MOSI_PIN, SCK_PIN, 8000000);
    bbep.setCS2(CS_PIN2);
    bbep.setRotation(0);
    bbep.allocBuffer();

    while (1) {
        bbep.fillScreen(BBEP_WHITE);
        bbep.setFont(&Roboto_Black_40);
        bbep.setTextColor(BBEP_BLACK);
        bbep.setCursor(0, 75);
        bbep.print("Mañana");
        bbep.setTextColor(BBEP_RED);
        bbep.setCursor(100, 150);
        bbep.print("Ótimo!");
        bbep.setTextColor(BBEP_BLUE);
        bbep.setCursor(200, 225);
        bbep.print("Non è vero?");
        bbep.setTextColor(BBEP_YELLOW);
        bbep.setCursor(300, 300);
        bbep.print("Não foi eu!");
        bbep.writePlane();
        bbep.refresh(REFRESH_FULL, false); 

        // check busy
        while (1) {
            esp_task_wdt_reset();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "wait for panel update to finish...");

            if (!bbep.isBusy()) {
                break;
            }
        }

        bbep.sleep(1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Update again");

        // do it with a restart
        // DISABLE TO REFRESH WITHOUT RESTART
        esp_restart();
    }
}

extern "C" void app_main(void)
{
    xTaskCreatePinnedToCore(
        main_application_task,     // Task function
        "main_app_task",           // Name for debugging
        4096,                      // Stack size (bytes)
        NULL,                      // Parameters
        5,                         // Priority (higher number = higher priority)
        NULL,                      // Task handle (if you need to reference this task later)
        APP_CPU_NUM                // Core ID (usually 1, as core 0 often handles WiFi/BT)
    );
}
