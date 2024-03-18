#include "MovingAverage.h"

moving_average_t init_moving_average(void)
{
    moving_average_t new_ma = malloc(sizeof(MovingAverage));

    new_ma->front = 0;
	new_ma->rear = WINDOW_SIZE - 1;
	new_ma->count = 0;

    return new_ma;
}

void moving_average_enqueue(moving_average_t ma, int item)
{
	ma->rear = (ma->rear + 1) % WINDOW_SIZE;
	if (ma->count >= WINDOW_SIZE)
	{
		ma->front = (ma->front + 1) % WINDOW_SIZE;
	}
	else
	{
		ma->count++;
	}
	ma->data[ma->rear] = item;
}

float get_moving_average(moving_average_t ma)
{
	float sum = 0;
	for (int i = 0; i < ma->count; i++)
	{
		sum += ma->data[(ma->front + i) % WINDOW_SIZE];
	}
	return (sum / ma->count);
}