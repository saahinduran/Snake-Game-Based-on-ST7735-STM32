/*
 * snake.h
 *
 *  Created on: Apr 29, 2024
 *      Author: sahin
 */

#ifndef INC_SNAKE_H_
#define INC_SNAKE_H_

#define MAX_LEN 256
#define ENABLE_BOLD_SNAKE
#define STEP_SIZE 3

#define AND &&
#define OR ||

typedef struct vec2
{
	uint8_t x;
	uint8_t y;
}vec2_t;

typedef struct vec2_Int
{
	int8_t x;
	int8_t y;
}vec2_Int_t;


typedef enum Direction
{
	UP = '8',
	RIGHT = '6',
	LEFT = '4',
	DOWN = '2',


}direction_t;

typedef enum IsFailed
{
	NOFAIL = 0x0,
	FAIL,

}isfailed_t;

typedef struct Snake
{
	vec2_t body[MAX_LEN];
	vec2_t head;
	vec2_Int_t dir;
	uint8_t len;

}snake_t;



void snake_main(void);
#endif /* INC_SNAKE_H_ */
