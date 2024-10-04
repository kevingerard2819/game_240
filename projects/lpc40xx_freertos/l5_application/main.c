
#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab_4.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "task.h"
#include <stdio.h>

static SemaphoreHandle_t switch_press_indication;

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
void led_task(void *params);
void switch_task(void *params);
void led_task_sem(void *params);
void led_task_bonus(void *task_parameter);

typedef struct {
  /* First get gpio0 driver to work only, and if you finish it
   * you can do the extra credit to also make it work for other Ports
   */
  // uint8_t port;

  uint8_t pin;
} port_pin_s;

int main(void) {
  create_blinky_tasks();
  // create_blinky_tasks();
  create_uart_task();

  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  puts("Starting RTOS");

  // Part 0
  // xTaskCreate(led_task, "led1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // Part 2
  /*
  static port_pin_s led1 = {26};
  static port_pin_s led2 = {24};
  xTaskCreate(led_task, "led1", 2048 / sizeof(void *), &led1, 1, NULL);
  xTaskCreate(led_task, "led2", 2048 / sizeof(void *), &led2, 1, NULL);
  */
  // part 3
  /*
  switch_press_indication = xSemaphoreCreateBinary();
  static port_pin_s switch1 = {15};
  static port_pin_s led2 = {24};
  xTaskCreate(switch_task, "sw1", 2048 / sizeof(void *), &switch1, 2, NULL);
  xTaskCreate(led_task_sem, "led1", 2048 / sizeof(void *), &led2, 1, NULL);


 */
  switch_press_indication = xSemaphoreCreateBinary();
  static port_pin_s switch1 = {15};
  static port_pin_s led2 = {24};
  xTaskCreate(switch_task, "sw1", 2048 / sizeof(void *), &switch1, 2, NULL);
  xTaskCreate(led_task_bonus, "led1", 2048 / sizeof(void *), &led2, 1, NULL);

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

// Part 0 : Function for basic led blinking task (LED 1)
/*
void led_task(void *params) {
 LPC_GPIO1->DIR |= (1 << 26);
 while (1) {
   LPC_GPIO1->PIN &= ~(1 << 26);
   vTaskDelay(500);
   LPC_GPIO1->PIN |= (1 << 26);
   vTaskDelay(500);
 }
}
*/
// Part 2 : Using parameters passed to function
/*
void led_task(void *task_parameter) {
 const port_pin_s *led = (port_pin_s *)(task_parameter);


 while (true) {
   gpio1__set_high(led->pin);
   vTaskDelay(100);


   gpio1__set_low(led->pin);
   vTaskDelay(100);
 }
}
*/
/*
// Part 3 : Semaphores
void led_task_sem(void *task_parameter) {
 const port_pin_s *led = (port_pin_s *)(task_parameter);
 while (true) {
   if (xSemaphoreTake(switch_press_indication, 1000)) {
     gpio1__set_high(led->pin);
     vTaskDelay(100);
     gpio1__set_low(led->pin);
     vTaskDelay(100);
   } else {
     puts("Timeout: No switch press indication for 1000ms");
   }
 }
}
*/
void switch_task(void *task_parameter) {
  port_pin_s *switch1 = (port_pin_s *)task_parameter;
  gpio1__set_as_input(switch1->pin);
  while (true) {
    if (gpio1__get_level(switch1->pin)) {
      xSemaphoreGive(switch_press_indication);
    }
    vTaskDelay(100);
  }
}

// Bonus Task :  Blinking 3 leds sequencially when the switch is pressed ( 4th isn't considered as the port was changed
void led_task_bonus(void *task_parameter) {
  int leds[] = {26, 24, 18}; // Three LEDs at pins 26, 24, and 18

  while (true) {
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // LED 1 blinks once
      gpio1__set_high(leds[0]);
      vTaskDelay(200);
      gpio1__set_low(leds[0]);
      vTaskDelay(200);

      // LED 2 blinks twice
      for (int i = 0; i < 2; i++) {
        gpio1__set_high(leds[1]);
        vTaskDelay(200);
        gpio1__set_low(leds[1]);
        vTaskDelay(200);
      }

      // LED 3 blinks three times
      for (int i = 0; i < 3; i++) {
        gpio1__set_high(leds[2]);
        vTaskDelay(200);
        gpio1__set_low(leds[2]);
        vTaskDelay(200);
      }

      // Small delay before repeating the pattern
      vTaskDelay(500);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}
static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  // If you wish to avoid malloc, use xTaskCreateStatic() in place of xTaskCreate()
  static StackType_t led0_task_stack[512 / sizeof(StackType_t)];
  static StackType_t led1_task_stack[512 / sizeof(StackType_t)];
  static StaticTask_t led0_task_struct;
  static StaticTask_t led1_task_struct;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreateStatic(blink_task, "led0", ARRAY_SIZE(led0_task_stack), (void *)&led0, PRIORITY_LOW, led0_task_stack,
                    &led0_task_struct);
  xTaskCreateStatic(blink_task, "led1", ARRAY_SIZE(led1_task_stack), (void *)&led1, PRIORITY_LOW, led1_task_stack,
                    &led1_task_struct);
#else
  periodic_scheduler__initialize();
  UNUSED(blink_task);
#endif
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
