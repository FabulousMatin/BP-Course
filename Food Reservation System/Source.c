#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERS_FILE_ADDRESS "users.txt"
#define FOODS_FILE_ADDRESS "food_report.txt"
#define HALLS_FILE_ADDRESS "halls_report.txt"
#define LOGGED_IN 1
#define LOGGED_OUT 0
#define TRUE 1
#define EXIST 1
#define NOT_EXIST 0
#define VALID 1
#define NOT_VALID 0
#define ADMIN 2
#define USER 1
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin"
#define SHOW_MONEY 1
#define GET_MONEY 2
#define NUMBER_OF_DAYS 7

//::all strings are dynamic::\\

typedef struct reserved_food
{
	int day;
	int food_id; // we can find food's name and hall id by food id, so we don't need to allocate extra memory for them 
	int spent_money;
	struct reserved_food* next;

}reserved_food;

typedef struct user
{
	char* username;	
	unsigned long long int id;
	char* password;
	int money;
	struct reserved_food* reserved_foods;
	struct user* next;

}user;

typedef struct hall
{
	char* name;
	int hall_id;
	int capacity;
	// every hall has a uniqe reserved foods and profit for each day so we need a new variable for each day
	int reserved[NUMBER_OF_DAYS];
	int profit[NUMBER_OF_DAYS];
	struct hall* next;

}hall;

typedef struct food
{
	char* name;
	int food_id;
	int price;
	int capacity;
	int hall_id;
	int day;
	int reserved;
	int profit;
	struct food* next;

}food;

typedef struct coupon
{
	int coupon_id;
	int percentage;
	int capacity;
	int reserved;
	struct coupon* next;

}coupon;

