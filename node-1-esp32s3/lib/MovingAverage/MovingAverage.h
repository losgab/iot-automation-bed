/*
    Moving Average for ESP32 (Struct based)

    Convenience library for a moving average data denoiser.
	Implements a basic circular queue.

    Improvements:
    - Configurable window size

    Written by Gabriel Thien 2023
*/
#include <stdlib.h>

#define WINDOW_SIZE 30 // TO BE REMOVED LATER

typedef struct MovingAverage
{
    int data[WINDOW_SIZE];
    int front;
    int rear;
    int count;
} MovingAverage;

typedef MovingAverage* moving_average_t;

/**
 * @brief Initialises a Moving Average struct
 * 
 * @return moving_average_t
*/
moving_average_t init_moving_average(void);

/**
 * @brief Enqueues item to the end of the queue
 * 
 * @param moving_average_t Moving Average struct pointer
 * @param int item to be queued
 * 
 * @return void
*/
void moving_average_enqueue(moving_average_t ma, int item);

/**
 * @brief Calculates moving average from the items currently in the queue
 * 
 * @param moving_average_t Moving Average struct pointer
 * 
 * @return Calculated Moving Average
*/
float get_moving_average(moving_average_t ma);