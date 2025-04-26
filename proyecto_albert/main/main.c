#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

static const char html[] = R"rawliteral(<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>NE555</title><style>body { background:#1a237e; font-family:sans-serif; color:white; margin:0; }.container { padding:2rem; }input, select { padding:0.5rem; border-radius:5px; border:none; width:100%; margin-top:0.5rem; }button { margin-top:1rem; padding:0.7rem; border:none; border-radius:5px; background:white; color:#1a237e; cursor:pointer; }</style></head><body><div class='container'><h1>Proyecto Albert Vasquez</h1><label>R1:</label><input type='number' id='r1'><label>C:</label><input type='number' id='c1'><button onclick='calc()'>Calcular</button><p id='out'></p></div><script>function calc() {  var r = parseFloat(document.getElementById('r1').value);  var c = parseFloat(document.getElementById('c1').value);  if (isNaN(r) || isNaN(c)) return alert("Valores inválidos");  document.getElementById('out').innerText = "T = " + (1.1*r*c).toFixed(6) + " s";}</script></body></html>)rawliteral";

esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_handler,
    .user_ctx = NULL
};

void start_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_root);
    }
}

void app_main(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "NE555",
            .ssid_len = strlen("NE555"),
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    start_server();
}