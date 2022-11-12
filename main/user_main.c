/* gpio example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "main";

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO2: output
 * GPIO0:  input, pulled up, interrupt from falling edge
 */

#define GPIO_OUTPUT_IO_0    2
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)
#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)

#define configUSE_IDLE_HOOK 1


static SemaphoreHandle_t mutex = NULL;

void vApplicationIdleHook()
{
    esp_err_t val;
    val = esp_sleep_enable_timer_wakeup(25000);
    if (val != ESP_OK)
    {
        printf("Cannot set sleep duration");
    }
    else
    {
              
        val = esp_light_sleep_start();
        if(val != ESP_OK)
        {
            printf("Did not sleep");
        }
        else
        {
            printf("Sleeping");
        }
    }
}

static void active_delay_500ms()
{
    uint32_t delay_time = 500;
    uint32_t start  = xTaskGetTickCount();
    uint32_t status = xTaskGetTickCount();
    uint32_t end    = (delay_time / portTICK_RATE_MS) + start;
    while (status < end)
    {
        status = xTaskGetTickCount();
    }
}

static void gpio_task_example1(void *arg)
{
    bool pin_out = 1;
    while(1)
    {
        if (xSemaphoreTake(mutex,(TickType_t)1) == pdTRUE) 
        {
            gpio_set_level(GPIO_OUTPUT_IO_0, pin_out); 
            //active delay 0.5s
            active_delay_500ms();
            xSemaphoreGive(mutex);
            //task delay 1s
            vTaskDelay(1000/portTICK_RATE_MS);
        }
    
        else
        {
            printf("Task1 Failed to take Mutex \n");
        }
    }
}

static void gpio_task_example2(void *arg)
{
    bool pin_out = 0;
    while(1)
    {
        if (xSemaphoreTake(mutex,(TickType_t)1) == pdTRUE) 
        {
            gpio_set_level(GPIO_OUTPUT_IO_0, pin_out); 
            //active delay 0.5s
            active_delay_500ms();
            xSemaphoreGive(mutex);
            //task delay 1s
            vTaskDelay(1000/portTICK_RATE_MS);
        }
        else
        {
        printf("Task2 Failed to take Mutex \n");
        }
    }    
}

static void gpio_task_example3(void *arg)
{
    uint32_t pin_out;
    while(1){
        if (xSemaphoreTake(mutex,(TickType_t)1) == pdTRUE) 
        {
            pin_out = gpio_get_level(GPIO_OUTPUT_IO_0);
            printf("Output level of pin is : %d \n", pin_out);
            xSemaphoreGive(mutex);
            //task delay 1s
            vTaskDelay(1000/portTICK_RATE_MS);
        }
        else
        {
            printf("Task3 Failed to take Mutex \n");
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO2
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    mutex = xSemaphoreCreateMutex();
    
    xTaskCreate(gpio_task_example1, "gpio_task_example1", 2048, NULL, 8, NULL);
    xTaskCreate(gpio_task_example2, "gpio_task_example2", 2048, NULL, 9, NULL);
    xTaskCreate(gpio_task_example3, "gpio_task_example3", 2048, NULL, 10, NULL);
}


