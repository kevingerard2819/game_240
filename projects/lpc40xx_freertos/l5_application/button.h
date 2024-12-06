#ifndef BUTTON_H
#define BUTTON_H

#include "FreeRTOS.h"
#include "semphr.h"

// Global semaphore handle for button press signaling
extern SemaphoreHandle_t button_semaphore;

// Function prototypes
void button_init(void);

#endif // BUTTON_H
