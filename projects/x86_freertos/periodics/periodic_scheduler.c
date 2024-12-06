#include "periodic_callbacks.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

// #include "common_macros.h"

/// Task data structure of each periodic task
typedef struct {
  /// Task uses vTaskDelayUntil() to carry out its periodic callback
  const TickType_t task_delay_in_ticks;

  /// This callback is invoked from the task
  periodic_callbacks_f callback;

  /// Incremented until task_delay_in_ticks and then task_finished_flag is checked for deadline
  TickType_t ticks_elapsed;

  /// After the callback() is invoked, this flag is set which is later checked by periodic_scheduler__task_monitor()
  volatile bool task_finished_flag;
} periodic_scheduler_s;

/// Instances of the 4 periodic tasks and their callback function pointer
static periodic_scheduler_s periodic_scheduler__1Hz = {1000, periodic_callbacks__1Hz};
static periodic_scheduler_s periodic_scheduler__10Hz = {100, periodic_callbacks__10Hz};
static periodic_scheduler_s periodic_scheduler__100Hz = {10, periodic_callbacks__100Hz};
static periodic_scheduler_s periodic_scheduler__1000Hz = {1, periodic_callbacks__1000Hz};

static bool periodic_scheduler__run_1000hz;

/// Common task runner for each periodic task
static void periodic_scheduler__run(periodic_scheduler_s *periodic) {
  TickType_t previous_tick = 0;
  uint32_t callback_counter = 0;

  // Note: the order of the while loop's logic is critical; change with care
  while (true) {
    periodic->callback(callback_counter);
    ++callback_counter;
    periodic->task_finished_flag = true;

    vTaskDelayUntil(&previous_tick, periodic->task_delay_in_ticks);
  }
}

static void periodic_scheduler__1Hz_task(void *param) { periodic_scheduler__run(&periodic_scheduler__1Hz); }
static void periodic_scheduler__10Hz_task(void *param) { periodic_scheduler__run(&periodic_scheduler__10Hz); }
static void periodic_scheduler__100Hz_task(void *param) { periodic_scheduler__run(&periodic_scheduler__100Hz); }
static void periodic_scheduler__1000Hz_task(void *param) { periodic_scheduler__run(&periodic_scheduler__1000Hz); }

#include <stdio.h>
static void periodic_scheduler__check_flag(periodic_scheduler_s *periodic_task, TickType_t tick_delay) {
  periodic_task->ticks_elapsed += tick_delay;

  if (periodic_task->ticks_elapsed >= periodic_task->task_delay_in_ticks) {
    periodic_task->ticks_elapsed = 0;

    if (periodic_task->task_finished_flag) 
    {
      periodic_task->task_finished_flag = false;
    } 
    else 
    {
      fprintf(stderr, "periodic_callbacks__%uHz exceeded the tick limit.\nThe board is resetting.",
              (unsigned) (1000 / periodic_task->task_delay_in_ticks));
      vTaskDelay(3 * 1000);
      NVIC_SystemReset();
    }
  }
}
static void periodic_scheduler__task_monitor(void *param) {
  const TickType_t tick_delay = periodic_scheduler__run_1000hz ? 1 : 10;

  // We let all the other tasks run first, and then check their flags
  while (true) {
    vTaskDelay(tick_delay);

    if (periodic_scheduler__run_1000hz) {
      periodic_scheduler__check_flag(&periodic_scheduler__1000Hz, tick_delay);
    }

    periodic_scheduler__check_flag(&periodic_scheduler__100Hz, tick_delay);
    periodic_scheduler__check_flag(&periodic_scheduler__10Hz, tick_delay);
    periodic_scheduler__check_flag(&periodic_scheduler__1Hz, tick_delay);
  }
}

void periodic_scheduler__initialize(void) {
  /* You can modify these stack sizes to suit your needs
   * Generally, you should not need higher than 4k stack size as you should not allocate
   * large data objects on the stack memory
   */

  // xTaskCreate(hello_thread, "1", 1, NULL, PRIORITY_LOW, NULL);

  // clang-format off
  xTaskCreate(periodic_scheduler__1Hz_task, "1Hz", 1000, NULL, 1, NULL);
  xTaskCreate(periodic_scheduler__10Hz_task, "10Hz", 1000, NULL, 2, NULL);
  xTaskCreate(periodic_scheduler__100Hz_task, "100Hz", 1000, NULL, 3, NULL);
  // clang-format on

  xTaskCreate(periodic_scheduler__task_monitor, "Hz_wdt", 1000, NULL, 1, NULL);

  periodic_callbacks__initialize();
}

void periodic_scheduler__initialize_with_1khz(void) {
  xTaskCreate(periodic_scheduler__1000Hz_task, "1000Hz", 1000, NULL, 3, NULL);

  periodic_scheduler__initialize();
}