user* read_and_save_from_file(user* users)
{
	FILE* saved_users = fopen(USERS_FILE_ADDRESS, "r");
	if (saved_users == NULL)//if file doesnt exist, make one
	{
		saved_users = fopen(USERS_FILE_ADDRESS, "w");
		fclose(saved_users);
		saved_users = fopen(USERS_FILE_ADDRESS, "r");
		fseek(saved_users, 0, SEEK_SET);
	}

	while (fgetc(saved_users) != EOF)
	{
		fseek(saved_users, -1, SEEK_CUR);

		//allocation
		user* new_user;
		new_user = (user*)malloc(1 * sizeof(user));
		if (new_user == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		new_user->next = users;
		users = new_user;
		new_user->money = 0;
		new_user->reserved_foods = NULL;

		//read username
		char nextchar;
		int number_of_chars = 1;
		new_user->username = (char*)malloc(number_of_chars * sizeof(char));
		if (new_user->username == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		while ((nextchar = fgetc(saved_users)) != ' ')
		{
			(new_user->username)[number_of_chars - 1] = nextchar;
			number_of_chars++;
			new_user->username = (char*)realloc(new_user->username, number_of_chars * sizeof(char));
		}
		(new_user->username)[number_of_chars - 1] = NULL;


		//read id
		fscanf(saved_users, "%llu", &(new_user->id));


		//read password
		fseek(saved_users, 1, SEEK_CUR);
		number_of_chars = 1;
		new_user->password = (char*)malloc(number_of_chars * sizeof(char));
		if (new_user->password == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		while (((nextchar = fgetc(saved_users)) != '\n'))
		{
			if ((nextchar == EOF))
			{
				break;
			}
			(new_user->password)[number_of_chars - 1] = nextchar;
			number_of_chars++;
			new_user->password = (char*)realloc(new_user->password, number_of_chars * sizeof(char));
		}
		(new_user->password)[number_of_chars - 1] = NULL;

	}

	fclose(saved_users);
	return users;
}

user* to_signup(user* users, char* username, char* password, char* user_id, int if_login, int number_of_commands)
{
	if (if_login == LOGGED_IN)
	{
		printf("can't signup when you are logged in\n");
		return users;
	}
	if (number_of_commands != 4)//command must be valid
	{
		printf("invalid command\n");
		return users;
	}

	int if_exist = NOT_EXIST;
	user* temp = users;
	unsigned long long int id = atoi(user_id);
	
	//username and id is uniqe for everyone
	while (users != NULL)
	{
		if ((users->id == id) || ((strcmp(username, users->username)) == 0) || ((strcmp(username, ADMIN_USERNAME)) == 0))
		{
			if_exist = EXIST;
			break;
		}
		users = users->next;
	}
	users = temp;
	if (if_exist == EXIST)
	{
		printf("user already exists\n");
		return users;
	}

	//allocation
	else
	{
		user* new_user;
		new_user = (user*)malloc(1 * sizeof(user));
		if (new_user == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		new_user->next = users;
		users = new_user;
		new_user->money = 0;
		new_user->reserved_foods = NULL;

		//to save on program
		new_user->username = (char*)malloc((strlen(username) + 1) * sizeof(char));
		new_user->password = (char*)malloc((strlen(password) + 1) * sizeof(char));
		if (new_user->username == NULL || new_user->password == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		strcpy(new_user->username, username);
		strcpy(new_user->password, password);
		new_user->id = id;

		//to write on file
		FILE* saved_users = fopen(USERS_FILE_ADDRESS, "r+");
		fseek(saved_users, -1, SEEK_END);
		char lastchar;
		if (((lastchar = fgetc(saved_users)) != '\n') && (lastchar != EOF))// to always have a unique file format which this program is based on
		{
			fseek(saved_users, 0, SEEK_END);
			fputc('\n', saved_users);
		}
		fseek(saved_users, 0, SEEK_END);
		fprintf(saved_users, "%s %llu", username, id);
		fprintf(saved_users, " %s\n", password);
		fclose(saved_users);


		printf("signed up successfully\n");
		return users;
	}
}

char** get_and_parse_command_line(char** parsed_command, char* command_line, int* to_return_number_of_commands)
{
	int number_of_commands = 1;
	int command_length = 1;

	parsed_command = (char**)malloc(number_of_commands * sizeof(char*));
	parsed_command[0] = (char*)malloc(command_length * sizeof(char));
	if (parsed_command == NULL || parsed_command[0] == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}

	//parse command line and save each word in an array
	for (int i = 0; command_line[i] != NULL; i++)
	{
		if (command_line[i] == ' ') // for any spcace bar, make a new string and save characters in it
		{
			parsed_command[number_of_commands - 1][command_length - 1] = NULL;
			command_length = 1;
			number_of_commands++;
			parsed_command = (char**)realloc(parsed_command, number_of_commands * sizeof(char*));
			parsed_command[number_of_commands - 1] = (char*)malloc(command_length * sizeof(char));
			if (parsed_command[number_of_commands - 1] == NULL)
			{
				printf("program crashed, please try again later\n");
				exit(EXIT_FAILURE);
			}
		}
		else // save characters in order
		{
			parsed_command[number_of_commands - 1][command_length - 1] = command_line[i];
			command_length++;
			parsed_command[number_of_commands - 1] = (char*)realloc(parsed_command[number_of_commands - 1], command_length * sizeof(char));
		}
	}
	parsed_command[number_of_commands - 1][command_length - 1] = NULL;

	*to_return_number_of_commands = number_of_commands; // number of commands is needed to know if command line is valid or not
	return parsed_command;
}

void to_login(user* users, char** parsed_command, int* if_login, int number_of_commands, int* if_admin, char** logged_in_user)
{
	//check if login is valid
	int if_valid = NOT_VALID;

	if (*if_login == LOGGED_IN)
	{
		printf("already logged in\n");
	}
	else if (number_of_commands != 3)
	{
		printf("invalid command\n");
	}
	else if (number_of_commands == 3)
	{
		//admin login
		if (((strcmp((parsed_command[1]), ADMIN_USERNAME)) == 0) && ((strcmp((parsed_command[2]), ADMIN_PASSWORD)) == 0))
		{
			*if_admin = ADMIN;
			*if_login = LOGGED_IN;
			printf("welcome admin\n");
		}
		else
		{	
			//user login
			while(users != NULL)
			{	
				//check if user exists
				if (((strcmp((parsed_command[1]), users->username)) == 0) && ((strcmp((parsed_command[2]), users->password)) == 0))
				{
					printf("welcome %s\n", users->username);
					*if_login = LOGGED_IN;
					*if_admin = USER;
					if_valid = VALID;
					(*logged_in_user) = (char*)malloc((strlen(users->username)+1) * sizeof(char));
					strcpy((*logged_in_user), (users->username));
					break;
				}
				users = users->next;
			}
			if (if_valid == NOT_VALID)
			{
				printf("wrong username or password\n");
			}
		}
	}

}

void to_logout(int* if_login, int* if_admin, char** logged_in_user)
{	
	//check if logout is valid
	if (*if_login == LOGGED_IN)
	{
		*if_admin = LOGGED_OUT;
		*if_login = LOGGED_OUT;
		free(*logged_in_user);
		(*logged_in_user) = NULL;
		printf("logged out successfully\n");
	}
	else
	{
		printf("no one is logged in to logout\n");
	}
}

hall* add_hall(hall* halls, char* name, char* hall_id_str, char* capacity_str, int if_admin, int number_of_commands)
{
	if (if_admin == USER)// Only logged in admin can add hall
	{
		printf("permission denied\n");
		return halls;
	}
	else if ((if_admin == LOGGED_OUT))
	{
		printf("access denied\n");
		return halls;
	}
	else if (number_of_commands != 4)//command must be valid
	{
		printf("invalid command\n");
		return halls;
	}

	hall* halls_head_temp = halls;
	int hall_id = atoi(hall_id_str);
	int capacity = atoi(capacity_str);
	//hall id must be unique
	while(halls != NULL)
	{
		if (halls->hall_id == hall_id)
		{
			printf("hall id is not unique\n");
			return halls_head_temp;
		}
		halls = halls->next;
	}
	halls = halls_head_temp;

	//allocation
	hall* new_hall;
	new_hall = (hall*)malloc(1 * sizeof(hall));
	if (new_hall == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}
	//add a hall node to the list in order of hall ids
	int pos = 1;
	while (halls != NULL)
	{
		if (halls->hall_id < hall_id)
		{
			pos++;
			halls = halls->next;
		}
		else
		{
			break;
		}
	}
	halls = halls_head_temp;
	if (pos == 1)
	{
		new_hall->next = halls;
		halls = new_hall;
	}
	else
	{
		for (int i = 1; i < pos - 1; i++)
		{
			halls_head_temp = halls_head_temp->next;
		}
		new_hall->next = halls_head_temp->next;
		halls_head_temp->next = new_hall;
	}

	//save read data
	new_hall->hall_id = hall_id;
	new_hall->capacity = capacity;
	for (int i = 0; i < NUMBER_OF_DAYS; i++)
	{
		new_hall->reserved[i] = 0;
		new_hall->profit[i] = 0;
	}
	new_hall->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
	if (new_hall->name == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}
	strcpy(new_hall->name, name);


	printf("hall is successfully added\n");
	return halls;
}

food* add_food(food* foods, char* name, char* food_id_str, char* price_str, char* capacity_str, char* hall_id_str, hall* halls, char* day_str, int if_admin, int number_of_commands)
{
	if (if_admin == USER)// Only logged in admin can add food
	{
		printf("permission denied\n");
		return foods;
	}
	else if (if_admin == LOGGED_OUT)
	{
		printf("access denied\n");
		return foods;
	}
	else if(number_of_commands != 7)//command must be valid
	{
		printf("invalid command\n");
		return foods;
	}

	food* foods_head_temp = foods;
	int food_id = atoi(food_id_str);
	int price = atoi(price_str);
	int capacity = atoi(capacity_str);
	int hall_id = atoi(hall_id_str);
	int day = atoi(day_str);

	//check if hall exists
	int if_exist = NOT_EXIST;
	while (halls != NULL)
	{
		if (hall_id == halls->hall_id)
		{
			if_exist = EXIST;
			break;
		}
		halls = halls->next;
	}
	if (if_exist == NOT_EXIST)
	{
		printf("wrong hall id\n");
		return foods;
	}

	//food id must be uniqe
	while(foods != NULL)
	{
		if (food_id == foods->food_id)
		{
			printf("food id is not unique\n");
			return foods_head_temp;
		}
		foods = foods->next;
	}
	foods = foods_head_temp;

	//allocation
	food* new_food;
	new_food = (food*)malloc(1 * sizeof(food));
	if (new_food == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}
	int pos = 1;
	//add a food node to the list in order of days
	while (foods != NULL)
	{
		if (foods->day < day)
		{
			pos++;
			foods = foods->next;
		}
		else
		{
			break;
		}
	}
	foods = foods_head_temp;
	if (pos == 1)
	{
		new_food->next = foods;
		foods = new_food;
	}
	else
	{
		for (int i = 1; i < pos - 1; i++)
		{
			foods_head_temp = foods_head_temp->next;
		}
		new_food->next = foods_head_temp->next;
		foods_head_temp->next = new_food;
	}

	//save read data
	new_food->food_id = food_id;
	new_food->capacity = capacity;
	new_food->price = price;
	new_food->hall_id = hall_id;
	new_food->day = day;
	new_food->profit = 0;
	new_food->reserved = 0;
	new_food->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
	if (new_food->name == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}
	strcpy(new_food->name, name);


	printf("food is successfully added\n");
	return foods;
}

coupon* add_coupon(coupon* coupons, char* coupon_id_str, char* percentage_str, char* capacity_str, int if_admin, int number_of_commands)
{
	if (if_admin == USER)// Only logged in admin can add coupon
	{
		printf("permission denied\n");
		return coupons;
	}
	else if ((if_admin == LOGGED_OUT))
	{
		printf("access denied\n");
		return coupons;
	}
	else if (number_of_commands != 4)//command must be valid
	{
		printf("invalid command\n");
		return coupons;
	}

	int coupon_id = atoi(coupon_id_str);
	int percentage = atoi(percentage_str);
	int capacity = atoi(capacity_str);
	coupon* coupons_head_temp = coupons;
	//coupon id must be unique
	while (coupons != NULL)
	{
		if (coupons->coupon_id == coupon_id)
		{
			printf("coupon id is not unique\n");
			return coupons_head_temp;
		}
		coupons = coupons->next;
	}
	coupons = coupons_head_temp;

	//allocation
	coupon* new_coupon;
	new_coupon = (coupon*)malloc(1 * sizeof(coupon));
	if (new_coupon == NULL)
	{
		printf("program crashed, please try again later\n");
		exit(EXIT_FAILURE);
	}
	new_coupon->next = coupons;
	coupons = new_coupon;

	//save read data
	new_coupon->coupon_id = coupon_id;
	new_coupon->percentage = percentage;
	new_coupon->capacity = capacity;
	new_coupon->reserved = 0;

	printf("coupon is successfully added\n");
	return coupons;
}

void free_all(user** users, hall** halls, food** foods, coupon** coupons, char** parsed_command,int number_of_commands)
{
	//free users 
	while ((*users) != NULL)
	{
		user* temp = (*users)->next;
		free((*users)->reserved_foods);//free reserved foods
		free((*users));
		(*users) = temp;
	}
	(*users) = NULL;
	//free halls
	while ((*halls) != NULL)
	{
		hall* temp = (*halls)->next;
		free((*halls));
		(*halls) = temp;
	}
	(*halls) = NULL;
	//free foods
	while ((*foods) != NULL)
	{
		food* temp = (*foods)->next;
		free((*foods));
		(*foods) = temp;
	}
	(*foods) = NULL;
	//free coupons
	while ((*coupons) != NULL)
	{
		coupon* temp = (*coupons)->next;
		free((*coupons));
		(*coupons) = temp;
	}
	(*coupons) = NULL;
	//free parsed command
	for (int i = 0; i < number_of_commands; i++)
	{
		free(parsed_command[i]);
	}
	free(parsed_command);
}

user* user_finder(user* users, char* username)
{	
	//find a user by its username and return it (user is logged in so surely exists)
	while (users != NULL)
	{
		if (strcmp(users->username, username) == 0)
		{
			return users;
		}
		users = users->next;
	}
}

void money_management(user* users, char* money_str, int if_admin, int number_of_commands, int money_command, char* logged_in_user)
{
	if ((if_admin == LOGGED_OUT) || (if_admin == ADMIN))//only users
	{
		printf("access denied\n");
		return;
	}
	user* current_user = user_finder(users, logged_in_user);
	//to get money
	if (money_command == GET_MONEY)
	{
		if (number_of_commands != 2)
		{
			printf("invalid command\n");
			return;
		}
		int money = atoi(money_str);
		current_user->money = current_user->money + money;
		printf("money added successfully\n");
	}
	//to show money
	else if (money_command == SHOW_MONEY)
	{
		if (number_of_commands != 1)
		{
			printf("invalid command\n");
			return;
		}
		printf("%d\n", current_user->money);
	}
	return;
}

void food_report(food* foods, int if_admin, int number_of_commands)
{
	if (if_admin == USER)// Only logged in admin can see report
	{
		printf("permission denied\n");
		return;
	}
	else if (if_admin == LOGGED_OUT)
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 1)
	{
		printf("invalid command\n");
		return;
	}

	//write on txt file
	FILE* foods_report = fopen(FOODS_FILE_ADDRESS, "w");

	if (foods == NULL)
	{
		fprintf(foods_report, "there's no food available\n");
	}
	else
	{
		while (foods != NULL)
		{
			fprintf(foods_report, "%d %d %d %d\n", foods->food_id, foods->capacity, foods->reserved, foods->profit);
			foods = foods->next;
		}
	}
	
	fclose(foods_report);
	printf("done\n");
	return;
}

hall* hall_finder(hall* halls, int hall_id)
{	
	//find a hall by its id and return it (surely exists)
	while (halls != NULL)
	{
		if (halls->hall_id == hall_id)
		{
			return halls;
		}
		halls = halls->next;
	}
}

void hall_report(hall* halls, int if_admin, int number_of_commands)
{
	if (if_admin == USER)// Only logged in admin can see report
	{
		printf("permission denied\n");
		return;
	}
	else if (if_admin == LOGGED_OUT)
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 1)
	{
		printf("invalid command\n");
		return;
	}

	FILE* halls_report = fopen(HALLS_FILE_ADDRESS, "w");
	hall* temp_head_halls = halls;
	if (halls == NULL)
	{
		fprintf(halls_report, "there's no hall\n");
	}
	else
	{	
		//hall's list is in order of days
		while (halls != NULL)
		{
			fprintf(halls_report, "%d %s %d\n", halls->hall_id, halls->name, halls->capacity);
			for (int i = 0; i < NUMBER_OF_DAYS; i++)
			{
				fprintf(halls_report, "%d %d %d\n", (i+1), halls->reserved[i], halls->profit[i]);
			}
			halls = halls->next;
		}
	}

	fclose(halls_report);
	printf("done\n");
	return;
}

void show_menu(food* foods, hall* halls, int if_admin, int number_of_commands)
{
	if ((if_admin == LOGGED_OUT) || (if_admin == ADMIN))//only users
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 1)
	{
		printf("invalid command\n");
		return;
	}

	for (int i = 1; i <= NUMBER_OF_DAYS; i++)
	{
		printf("%d:\n", i);
		if ((foods != NULL) && (foods->day == i))
		{	
			//show all foods in a specific day
			while (TRUE)
			{
				printf("%s %d %d %s %d\n", foods->name, foods->price, foods->food_id, (hall_finder(halls, foods->hall_id))->name, ((foods->capacity) - (foods->reserved)));
				if (foods->next == NULL || ((foods->next)->day != i))
				{
					foods = foods->next;
					break;
				}
				foods = foods->next;
			}
		}
	}
}

void reserve_food(food* foods, user* users, hall* halls, char* food_id_str, int if_admin, char* logged_in_user, int number_of_commands)
{
	if ((if_admin == LOGGED_OUT) || (if_admin == ADMIN))//only users
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 2)
	{
		printf("invalid command\n");
		return;
	}

	int food_id = atoi(food_id_str);
	//check if food id exists
	int if_exist = NOT_EXIST;
	while (foods != NULL)
	{
		if (food_id == foods->food_id)
		{
			if_exist = EXIST;
			break;
		}
		foods = foods->next;
	}
	if (if_exist == NOT_EXIST)
	{
		printf("not possible - wrong food id\n");
		return;
	}
	
	user* current_user = user_finder(users, logged_in_user);
	hall* current_hall = hall_finder(halls, foods->hall_id);
	if (foods->capacity > foods->reserved)//check food capacity
	{
		if (current_user->money >= foods->price)//check if user has enough money
		{
			if (current_hall->capacity > current_hall->reserved[foods->day - 1])//check hall capacity
			{
				//apply changes in variables
				(current_hall->reserved[foods->day - 1])++;
				(foods->reserved)++;
				current_user->money = current_user->money - foods->price;
				foods->profit = foods->profit + foods->price;
				(current_hall->profit[foods->day - 1]) = (current_hall->profit[foods->day - 1]) + foods->price;

				//add food to user's reserved foods in order of day
				reserved_food* reserve_foods_head_temp = current_user->reserved_foods;
				reserved_food* new_reserved_food = (reserved_food*)malloc(1 * sizeof(reserved_food));
				if (new_reserved_food == NULL)
				{
					printf("program crashed, please try again later\n");
					exit(EXIT_FAILURE);
				}
				int pos = 1;
				while (current_user->reserved_foods != NULL)
				{
					if ((current_user->reserved_foods)->day < foods->day)
					{
						pos++;
						current_user->reserved_foods = (current_user->reserved_foods)->next;
					}
					else
					{
						break;
					}
				}
				current_user->reserved_foods = reserve_foods_head_temp;
				if (pos == 1)
				{
					new_reserved_food->next = current_user->reserved_foods;
					current_user->reserved_foods = new_reserved_food;
				}
				else
				{
					for (int i = 1; i < pos - 1; i++)
					{
						reserve_foods_head_temp = reserve_foods_head_temp->next;
					}
					new_reserved_food->next = reserve_foods_head_temp->next;
					reserve_foods_head_temp->next = new_reserved_food;
				}
				new_reserved_food->day = foods->day;
				new_reserved_food->food_id = foods->food_id;
				new_reserved_food->spent_money = foods->price;

				printf("food is successfully reserved\n");
			}
			else
			{
				printf("not possible - hall is full\n");
			}
		}
		else
		{
			printf("not possible - not enough money\n");
		}
	}
	else
	{
		printf("not possible - food is full\n");
	}
}

void reserve_food_discount(food* foods, user* users, hall* halls, coupon* coupons, char* food_id_str, char* coupon_id_str, int if_admin, char* logged_in_user, int number_of_commands)
{
	if ((if_admin == LOGGED_OUT) || (if_admin == ADMIN))//only users
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 3)
	{
		printf("invalid command\n");
		return;
	}

	int food_id = atoi(food_id_str);
	int coupon_id = atoi(coupon_id_str);

	//check if food id exists
	int if_exist_food_id = NOT_EXIST;
	int if_exist_coupon_id = NOT_EXIST;
	while (foods != NULL)
	{
		if (food_id == foods->food_id)
		{
			if_exist_food_id = EXIST;
			break;
		}
		foods = foods->next;
	}
	//check if coupon id exists
	while (coupons != NULL)
	{
		if (coupon_id == coupons->coupon_id)
		{
			if_exist_coupon_id = EXIST;
			break;
		}
		coupons = coupons->next;
	}
	if (if_exist_food_id == NOT_EXIST)
	{
		printf("not possible - wrong food id\n");
		return;
	}
	if (if_exist_coupon_id == NOT_EXIST)
	{
		printf("not possible - wrong coupon id\n");
		return;
	}

	
	user* current_user = user_finder(users, logged_in_user);
	hall* current_hall = hall_finder(halls, foods->hall_id);
	int price = foods->price - ((int)((foods->price * coupons->percentage) / 100));
	if (foods->capacity > foods->reserved)//check food capacity
	{
		if (coupons->capacity > coupons->reserved)//check coupons capacity
		{
			if (current_user->money >= price)//check if user has enough money
			{
				if (current_hall->capacity > current_hall->reserved[foods->day - 1])//check hall capacity
				{
					(current_hall->reserved[foods->day - 1])++;
					(foods->reserved)++;
					(coupons->reserved)++;
					current_user->money = current_user->money - price;
					foods->profit = foods->profit + price;
					(current_hall->profit[foods->day - 1]) = (current_hall->profit[foods->day - 1]) + price;
					
					//add food to user's reserved foods in order of day
					reserved_food* reserve_foods_head_temp = current_user->reserved_foods;
					reserved_food* new_reserved_food = (reserved_food*)malloc(1 * sizeof(reserved_food));
					if (new_reserved_food == NULL)
					{
						printf("program crashed, please try again later\n");
						exit(EXIT_FAILURE);
					}
					int pos = 1;
					while (current_user->reserved_foods != NULL)
					{
						if ((current_user->reserved_foods)->day < foods->day)
						{
							pos++;
							current_user->reserved_foods = (current_user->reserved_foods)->next;
						}
						else
						{
							break;
						}
					}
					current_user->reserved_foods = reserve_foods_head_temp;
					if (pos == 1)
					{
						new_reserved_food->next = current_user->reserved_foods;
						current_user->reserved_foods = new_reserved_food;
					}
					else
					{
						for (int i = 1; i < pos - 1; i++)
						{
							reserve_foods_head_temp = reserve_foods_head_temp->next;
						}
						new_reserved_food->next = reserve_foods_head_temp->next;
						reserve_foods_head_temp->next = new_reserved_food;
					}
					new_reserved_food->day = foods->day;
					new_reserved_food->food_id = foods->food_id;
					new_reserved_food->spent_money = price;

					printf("food is successfully reserved\n");
				}
				else
				{
					printf("not possible - hall is full\n");
				}
			}
			else
			{
				printf("not possible - not enough money\n");
			}
		}
		else
		{
			printf("not possible - coupon is full\n");
		}
	}
	else
	{
		printf("not possible - food is full\n");
	}
}

