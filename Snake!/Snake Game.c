#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

#pragma warning(disable:4996)

//colors
#define RED 12
#define BLUE 3
#define GREEN 10
#define YELLOW 14
#define GRAY 8
#define PINK 13
#define WHITE 15
#define WAIT_TIME_MILI_SEC 100
//directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
// general
#define BOARD_SIZE 40
#define INITIAL_SNAKE_LENGTH 3
#define MINIMUM_SNAKE_LENGTH 2
#define MAX_LEN_SNAKES 30
#define NUMBER_OF_MOUSES 20
//board_characters
#define EMPTY '\0' //don't change this
#define MOUSE 'm'
#define PLAYER1_SNAKE_HEAD '1'
#define PLAYER2_SNAKE_HEAD '2'
#define PLAYER1_SNAKE_BODY 'a'
#define PLAYER2_SNAKE_BODY 'b'
//Bernard, Poison and golden star
#define BERNARD_CLOCK 'c' //on the board character
#define GOLDEN_STAR '*' //on the board character
#define POISON 'x' //on the board character
#define NUMBER_OF_POISONS 5
#define NUMBER_OF_GOLDEN_STARS 3
#define BERNARD_CLOCK_APPEARANCE_CHANCE_PERCENT 20
#define BERNARD_CLOCK_APPEARANCE_CHECK_PERIOD_MILI_SEC 2000
#define BERNARD_CLOCK_FROZEN_TIME_MILI_SEC 4000

CONSOLE_FONT_INFOEX former_cfi;
CONSOLE_CURSOR_INFO former_info;
COORD former_screen_size;

void reset_console() 
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(consoleHandle, CONSOLE_WINDOWED_MODE, &former_screen_size);
	SetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
	SetConsoleCursorInfo(consoleHandle, &former_info);
}

void hidecursor(HANDLE consoleHandle)
{
	GetConsoleCursorInfo(consoleHandle, &former_info);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void set_console_font_and_font_size(HANDLE consoleHandle) 
{
	former_cfi.cbSize = sizeof(former_cfi);
	GetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 20;
	cfi.dwFontSize.Y = 20;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy(cfi.FaceName, L"Courier");
	SetCurrentConsoleFontEx(consoleHandle, FALSE, &cfi);
}

void set_full_screen_mode(HANDLE consoleHandle) 
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	former_screen_size.X = csbi.dwSize.X; former_screen_size.Y = csbi.dwSize.Y;
	COORD coord;
	SetConsoleDisplayMode(consoleHandle, CONSOLE_FULLSCREEN_MODE, &coord);
}

void init_screen()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	set_full_screen_mode(consoleHandle);
	hidecursor(consoleHandle);
	set_console_font_and_font_size(consoleHandle);

}

void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction) 
{
	DWORD64 start_time, check_time;
	start_time = GetTickCount64();
	check_time = start_time + WAIT_TIME_MILI_SEC; //GetTickCount returns time in miliseconds
	char key = 0;
	char player1_key_hit = 0;
	char player2_key_hit = 0;

	while (check_time > GetTickCount64())
	{
		if (_kbhit())
		{
			key = _getch();
			if (key == 0)
				key = _getch();
			if (key == 'w' || key == 'a' || key == 's' || key == 'd')
				player1_key_hit = key;
			if (key == 'i' || key == 'j' || key == 'k' || key == 'l')
				player2_key_hit = key;
		}
	}

	switch (player1_key_hit) 
	{
		case 'w': if (*player1_snake_direction != DOWN) *player1_snake_direction = UP; break;
		case 'a': if (*player1_snake_direction != RIGHT) *player1_snake_direction = LEFT; break;
		case 's': if (*player1_snake_direction != UP) *player1_snake_direction = DOWN; break;
		case 'd': if (*player1_snake_direction != LEFT) *player1_snake_direction = RIGHT; break;
		default: break;
	}

	switch (player2_key_hit)
	{
		case 'i': if (*player2_snake_direction != DOWN) *player2_snake_direction = UP; break;
		case 'j': if (*player2_snake_direction != RIGHT) *player2_snake_direction = LEFT; break;
		case 'k': if (*player2_snake_direction != UP) *player2_snake_direction = DOWN; break;
		case 'l': if (*player2_snake_direction != LEFT) *player2_snake_direction = RIGHT; break;
		default: break;
	}
}

void draw_point(char point_content)
{
	switch (point_content)
	{
		case PLAYER1_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED); printf("@"); break;
		case PLAYER2_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("@"); break;
		case PLAYER1_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);  printf("o"); break;
		case PLAYER2_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("o"); break;
		case MOUSE: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); printf("m"); break;
		case GOLDEN_STAR: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW); printf("*"); break;
		case POISON: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN); printf("x"); break;
		case BERNARD_CLOCK: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PINK); printf("c"); break;
		default: printf(" ");
	}
}

