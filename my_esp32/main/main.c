#include "led.h"
#include "wifi.h"
#include "tcp.h"
#include "sdkconfig.h"

extern char *TAG;

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    init_led();
    xTaskCreate(tcp_server_task, "tcp_server", 4096*2, NULL, 5, NULL);

}