food* food_finder(food* foods, int food_id)
{	
	//finds a food by its id and return it (surely exists)
	while (foods != NULL)
	{
		if (foods->food_id == food_id)
		{
			return foods;
		}
		foods = foods->next;
	}
}

void show_reserved(user* users, food* foods, hall* halls, char* logged_in_user, int if_admin, int number_of_commands)
{
	if ((if_admin == LOGGED_OUT) || (if_admin == ADMIN))//only users
	{
		printf("access denied\n");
		return;
	}
	else if (number_of_commands != 1)
	{
		printf("invalid command\n");
		return;
	}

	user* current_user = user_finder(users, logged_in_user);
	reserved_food* reserved_foods_head_temp = current_user->reserved_foods;
	for (int i = 1; i <= NUMBER_OF_DAYS; i++)
	{	
		//reserved foods are arranged in order of days so it prints everyday reserved foods
		printf("%d:\n", i);
		if (current_user->reserved_foods != NULL && (current_user->reserved_foods)->day == i)
		{
			while (TRUE)
			{
				printf("%s %d %d %s\n", (food_finder(foods, (current_user->reserved_foods)->food_id))->name, (current_user->reserved_foods)->spent_money, (current_user->reserved_foods)->food_id, (hall_finder(halls, (food_finder(foods, (current_user->reserved_foods)->food_id))->hall_id))->name);
				
				if ((current_user->reserved_foods)->next == NULL || (((current_user->reserved_foods)->next)->day != i))
				{
					(current_user->reserved_foods) = (current_user->reserved_foods)->next;
					break;
				}
				(current_user->reserved_foods) = (current_user->reserved_foods)->next;
			}
		}
	}
	current_user->reserved_foods = reserved_foods_head_temp;
}

