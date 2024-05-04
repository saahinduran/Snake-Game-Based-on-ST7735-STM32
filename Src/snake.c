/*
 * snake.c
 *
 *  Created on: Apr 29, 2024
 *      Author: sahin
 */

#include "main.h"
#include "snake.h"
#include <ST7735.h>

extern RNG_HandleTypeDef hrng;
extern UART_HandleTypeDef huart3;

static vec2_t generate_bait(void)
{
	vec2_t baitPos;
	HAL_RNG_GenerateRandomNumber(&hrng, &baitPos.x);
	baitPos.x %=128;

	HAL_RNG_GenerateRandomNumber(&hrng, &baitPos.y);
	baitPos.y %=128;

	return baitPos;
}

static void update_body(snake_t *snake)
{
	/* This block shifts the snake's body by one iteration*/
	/* What is happening here is nothing but body following head */
	for (int i = snake->len; i > 0; i--)
	{
		snake->body[i] = snake->body[i - 1];
	}
	snake->body[0] = snake->head;

}

static bool update_head(snake_t *snake, direction_t dir, vec2_t *bait)
{
	bool isBaitEaten = false;
	switch(dir )
	{
	case UP:
		if(! (snake->dir.y == 1) )
		{
			snake->dir.y = -1;
			snake->dir.x = 0;
		}
		break;

	case RIGHT:
		if(! (snake->dir.x == -1) )
		{
			snake->dir.x = 1;
			snake->dir.y = 0;
		}
		break;

	case LEFT:
		if(! (snake->dir.x == 1) )
		{
			snake->dir.x = -1;
			snake->dir.y = 0;
		}
		break;

	case DOWN:
		if(! (snake->dir.y == -1) )
		{
			snake->dir.y = 1;
			snake->dir.x = 0;
		}
		break;


	default:
		break;

	}

	// Move the head
	snake->head.x += snake->dir.x;
	snake->head.y += snake->dir.y;

	snake->head.x %= 128;
	snake->head.y %= 128;

	// Check if we ate the bait
	if(snake->head.x == bait->x && snake->head.y == bait->y)
	{
		snake->len += 50;
		isBaitEaten = true;
	}

	return isBaitEaten;
}

static void draw_snake(snake_t *snake)
{
#ifdef DRAW_SNAKE_LOW_FPS
	int tempLen = snake->len;
	while(tempLen--)
	{
		ST7735_DrawPixel(snake->body[tempLen].x, snake->body[tempLen].y, WHITE);
	}
#endif
	ST7735_DrawPixel(snake->body[0].x, snake->body[0].y, WHITE);
}

static void clear_snake(snake_t *snake)
{
#ifdef DRAW_SNAKE_LOW_FPS
	int tempLen = snake->len;
	while(tempLen--)
	{
		ST7735_DrawPixel(snake->body[tempLen].x, snake->body[tempLen].y, BLACK);
	}
#endif
	ST7735_DrawPixel(snake->body[snake->len].x, snake->body[snake->len].y, BLACK);

}

static direction_t process_input(void)
{
	uint8_t key;
	//HAL_UART_Receive(&huart3, &key, 1, 1);
	uint32_t dummyRead = *(uint32_t *)0x4000481c;
	*(uint32_t *)0x40004820 |= (0x1 << 3);
	key = (uint8_t )*(uint32_t *)0x40004824;
	return (direction_t)key;
}

static void draw_bait(vec2_t *bait)
{
	ST7735_DrawPixel(bait->x, bait->y, WHITE);

}

static void clear_bait(vec2_t *bait)
{
	ST7735_DrawPixel(bait->x, bait->y, BLACK);
}

void snake_main(void)
{
	snake_t mySnake = {0};
	mySnake.head.x = 63;
	mySnake.head.y = 63;
	mySnake.len = 1;
	direction_t dir;
	vec2_t bait;
	bait = generate_bait();
	draw_bait(&bait);
	while(1)
	{

		dir = process_input();
		if( update_head(&mySnake, dir, &bait) )
		{
			clear_bait(&bait);
			bait = generate_bait();
			draw_bait(&bait);
		}
		clear_snake(&mySnake);
		update_body(&mySnake);
		draw_snake(&mySnake);
		HAL_Delay(50);

	}

}