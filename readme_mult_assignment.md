code :
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>
int main(void) {
puts("Starting RTOS");
  xTaskCreate(task_one, "Task One", 1000, NULL, 2, NULL);

  xTaskCreate(task_two, "Task Two", 1000, NULL, 1, NULL);

  
  vTaskStartScheduler();
  while(1);

  return 0;
}
static void task_one(void *task_parameter) {
  while (true) {
    fprintf(stderr, "AAAAAAAAAAAA");
    vTaskDelay(100);
  }
  
}
static void task_two(void *task_parameter) {
  while (true) {
    fprintf(stderr, "bbbbbbbbbbbb");
    vTaskDelay(100);
  }
  
}


baudrate : 38400 bits per second  -> 3840 bytes per second  -> 3.84 byte per second
tick time: 1ms

Question 1 : 
-How come 4(or 3 sometimes) characters are printed from each task? Why not 2 or 5, or 6?

Ans: In this scenario, each task will share a time slice of one millisecond and have the same priority.
Because of this, every task will execute for one millisecond before being pushed back to the end of the round-robin scheduler's queue.
Therefore, the task will complete whatever it can in one millisecond. Because it takes one byte, or ten bits, to transfer one character, three or even four characters are printed by each task. Three and four characters translate into three and four bytes of data transferred every second, and utilizing a serial port, we can transport 3.84 bytes per second based on baud rate.

Question 2
Alter the priority of one of the tasks, and note down the observations. Note down WHAT you see and WHY.
Ans: 
Task 1 is given more priority than Task 2, therefore it will finish its execution before sleeping for a duration of 100 milliseconds. As task 1 shuts down, task 2 will begin to execut; in the interim, the CPU will operate at peak efficiency until job 1 resumes.
Task1 - priority 1, Task1 - priority 2: In this scenario, task 2 will execute entirely because it has a higher priority than the previous one,
additionally, it doesn't split time with other tasks. Task 1 will begin running and finish its execution before retiring to rest after it has finished.
If same priority , since it follows the RTOS follows round scheduling both tasks share the time and print at intervals .

Round-Robin Scheduling: For equal task priority, round-robin scheduling ensures that following every 1 millisecond tick, the two tasks run one after another, hence in this case, characters are printed in a quantized 'interleaved' pattern. Higher priority tasks preempt lower priority tasks. If you ever increase one task's priority, that task would resume running without interference from the other until it is either completed or yields, which means it will get more CPU time and print more characters.
As per the calculations the baudrate si 38400 bits per second  which is  3840 bytes per second  hence 3.84 byte per second so the ,
tick time: 1ms
This behavior is consistent with the principles of RTOS scheduling, ensuring that higher priority tasks run first and that equal priority tasks share CPU time equally.

![Screenshot of Same priority](images/same_priority.png)

![Screenshot of Differet  priority](images/different.png)