void draw_horizonatal_walls()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
	for (int i = 0; i < BOARD_SIZE + 2; ++i)
		printf("-");
	printf("\n");
}

void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE])
{
	system("cls");
	draw_horizonatal_walls();
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		printf("|"); // vertical wall 
		for (int j = 0; j < BOARD_SIZE; j++)
			draw_point(board_content[i][j]);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		printf("|\n"); // vertical wall
	}
	draw_horizonatal_walls();
}

//added functions

void mice_init(char board_content[BOARD_SIZE][BOARD_SIZE], char mice_location[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	srand(time(0));
	for (int counter = 0; counter < NUMBER_OF_MOUSES; counter++)
	{
		while (TRUE)
		{
			i = rand() % BOARD_SIZE;
			j = rand() % BOARD_SIZE;
			if (board_content[i][j] == EMPTY)
			{
				break;
			}
		}
		board_content[i][j] = MOUSE;
		mice_location[i][j] = MOUSE;
	}
}

void mouse_adder(char board_content[BOARD_SIZE][BOARD_SIZE], char mice_location[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	srand(time(0));
	while (TRUE)
	{
		i = rand() % BOARD_SIZE;
		j = rand() % BOARD_SIZE;
		if (board_content[i][j] == EMPTY)
		{
			break;
		}
	}
	board_content[i][j] = MOUSE;
	mice_location[i][j] = MOUSE;
}

void golden_star_init(char board_content[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	srand(time(0));
	for (int counter = 0; counter < NUMBER_OF_GOLDEN_STARS; counter++)
	{
		while (TRUE)
		{
			i = rand() % BOARD_SIZE;
			j = rand() % BOARD_SIZE;
			if (board_content[i][j] == EMPTY)
			{
				break;
			}
		}
		board_content[i][j] = GOLDEN_STAR;
	}
}

void golden_star_adder(char board_content[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	srand(time(0));
	while (TRUE)
	{
		i = rand() % BOARD_SIZE;
		j = rand() % BOARD_SIZE;
		if (board_content[i][j] == EMPTY)
		{
			break;
		}
	}
	board_content[i][j] = GOLDEN_STAR;
}

// prototypes
void init_screen();
void reset_console();
void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction);
void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]);

int main()
{

	//::  SNAKES, FUNCTIONS, BOARD AND INTIALIZATIONS ,ALL, ARE DEFINED COPMLETELY ADAPTABLE TO CHANGING "ANY" VARIABLE  :://


	// board initializing
	char board_content[BOARD_SIZE][BOARD_SIZE] = { "" };

	//snake 1 starting point 
	for (int i = 0; i < INITIAL_SNAKE_LENGTH - 1; i++)
	{
		board_content[i][0] = 'a';
	}
	board_content[INITIAL_SNAKE_LENGTH - 1][0] = '1';

	//snake 2 starting point 
	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++)
	{
		board_content[BOARD_SIZE - i][BOARD_SIZE - 1] = 'b';
	}
	board_content[BOARD_SIZE - INITIAL_SNAKE_LENGTH][BOARD_SIZE - 1] = '2';
	

	// snakes' starting movement directions
	int player1_snake_direction = DOWN, player2_snake_direction = UP; 


	int snake1_length = INITIAL_SNAKE_LENGTH;
	//snake 1 head location
	int i_snake1_head = INITIAL_SNAKE_LENGTH - 1, j_snake1_head = 0;
	int i_snake1_head_temp = i_snake1_head, j_snake1_head_temp = j_snake1_head;
	//snake 1 body movement memory
	int move_i_snake1[MAX_LEN_SNAKES] = { 0 }, move_j_snake1[MAX_LEN_SNAKES] = { 0 }; 
	for (int i = 0; i < INITIAL_SNAKE_LENGTH - 1; i++)
	{
		move_i_snake1[i] = i;
		move_j_snake1[i] = 0;
	}


	int snake2_length = INITIAL_SNAKE_LENGTH;
	//snake 2 head location
	int i_snake2_head = BOARD_SIZE - INITIAL_SNAKE_LENGTH, j_snake2_head = BOARD_SIZE - 1;
	int i_snake2_head_temp = i_snake2_head, j_snake2_head_temp = j_snake2_head;
	//snake 2 body movement memory
	int move_i_snake2[MAX_LEN_SNAKES] = { 0 }, move_j_snake2[MAX_LEN_SNAKES] = { 0 };
	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++)
	{
		move_i_snake2[i - 1] = BOARD_SIZE - i;
		move_j_snake2[i - 1] = BOARD_SIZE - 1;
	}


	//initializing 
	char mice_location[BOARD_SIZE][BOARD_SIZE] = { "" }; // to don't lose mice's locations 
	init_screen();
	mice_init(board_content, mice_location);//draws all NUMBER_OF_MOUSES mice
	golden_star_init(board_content);//draws all NUMBER_OF_GOLDEN_STARS golden stars
	draw_board(board_content);


	while (TRUE)
	{

		////////// snake 1 TAIL //////////
		move_i_snake1[snake1_length - 1] = i_snake1_head, move_j_snake1[snake1_length - 1] = j_snake1_head; //adding head's location to memory

		if ((mice_location[i_snake1_head][j_snake1_head] == MOUSE) && (snake1_length < MAX_LEN_SNAKES))
		{
			snake1_length++;
			mouse_adder(board_content, mice_location);//to add a mouse 
			mice_location[i_snake1_head][j_snake1_head] = EMPTY;
		}

		else if ((mice_location[i_snake1_head][j_snake1_head] == MOUSE) && (snake1_length == MAX_LEN_SNAKES))//nothing happens to snake(see "else" bellow), but a mouse must be added
		{
			board_content[move_i_snake1[0]][move_j_snake1[0]] = EMPTY;
			for (int counter = 0; counter < snake1_length; counter++)
			{
				move_i_snake1[counter] = move_i_snake1[counter + 1];
				move_j_snake1[counter] = move_j_snake1[counter + 1];
			}
			mouse_adder(board_content, mice_location);//to add a mouse 
			mice_location[i_snake1_head][j_snake1_head] = EMPTY;
		}

		else
		{
			board_content[move_i_snake1[0]][move_j_snake1[0]] = EMPTY;//erasing last memory ( LAST TAiL of snake 1 )
			for (int counter = 0; counter < snake1_length; counter++)//to make a new TAiL
			{
				move_i_snake1[counter] = move_i_snake1[counter + 1];
				move_j_snake1[counter] = move_j_snake1[counter + 1];
			}
		}


		////////// snake 2 TAIL ( SIMILAR to snake 1 ) //////////
		move_i_snake2[snake2_length - 1] = i_snake2_head, move_j_snake2[snake2_length - 1] = j_snake2_head;

		if ((mice_location[i_snake2_head][j_snake2_head] == MOUSE) && (snake2_length < MAX_LEN_SNAKES))
		{
			snake2_length++;
			mouse_adder(board_content, mice_location);
			mice_location[i_snake2_head][j_snake2_head] = EMPTY;
		}

		else if ((mice_location[i_snake2_head][j_snake2_head] == MOUSE) && (snake2_length == MAX_LEN_SNAKES))
		{
			board_content[move_i_snake2[0]][move_j_snake2[0]] = EMPTY;
			for (int counter = 0; counter < snake2_length; counter++)
			{
				move_i_snake2[counter] = move_i_snake2[counter + 1];
				move_j_snake2[counter] = move_j_snake2[counter + 1];
			}
			mouse_adder(board_content, mice_location);
			mice_location[i_snake2_head][j_snake2_head] = EMPTY;
		}

		else
		{
			board_content[move_i_snake2[0]][move_j_snake2[0]] = EMPTY;
			for (int counter = 0; counter < snake2_length; counter++)
			{
				move_i_snake2[counter] = move_i_snake2[counter + 1];
				move_j_snake2[counter] = move_j_snake2[counter + 1];
			}
		}


		////////// snake 1 MOVEMENT //////////
		i_snake1_head_temp = i_snake1_head, j_snake1_head_temp = j_snake1_head;//saving snake 1 previous head location
		//locating snake 1 head
		wait_and_get_direction(&player1_snake_direction, &player2_snake_direction);
		if (player1_snake_direction == DOWN)
		{
			i_snake1_head++;
		}
		else if (player1_snake_direction == RIGHT)
		{
			j_snake1_head++;
		}
		else if (player1_snake_direction == LEFT)
		{
			j_snake1_head--;
		}
		else if (player1_snake_direction == UP)
		{
			i_snake1_head--;
		}

		// to make sure snake stay on map
		if (i_snake1_head > BOARD_SIZE - 1) { i_snake1_head = 0; }
		if (i_snake1_head < 0) { i_snake1_head = BOARD_SIZE - 1; }
		if (j_snake1_head > BOARD_SIZE - 1) { j_snake1_head = 0; }
		if (j_snake1_head < 0) { j_snake1_head = BOARD_SIZE - 1; }

		// when snake 1 runs into either of two snakes' body
		if ((board_content[i_snake1_head][j_snake1_head] == 'a') || (board_content[i_snake1_head][j_snake1_head] == 'b'))
		{
			reset_console();
			system("cls");
			printf("BLUE WINS!\n");
			break;
		}

		//when snakes face each other's head
		if (board_content[i_snake1_head][j_snake1_head] == '2')
		{
			if (snake1_length > snake2_length)
			{
				reset_console();
				system("cls");
				printf("RED WINS!\n");
				break;
			}
			else if (snake1_length < snake2_length)
			{
				reset_console();
				system("cls");
				printf("BLUE WINS!\n");
				break;
			}
			else
			{
				reset_console();
				system("cls");
				printf("DRAW!!\n");
				break;
			}
		}

		//when snake 1 eats a golden star
		if (board_content[i_snake1_head][j_snake1_head] == GOLDEN_STAR)
		{
			snake2_length--;//other snake loses last part of body (tail)
			if (snake2_length < MINIMUM_SNAKE_LENGTH)//if snake's length becomes less than MINIMUM_SNAKE_LENGTH , snake loses the game
			{
				reset_console();
				system("cls");
				printf("RED WINS!\n");
				break;
			}
			board_content[move_i_snake2[0]][move_j_snake2[0]] = EMPTY;// new body locations
			for (int counter = 0; counter < snake2_length; counter++)
			{
				move_i_snake2[counter] = move_i_snake2[counter + 1];
				move_j_snake2[counter] = move_j_snake2[counter + 1];
			}
			golden_star_adder(board_content);//add a star to have NUMBER_OF_GOLDEN_STARS stars always
		}

		//if no one loses, snake continues moving
		board_content[i_snake1_head][j_snake1_head] = '1';//moving snake 1 head forward
		if (snake1_length != 1)//adding a part of body 'o' to snake 1 head's last location
		{
			board_content[i_snake1_head_temp][j_snake1_head_temp] = 'a';
		}


		////////// snake 2 MOVEMENT ( SIMILAR to snake 1 ) //////////
		i_snake2_head_temp = i_snake2_head, j_snake2_head_temp = j_snake2_head;

		if (player2_snake_direction == DOWN)
		{
			i_snake2_head++;
		}
		else if (player2_snake_direction == RIGHT)
		{
			j_snake2_head++;
		}
		else if (player2_snake_direction == LEFT)
		{
			j_snake2_head--;
		}
		else if (player2_snake_direction == UP)
		{
			i_snake2_head--;
		}

		if (i_snake2_head > BOARD_SIZE - 1) { i_snake2_head = 0; }
		if (i_snake2_head < 0) { i_snake2_head = BOARD_SIZE - 1; }
		if (j_snake2_head > BOARD_SIZE - 1) { j_snake2_head = 0; }
		if (j_snake2_head < 0) { j_snake2_head = BOARD_SIZE - 1; }


		if ((board_content[i_snake2_head][j_snake2_head] == 'a') || (board_content[i_snake2_head][j_snake2_head] == 'b'))
		{
			reset_console();
			system("cls");
			printf("RED WINS!\n");
			break;
		}

		if (board_content[i_snake2_head][j_snake2_head] == '1')
		{
			if (snake1_length > snake2_length)
			{
				reset_console();
				system("cls");
				printf("RED WINS!\n");
				break;
			}
			else if (snake1_length < snake2_length)
			{
				reset_console();
				system("cls");
				printf("BLUE WINS!\n");
				break;
			}
			else
			{
				reset_console();
				system("cls");
				printf("DRAW!!\n");
				break;
			}
		}

		if (board_content[i_snake2_head][j_snake2_head] == GOLDEN_STAR)
		{
			snake1_length--;
			if (snake1_length < MINIMUM_SNAKE_LENGTH)
			{
				reset_console();
				system("cls");
				printf("BLUE WINS!\n");
				break;
			}
			board_content[move_i_snake1[0]][move_j_snake1[0]] = EMPTY;
			for (int counter = 0; counter < snake1_length; counter++)
			{
				move_i_snake1[counter] = move_i_snake1[counter + 1];
				move_j_snake1[counter] = move_j_snake1[counter + 1];
			}
			golden_star_adder(board_content);
		}

		board_content[i_snake2_head][j_snake2_head] = '2';
		if (snake2_length != 1)
		{
			board_content[i_snake2_head_temp][j_snake2_head_temp] = 'b';
		}


		draw_board(board_content);//updates game board after changes
	}

	return 0;
}