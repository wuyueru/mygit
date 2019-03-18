deps_config := \
	/home/rita/esp32/esp-idf/components/app_trace/Kconfig \
	/home/rita/esp32/esp-idf/components/aws_iot/Kconfig \
	/home/rita/esp32/esp-idf/components/bt/Kconfig \
	/home/rita/esp32/esp-idf/components/driver/Kconfig \
	/home/rita/esp32/esp-idf/components/efuse/Kconfig \
	/home/rita/esp32/esp-idf/components/esp32/Kconfig \
	/home/rita/esp32/esp-idf/components/esp_adc_cal/Kconfig \
	/home/rita/esp32/esp-idf/components/esp_event/Kconfig \
	/home/rita/esp32/esp-idf/components/esp_http_client/Kconfig \
	/home/rita/esp32/esp-idf/components/esp_http_server/Kconfig \
	/home/rita/esp32/esp-idf/components/esp_https_ota/Kconfig \
	/home/rita/esp32/esp-idf/components/espcoredump/Kconfig \
	/home/rita/esp32/esp-idf/components/ethernet/Kconfig \
	/home/rita/esp32/esp-idf/components/fatfs/Kconfig \
	/home/rita/esp32/esp-idf/components/freemodbus/Kconfig \
	/home/rita/esp32/esp-idf/components/freertos/Kconfig \
	/home/rita/esp32/esp-idf/components/heap/Kconfig \
	/home/rita/esp32/esp-idf/components/libsodium/Kconfig \
	/home/rita/esp32/esp-idf/components/log/Kconfig \
	/home/rita/esp32/esp-idf/components/lwip/Kconfig \
	/home/rita/esp32/esp-idf/components/mbedtls/Kconfig \
	/home/rita/esp32/esp-idf/components/mdns/Kconfig \
	/home/rita/esp32/esp-idf/components/mqtt/Kconfig \
	/home/rita/esp32/esp-idf/components/nvs_flash/Kconfig \
	/home/rita/esp32/esp-idf/components/openssl/Kconfig \
	/home/rita/esp32/esp-idf/components/pthread/Kconfig \
	/home/rita/esp32/esp-idf/components/spi_flash/Kconfig \
	/home/rita/esp32/esp-idf/components/spiffs/Kconfig \
	/home/rita/esp32/esp-idf/components/tcpip_adapter/Kconfig \
	/home/rita/esp32/esp-idf/components/unity/Kconfig \
	/home/rita/esp32/esp-idf/components/vfs/Kconfig \
	/home/rita/esp32/esp-idf/components/wear_levelling/Kconfig \
	/home/rita/esp32/esp-idf/components/app_update/Kconfig.projbuild \
	/home/rita/esp32/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/rita/esp32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/rita/mygit/my_esp32/main/Kconfig.projbuild \
	/home/rita/esp32/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/rita/esp32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_TARGET)" "esp32"
include/config/auto.conf: FORCE
endif
ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