int main()
{
	//users initialization and adding users to program
	user* users = NULL;
	users = read_and_save_from_file(users);

	//halls, foods and coupons initilization
	hall* halls = NULL;
	food* foods = NULL;
	coupon* coupons = NULL;

	//intialization a 2D char array for command line which is meant to be saved in array of strings
	char** parsed_command = NULL;
	int number_of_commands = 0;
	int command_chars = 1;
	char nextchar;

	//system logic
	int if_login = LOGGED_OUT;
	int if_admin = LOGGED_OUT;
	char* logged_in_user = NULL;
	
	while (TRUE)
	{
		//to get a command line with unknown length
		char* command_line = (char*)malloc(command_chars * sizeof(char));
		if (command_line == NULL)
		{
			printf("program crashed, please try again later\n");
			exit(EXIT_FAILURE);
		}
		while ((nextchar = getchar()) != '\n')
		{
			command_line[command_chars - 1] = nextchar;
			command_chars++;
			command_line = (char*)realloc(command_line, command_chars * sizeof(char));
		}
		command_line[command_chars - 1] = NULL;

		//parse command line to its words and save it
		parsed_command = get_and_parse_command_line(parsed_command, command_line, &number_of_commands);


		//login
		if ((strcmp((parsed_command[0]), "login")) == 0)
		{
			to_login(users, parsed_command, &if_login, number_of_commands, &if_admin, &logged_in_user);
		}

		//logout
		else if ((strcmp((parsed_command[0]), "logout")) == 0)
		{
			to_logout(&if_login, &if_admin, &logged_in_user);
		}

		//signup
		else if ((strcmp((parsed_command[0]), "signup")) == 0)
		{
			users = to_signup(users, parsed_command[1], parsed_command[3], parsed_command[2], if_login, number_of_commands);
		}

		//add hall
		else if ((strcmp((parsed_command[0]), "add_hall")) == 0)
		{
			halls = add_hall(halls, parsed_command[1], parsed_command[2], parsed_command[3], if_admin, number_of_commands);
		}

		//add food
		else if ((strcmp((parsed_command[0]), "add_food")) == 0)
		{
			foods = add_food(foods, parsed_command[1], parsed_command[2], parsed_command[3], parsed_command[4], parsed_command[5], halls, parsed_command[6], if_admin, number_of_commands);
		}

		//add coupon
		else if ((strcmp((parsed_command[0]), "add_coupon")) == 0)
		{
			coupons = add_coupon(coupons, parsed_command[1], parsed_command[2], parsed_command[3], if_admin, number_of_commands);
		}

		//get money
		else if ((strcmp((parsed_command[0]), "get_money")) == 0)
		{
			money_management(users, parsed_command[1], if_admin, number_of_commands, GET_MONEY, logged_in_user);
		}

		//show money
		else if ((strcmp((parsed_command[0]), "show_money")) == 0)
		{
			money_management(users, NULL, if_admin, number_of_commands, SHOW_MONEY, logged_in_user);
		}
		
		//food report
		else if ((strcmp((parsed_command[0]), "food_report")) == 0)
		{
			food_report(foods, if_admin, number_of_commands);
		}

		//hall report
		else if ((strcmp((parsed_command[0]), "hall_report")) == 0)
		{
			hall_report(halls, if_admin, number_of_commands);
		}

		//show menu
		else if ((strcmp((parsed_command[0]), "menu")) == 0)
		{
			show_menu(foods, halls, if_admin, number_of_commands);
		}

		//reserve food
		else if ((strcmp((parsed_command[0]), "reserve")) == 0)
		{
			reserve_food(foods, users, halls, parsed_command[1], if_admin, logged_in_user, number_of_commands);
		}

		//reserve food with discount
		else if ((strcmp((parsed_command[0]), "reserve_discount")) == 0)
		{
			reserve_food_discount(foods, users, halls, coupons, parsed_command[1], parsed_command[2], if_admin, logged_in_user, number_of_commands);
		}

		//show reserved foods
		else if ((strcmp((parsed_command[0]), "show_reserved")) == 0)
		{
			show_reserved(users, foods, halls, logged_in_user, if_admin, number_of_commands);
		}

		//free and exit
		else if ((strcmp((parsed_command[0]), "exit")) == 0)
		{
			free_all(&users, &halls, &foods, &coupons, parsed_command, number_of_commands);
			break;
		}

		// invalid commands
		else
		{
			if (if_login == LOGGED_OUT)
			{
				printf("access denied\n");
			}
			else if (if_login == LOGGED_IN)
			{
				printf("invalid command\n");
			}
		}

		//reset and continue
		for (int i = 0; i < number_of_commands; i++)
		{
			free(parsed_command[i]);
		}
		free(parsed_command);

		number_of_commands = 0;
		command_chars = 1;
	}
	
	printf("program has ended\n");
	return 0;
}
