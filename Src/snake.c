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
	baitPos.x /= STEP_SIZE;
	baitPos.x *= STEP_SIZE;

	HAL_RNG_GenerateRandomNumber(&hrng, &baitPos.y);
	baitPos.y %=128;
	baitPos.y /= STEP_SIZE;
	baitPos.y *= STEP_SIZE;

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
	static bool isMotionDetected = false;
	switch(dir )
	{
	case UP:
		if(! (snake->dir.y == STEP_SIZE) )
		{
			snake->dir.y = -STEP_SIZE;
			snake->dir.x = 0;
		}
		isMotionDetected = true;
		break;

	case RIGHT:
		if(! (snake->dir.x == -STEP_SIZE) )
		{
			snake->dir.x = STEP_SIZE;
			snake->dir.y = 0;
		}
		isMotionDetected = true;
		break;

	case LEFT:
		if(! (snake->dir.x == STEP_SIZE) )
		{
			snake->dir.x = -STEP_SIZE;
			snake->dir.y = 0;
		}
		isMotionDetected = true;
		break;

	case DOWN:
		if(! (snake->dir.y == -STEP_SIZE) )
		{
			snake->dir.y = STEP_SIZE;
			snake->dir.x = 0;
		}
		isMotionDetected = true;
		break;


	default:
		break;

	}

	// Move the head
	snake->head.x += snake->dir.x;
	snake->head.y += snake->dir.y;

	snake->head.x %= 128;
	snake->head.y %= 128;

	snake->head.x /= STEP_SIZE;
	snake->head.x *= STEP_SIZE;

	snake->head.y /= STEP_SIZE;
	snake->head.y *= STEP_SIZE;

	if( (snake->head.x % 3 != 0) || (snake->head.y % 3 != 0) )
	{
		asm volatile ("nop");
	}

	// Check if we ate ourself
	for(int i = snake->len -1; i >= 0; i--)
	{
		if(snake->head.x == snake->body[i].x &&
				snake->head.y == snake->body[i].y && isMotionDetected)
		{
			snake->score = snake->len;
			snake->len = 0;
			snake->head.x = 63;
			snake->head.y = 63;
		}
	}

	// Check if we ate the bait
	if(snake->head.x == bait->x && snake->head.y == bait->y)
	{
		snake->len += SCORE_STEP;
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
#else
	ST7735_DrawPixel(snake->body[0].x, snake->body[0].y, WHITE);
#endif

#ifdef ENABLE_BOLD_SNAKE
	ST7735_DrawPixel(snake->body[0].x, snake->body[0].y+1, WHITE);
	ST7735_DrawPixel(snake->body[0].x, snake->body[0].y-1, WHITE);
	ST7735_DrawPixel(snake->body[0].x+1, snake->body[0].y, WHITE);
	ST7735_DrawPixel(snake->body[0].x-1, snake->body[0].y, WHITE);
	ST7735_DrawPixel(snake->body[0].x+1, snake->body[0].y+1, WHITE);
	ST7735_DrawPixel(snake->body[0].x+1, snake->body[0].y-1, WHITE);
	ST7735_DrawPixel(snake->body[0].x-1, snake->body[0].y+1, WHITE);
	ST7735_DrawPixel(snake->body[0].x-1, snake->body[0].y-1, WHITE);
#endif
}

static void clear_snake(snake_t *snake)
{
#ifdef DRAW_SNAKE_LOW_FPS
	int tempLen = snake->len;
	while(tempLen--)
	{
		ST7735_DrawPixel(snake->body[tempLen].x, snake->body[tempLen].y, BLACK);
	}
#else
	ST7735_DrawPixel(snake->body[snake->len].x, snake->body[snake->len].y, BLACK);
#endif

#ifdef ENABLE_BOLD_SNAKE
	ST7735_DrawPixel(snake->body[snake->len].x, snake->body[snake->len].y, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x, snake->body[snake->len].y+1, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x, snake->body[snake->len].y-1, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x+1, snake->body[snake->len].y, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x-1, snake->body[snake->len].y, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x+1, snake->body[snake->len].y+1, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x+1, snake->body[snake->len].y-1, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x-1, snake->body[snake->len].y+1, BLACK);
	ST7735_DrawPixel(snake->body[snake->len].x-1, snake->body[snake->len].y-1, BLACK);
#endif
}

