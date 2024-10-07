#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include <stdio.h>

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static QueueHandle_t adc_to_pwm_task_queue;

// part 0
/*
void pin_configure_pwm_channel_as_io_pin(void)
{
gpio__construct_with_function(2, 0, GPIO__FUNCTION_1); // P2_0
}
// Part 0
static void pwm_task(void *params)
{
pwm1__init_single_edge(1000);
pin_configure_pwm_channel_as_io_pin();
pwm1__set_duty_cycle(PWM1__2_0, 100);
uint8_t percent = 0;
while (1) {
  pwm1__set_duty_cycle(PWM1__2_0, percent);
  if (++percent > 100) {
    percent = 0;
  }
  vTaskDelay(10);
}
}
*/

// part 1 and 2
/*
void pwm_task(void *p) {
  pwm1__init_single_edge(1000);
  uint16_t adc_reading = 0;
  gpio__construct_with_function(2, 0, GPIO__FUNCTION_1); // P2_0

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  uint8_t percent = 0;
  while (1) {
    // Implement code to receive potentiometer value from queue
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {

      float normalized_reading = adc_reading / 4095.0f;
      int percent = (int)(pow(normalized_reading, 2) * 100);

      printf("ADC Reading: %d, Duty Cycle Percent: %d\n", adc_reading, percent);
      pwm1__set_duty_cycle(PWM1__2_0, percent);
    }
  }
}

*/
// Part 3 and extra credits part

// part 3
/*
void pwm_task(void *p) {
  pwm1__init_single_edge(1000); // Initialize PWM at 1000 Hz
  uint16_t adc_reading = 0;

  // Initialize GPIO for RGB PWM
  gpio__construct_with_function(2, 0, GPIO__FUNCTION_1); // Red
  gpio__construct_with_function(2, 1, GPIO__FUNCTION_1); // Green
  gpio__construct_with_function(2, 2, GPIO__FUNCTION_1); // Blue

  while (1) {
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, portMAX_DELAY)) {
      // Calculate percentage for PWM duty cycle based on ADC value
      float normalized_reading = adc_reading / 4095.0f;
      int percent = (int)(pow(normalized_reading, 2) * 100);
      printf("ADC Reading: %d, PWM Percent: %d\n", adc_reading, percent);

      // Set duty cycle for RGB LED (using MR1 for Red, MR2 for Green, MR3 for Blue)
      pwm1__set_duty_cycle(PWM1__2_0, percent); // Control Red LED
      pwm1__set_duty_cycle(PWM1__2_1, percent); // Control Green LED
      pwm1__set_duty_cycle(PWM1__2_2, percent); // Control Blue LED

      // Print the values of the Match Registers
      printf("MR0: %d, MR1: %d, MR2: %d, MR3: %d\n", LPC_PWM1->MR0, LPC_PWM1->MR1, LPC_PWM1->MR2, LPC_PWM1->MR3);
    }
  }
}
*/

/// extra credits

void pwm_task(void *p) {
  pwm1__init_single_edge(1000); // Initialize PWM at 1000 Hz
  uint16_t adc_reading = 0;

  // Initialize GPIO for RGB PWM
  gpio__construct_with_function(2, 0, GPIO__FUNCTION_1); // Red
  gpio__construct_with_function(2, 1, GPIO__FUNCTION_1); // Green
  gpio__construct_with_function(2, 2, GPIO__FUNCTION_1); // Blue

  while (1) {
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, portMAX_DELAY)) {
      
      float normalized_reading = adc_reading / 4095.0f;
      int percent = (int)(pow(normalized_reading, 2) * 100);
      printf("ADC Reading: %d, PWM Percent: %d\n", adc_reading, percent);

      // Map the potentiometer value to RGB values
      uint8_t red_duty_cycle = 0;
      uint8_t green_duty_cycle = 0;
      uint8_t blue_duty_cycle = 0;

      // Create color combinations based on the potentiometer value
      if (adc_reading < 1365) { // 0 - 1364 maps to Red
        red_duty_cycle = percent;
        green_duty_cycle = 0;
        blue_duty_cycle = 0;
      } else if (adc_reading < 2730) { // 1365 - 2730 maps to Yellow (Red + Green)
        red_duty_cycle = percent;
        green_duty_cycle = percent;
        blue_duty_cycle = 0;
      } else { // 2730 - 4095 maps to Green to Blue transition
        red_duty_cycle = 0;
        green_duty_cycle = percent;
        blue_duty_cycle = percent;
      }

      // Set duty cycles for the RGB LED
      pwm1__set_duty_cycle(PWM1__2_0, red_duty_cycle);   // Control Red LED
      pwm1__set_duty_cycle(PWM1__2_1, green_duty_cycle); // Control Green LED
      pwm1__set_duty_cycle(PWM1__2_2, blue_duty_cycle);  // Control Blue LED

      // Print the values of the Match Registers
      printf("MR0: %d, MR1: %d, MR2: %d, MR3: %d\n", LPC_PWM1->MR0, LPC_PWM1->MR1, LPC_PWM1->MR2, LPC_PWM1->MR3);
    }
  }
}

void adc_pin_initialize(void) {
  LPC_IOCON->P0_25 &= ~(1 << 7);
  LPC_IOCON->P0_25 &= ~(0b111 << 3);
  gpio__construct_with_function(0, 25, GPIO__FUNCTION_1); // calling the API from gpio.h
}

void adc_task(void *p) 
{
  adc_pin_initialize();
  adc__initialize();
  adc__enable_burst_mode();
  static uint16_t adc_value = 0;

  while (1)
   {
    LPC_ADC->CR |= (1 << 2); // Selecting the ADC channel number
    adc_value = adc__get_channel_reading_with_burst_mode(2);
    float adc_voltage = (adc_value * 3.3f) / 4095.0f;
    printf("ADC Value: %d, Voltage: %.3fV\n", adc_value, adc_voltage);
    xQueueSend(adc_to_pwm_task_queue, &adc_value, 0);
    vTaskDelay(100);
  }
}

int main(void) {
  create_blinky_tasks();
  create_uart_task();

  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(uint16_t)); // making a queue

  xTaskCreate(pwm_task, "PWM", 1024 / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(adc_task, "ADC", 1024 / sizeof(void *), NULL, 1, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler(); // Start the FreeRTOS scheduler

  return 0;
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
