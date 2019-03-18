#include "led.h"
#include "driver/gpio.h"
#include "esp_err.h"

void led_on(void)
{
	gpio_set_level(BLINK_GPIO, 0);
}

void led_off(void)
{
	gpio_set_level(BLINK_GPIO, 1);
}

void init_led(void)
{
	gpio_pad_select_gpio(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO,GPIO_MODE_OUTPUT);
	led_off();//初始化时先LED的状态是关闭的
}