static direction_t process_input(void)
{
	uint8_t key;
	//HAL_UART_Receive(&huart3, &key, 1, 1);
	/* Workaround for overrun error. When read data buffer is overrun, status register
	 * should be read and overrun flag must be cleared to be able to read new data.
	 */
	uint32_t dummyRead = *(uint32_t *)0x4000481c;
	*(uint32_t *)0x40004820 |= (0x1 << 3);
	key = (uint8_t )*(uint32_t *)0x40004824;
	return (direction_t)key;
}

static void draw_bait(vec2_t *bait)
{
	ST7735_DrawPixel(bait->x, bait->y, WHITE);
#ifdef ENABLE_BOLD_SNAKE
	ST7735_DrawPixel(bait->x, bait->y-1, WHITE);
	ST7735_DrawPixel(bait->x, bait->y+1, WHITE);
	ST7735_DrawPixel(bait->x+1, bait->y, WHITE);
	ST7735_DrawPixel(bait->x+1, bait->y-1, WHITE);
	ST7735_DrawPixel(bait->x+1, bait->y, WHITE);
	ST7735_DrawPixel(bait->x+1, bait->y+1, WHITE);
	ST7735_DrawPixel(bait->x-1, bait->y+1, WHITE);
	ST7735_DrawPixel(bait->x-1, bait->y, WHITE);
	ST7735_DrawPixel(bait->x-1, bait->y-1, WHITE);
#endif
}

static void clear_bait(vec2_t *bait)
{
	ST7735_DrawPixel(bait->x, bait->y, BLACK);
#ifdef ENABLE_BOLD_SNAKE
	ST7735_DrawPixel(bait->x, bait->y-1, BLACK);
	ST7735_DrawPixel(bait->x, bait->y+1, BLACK);
	ST7735_DrawPixel(bait->x+1, bait->y, BLACK);
	ST7735_DrawPixel(bait->x+1, bait->y-1, BLACK);
	ST7735_DrawPixel(bait->x+1, bait->y, BLACK);
	ST7735_DrawPixel(bait->x+1, bait->y+1, BLACK);
	ST7735_DrawPixel(bait->x-1, bait->y+1, BLACK);
	ST7735_DrawPixel(bait->x-1, bait->y, BLACK);
	ST7735_DrawPixel(bait->x-1, bait->y-1, BLACK);
#endif
}

static bool is_failed(snake_t *snake)
{
	bool retVal = false;

	if(snake->len == 0)
	{
		retVal = true;
	}

	return retVal;
}

void snake_reset(snake_t *snake)
{
	/* Print the score and clear the screen */
	char buffer[4];
	ST7735_FillRectangle(0, 0, 128, 128, BLACK);
	ST7735_WriteString(0, 0, "YOUR    SCORE   IS: ", Font_16x26, CYAN, BLACK);
	itoa (snake->score,buffer,10);
	ST7735_WriteString(3*16, 2*26, buffer, Font_16x26, CYAN, BLACK);
	HAL_Delay(1000);
	ST7735_FillRectangle(0, 0, 128, 128, BLACK);


}

void snake_main(void)
{
snake_main:
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
		}

		if(is_failed(&mySnake) )
		{
			snake_reset(&mySnake);
			goto snake_main;
		}
		draw_bait(&bait);
		clear_snake(&mySnake);
		update_body(&mySnake);
		draw_snake(&mySnake);
		HAL_Delay(50);

	}

}



