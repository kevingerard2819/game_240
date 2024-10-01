
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "board_io.h"
#include "common_macros.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "gpio_intr.h"


static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static SemaphoreHandle_t switch_pressed_signal;

static uint8_t SW2 = 30;
static uint8_t led2 = 24;

// Used in 1 and 2
void sleep_on_sem_task(void *p) {
  fprintf("status : %d", xSemaphoreTake(switch_pressed_signal, portMAX_DELAY));
  while (1) {
    if (xSemaphoreTake(switch_pressed_signal, 1000)) {
      vTaskDelay(100);
      LPC_GPIO1->SET = (1 << led2);
      vTaskDelay(100);
      LPC_GPIO1->CLR = (1 << led2);
    }
  }
}
//PART 2 
// def of pin29_isr and pin30_isr
void pin29_isr(void) {
  fprintf(stderr, "ISR Entry from pin 29\n");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
}

void pin30_isr(void) {
  fprintf(stderr, "ISR Entry from pin 30\n");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
}

// Main function here 

int main(void) {
  create_blinky_tasks();
  // create_blinky_tasks();
  create_uart_task();
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  puts("Starting RTOS");
  // Part 0
  // LPC_GPIO0->DIR &= ~(1 << SW2);
  // LPC_GPIOINT->IO0IntEnF |= (1 << SW2);
  // lpc_peripheral__enable_interrupt(54, gpio_interrupt, "gpio_SW2");
  // NVIC_EnableIRQ(GPIO_IRQn);
  // xTaskCreate(led_blinking, "led2", 4096 / sizeof(void *), NULL, 1, NULL);

  // Used in both 
  switch_pressed_signal = xSemaphoreCreateBinary();
  xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // Part 1
  /*
  configure_your_gpio_interrupt();
  NVIC_EnableIRQ(GPIO_IRQn);
  */
  // Part 2

  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, pin29_isr);
  gpio0__attach_interrupt(30, GPIO_INTR__RISING_EDGE, pin30_isr);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "Interrupt");

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

// Part 0
// static void led_blinking(void) {
//   while (1) {
//     vTaskDelay(100);
//     LPC_GPIO1->SET = (1 << led2);
//     vTaskDelay(100);
//     LPC_GPIO1->CLR = (1 << led2);
//   }
// }
// void gpio_interrupt(void) { LPC_GPIOINT->IO0IntClr |= (1 << SW2); }

// Part 1
/*
void clear_gpio_interrupt(void) { LPC_GPIOINT->IO0IntClr |= (1 << SW2); }

void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry\n");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  clear_gpio_interrupt();
}
void configure_your_gpio_interrupt() {
  LPC_GPIO0->DIR &= ~(1 << SW2);        // Setting SW2 to input
  LPC_GPIOINT->IO0IntEnF |= (1 << SW2); // SW2 trigger at falling edge
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "gpio_SW2");
}

*/
// Part 2

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