#include<iostream>
#include<conio.h>   // provides functions like _getch() and and _kbhit(), allowing pressing of keyboard keys and returning them without pressing enter.
#include <windows.h>  // control the console window eg move cursor, change text color etc.
#include <time.h> // provide time functions e.g. srand((unsigned)time(NULL)); this time function along with srand helps genertae enemies on random time at different locations.
#include<fstream>  // allows file handling e.g saving high score of the game in a txt file.
#include <mmsystem.h>   // for playing sound
#pragma comment(lib, "winmm.lib") // to link the windows sound library
#define _CRT_SECURE_NO_WARNINGS
// Defined constants so we donot want to define them every time

const int width = 100;  //total width of the console game area=90 characters across
const int height = 26; //total height of the console game area = 26 rows down
const int playareawidth = 75;  //  width of the actual playable area e.g. where the spaceshooter and aliens move
const int sidemenuewidth = 25;   //width of sidepanel , where score is seen.


using namespace std;

// HANDLE console is like a remote control with which we can send commands like move cursor to aspecefic position, Change text color, 
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

// This helps to store x and y coordinates of cursor position so that we can move to a specefic loaction of the console window.
COORD cursor_position;

// arrays to store information about enemies
int enemy_y[5];  // stores vertical position of each enemy, at one time there can be max of 5 enemies on screen
int enemy_x[5];
int enemy_status[5]; // when it is true or 1 means enemy is alive and should be drawn and vice versa
float enemy_speed[5] = { 1, 1, 1, 1, 1 };

// array named shooter used to draw the shooter
char shooter[3][5] =
{
	{' ','^','^','^',' '},
	{'/','|','|','|','\\'},
	{' ','|','|','|',' '}
};

//initial position of shooter eg in the middle of the console window
int shooter_position = playareawidth / 2;

int score = 0;
int lives = 3;
int level = 1;
int level_goal = 10;
int enemycount = 2;
int highscore = 0;
int bullets[20][2];
int enemyXbullet[20];
int enemyYbullet[20];
bool enemybullet_status[20];
float enemybullet_speed[20];
int bullet_index = 0;   //counter that keeps track of which bullet slot/index to use next in the  bullets[20][2] array.
int shieldX, shieldY;
bool shield_status = false;
bool shield_taken = false;

// function to set color in console
void change_color(int color) {

	SetConsoleTextAttribute(console, color);

}

// moves cursor position before printing
void move_to_coordinate(int x, int y) {

	cursor_position.X = x;
	cursor_position.Y = y;

	SetConsoleCursorPosition(console, cursor_position);
}


void setcursor(bool visible, DWORD size) {
	if (size == 0)
	{

		size = 20;
	}

	CONSOLE_CURSOR_INFO lpCursor;
	lpCursor.bVisible = visible;
	lpCursor.dwSize = size;
	SetConsoleCursorInfo(console, &lpCursor);
}

void border() {

	for (int i = 0; i < width; i++) {

		move_to_coordinate(i, height); cout << "±";  // prints the bottom horizontal border as screen height sets the y position to the max column of console
	}

	for (int i = 0; i < height; i++) {

		move_to_coordinate(0, i); cout << "±";               // prints the left vertical border
		move_to_coordinate(width, i); cout << "±";    // prints the right vertical border eg screen-width as row and i(which iterates) as column
	}
	for (int i = 0; i < height; i++) {

		move_to_coordinate(playareawidth, i); cout << "±";        // prints vertcial line to saperate play area from menue
	}
}

// erases enemy from screen by printing spaces over enemies position      
void remove_enemy(int ind) {
	move_to_coordinate(enemy_x[ind], enemy_y[ind]);
	cout << "    ";                                       // as enemy is made of 4 rows,  print space on first row
	move_to_coordinate(enemy_x[ind], enemy_y[ind] + 1);
	cout << "    ";                                       // prints spaces on second
	move_to_coordinate(enemy_x[ind], enemy_y[ind] + 2);
	cout << "    ";                                       // on third
	move_to_coordinate(enemy_x[ind], enemy_y[ind] + 3);
	cout << "    ";                                       // on fourth
}

// function to decides where the enemy will appear, starting from top
void enemy_position(int ind) {

	enemy_x[ind] = 3 + rand() % (playareawidth - 10);  // +3 added to avoid enemies coming from very left // rand()%10 gives a random number between 0 to 9// in this case as width is 70, we minus 10 from it to avoid getting to to the right side. so rand() % (70 - 10) gives random number between 0 and 59, them 3 is added to give final position.
	enemy_y[ind] = 1;                              // initially set to one so generated at top
	enemy_status[ind] = 1;                          // to mark the enemy as active 

}


// function to draw enemy/alien using the coordinates from genEnemy, allowing its random nature.
void create_enemy(int ind) {

	if (enemy_status[ind] == true)
	{
		change_color(12);
		move_to_coordinate(enemy_x[ind], enemy_y[ind]);
		cout << " __ ";
		move_to_coordinate(enemy_x[ind], enemy_y[ind] + 1);
		cout << "/  \\";
		move_to_coordinate(enemy_x[ind], enemy_y[ind] + 2);
		cout << "\\__/";
		move_to_coordinate(enemy_x[ind], enemy_y[ind] + 3);
		cout << " || ";
		change_color(7);  // reset the text color back to white
	}
}

// is called when enemy is destroyed or goes down the screen, it then erases that enemy and generates new one on  a random position
void enemy_refresh(int ind) {

	remove_enemy(ind);
	enemy_position(ind);
	enemy_status[ind] = 1;
}

// creates a new bullet in bullet array when the user presses SPACE 
void bullet_coordinate() {

	bullets[bullet_index][0] = 21;    // y coordinate of bullet set to 21, so it starts from bottom
	bullets[bullet_index][1] = shooter_position + 2;  // x coordinate of bullet set to in the middle of the shooter
	bullet_index++;                           // after creating a bullet, move to next array index to make another bullet
	if (bullet_index == 20)
	{
		bullet_index = 0;     // if all 20 slots are full, then start from first bullet array index
	}

	Beep(750, 100);
}


// moves the bullet up by decreasing its y coordinate/row 
void move_bullet() {

	for (int i = 0; i < 20; i++) {   // as bullet moves up, its y coordinate/row decreases
		if (bullets[i][0] > 2)   // if bullet y coordinate is greater than 2, it means it has not reached the top and is ACTIVE 
		{
			bullets[i][0]--;       // as far as it is active, keep decrementing its y coordinate, to keep moving it up the console window.
		}
		else {
			bullets[i][0] = 0;       // if reached the top, make it inactive
		}
	}
}

// function to draw active bullets from the bullet array
void create_bullet() {

	for (int i = 0; i < 20; i++) {    // loops through all 20 array index

		int color_array[5] = { 10, 11, 12, 13, 14 };
		int color = color_array[i % 5];
		change_color(color);
		if (bullets[i][0] > 1) {     // if the row/y position of bullet is greater than 1, it means that bullet is still on screen and is active

			move_to_coordinate(bullets[i][1], bullets[i][0]);
			cout << "|";                                    // drwas only the active bullets

		}
	}
	change_color(7); //resetting colour to white

}


// function to erase old position of all active bullets, once erased moveBullet() will update its y coordinate and then drawBullets() will redraw at new location.
void erasebullet_oldposition() {
	for (int i = 0; i < 20; i++) {

		if (bullets[i][0] >= 1) {
			move_to_coordinate(bullets[i][1], bullets[i][0]); cout << " ";
		}

	}
}

// called when bullet hits enemy so that specefic bullet is erased
void remove_collided_bullet(int i) {

	move_to_coordinate(bullets[i][1], bullets[i][0]); cout << " ";
	bullets[i][0] = 0;
	bullets[i][1] = 0;

}

void explosion_effect(int x, int y) {

	for (int i = 0; i < 4; i++) {

		int color_value = 12 + i;
		change_color(color_value);

		move_to_coordinate(x + 1, y);
		cout << "*";

		Sleep(50);
	}

	change_color(7);

	move_to_coordinate(x, y);
	cout << " ";
}


int star_x[18];
int star_y[18];

void background_stars() {

	for (int i = 0; i < 18; i++) {

		int randomx = rand() % (playareawidth - 2);
		star_x[i] = randomx;

		int randomy = rand() % height;
		star_y[i] = randomy;
	}
}

void draw_stars() {


	change_color(8);  //grey color

	for (int i = 0; i < 18; i++) {

		int currentX = star_x[i];
		int currentY = star_y[i];

		if (currentY >= 22 && currentY <= 24 &&
			currentX >= shooter_position && currentX <= shooter_position + 4) {
			continue;
		}
		move_to_coordinate(currentX, currentY);
		cout << ".";
	}

	// reset color back to white
	change_color(7);
}

void erase_stars() {
	for (int i = 0; i < 18; i++) {

		if (star_y[i] >= 22 && star_y[i] <= 24 &&
			star_x[i] >= shooter_position && star_x[i] <= shooter_position + 4) {
			continue;
		}

		move_to_coordinate(star_x[i], star_y[i]);
		cout << " ";

	}
}

void move_stars() {

	for (int i = 0; i < 18; i++) {


		star_y[i] = star_y[i] + 1;


		if (star_y[i] >= height) {


			star_y[i] = 1;


			int random_position = rand() % (playareawidth - 2);  // re assigning random position
			star_x[i] = random_position;
		}
	}
}



void draw_shooter() {
	change_color(11);  // setting color to blue

	for (int i = 0; i < 3; i++) {

		for (int j = 0; j < 5; j++) {
			move_to_coordinate(j + shooter_position, i + 22); cout << shooter[i][j];  // console height is 25, se atrt from 22 to print shooters head and then till 25
		}

	}
	change_color(7);  // resetting colour
}

// erase shooter from old position in order to make it move
void eraseshooter_oldposition() {

	for (int i = 0; i < 3; i++) {

		for (int j = 0; j < 5; j++) {
			move_to_coordinate(j + shooter_position, i + 22); cout << " ";
		}

	}
}

// function to return the enemy's index which is in collision. 
// collision is detected when both x and y coordinated overlap of enemy and shooter
int collision() {

	for (int i = 0; i < enemycount; i++) {

		if (enemy_y[i] + 3 >= 22) {   // check that has the enemy's bottom reached row 22 (potential y posotion to touvh the shoter)

			// checks if enemy's right side is equal to or greater than shooter's left side AND enemy’s left side is at or less than the ship’s right side, then enemy's horizontally on the ship
			if (enemy_x[i] + 3 >= shooter_position && enemy_x[i] <= shooter_position + 4) {
				return i;
			}

		}
	}
	return -1;
}

int bullet_collision() {

	for (int i = 0; i < 20; i++) {
		if (bullets[i][0] != 0) {  // check for active bullets 
			for (int j = 0; j < enemycount; j++) {   // checks the bullet hitting for all enemies
				if (bullets[i][0] >= enemy_y[j] && bullets[i][0] <= enemy_y[j] + 3) {   // checks if bullet y positon overlaps with enemy
					if (bullets[i][1] >= enemy_x[j] && bullets[i][1] <= enemy_x[j] + 3) { // // checks if bullet x positon overlaps with enemy
						remove_collided_bullet(i);
						explosion_effect(enemy_x[j], enemy_y[j]);
						enemy_refresh(j);
						Beep(1000, 150);
						return 1;
					}
				}
			}
		}
	}
	return 0;   // return 0 if bullet doesnt hit enemy
}

void end_game() {

	PlaySound(NULL, 0, 0); // stops background music

	PlaySound(TEXT("gameover.wav"), NULL, SND_SYNC); // play game over sound once


	system("cls");

	move_to_coordinate(30, 5);
	change_color(12); // red
	cout << "*********************************";

	move_to_coordinate(30, 6);
	cout << "          GAME IS OVER!         ";

	move_to_coordinate(30, 7);
	cout << "*********************************";


	change_color(3);
	move_to_coordinate(32, 10);
	cout << "Final Score   : " << score;

	move_to_coordinate(32, 11);
	cout << "High Score    : " << highscore;

	move_to_coordinate(32, 12);
	cout << "Lives Left    : " << lives;

	move_to_coordinate(32, 13);
	cout << "Level Reached : " << level;

	change_color(15);
	move_to_coordinate(30, 16);
	cout << "Press R to Restart game or Q to Quit";
	change_color(7);
	char input;
	while (true) {

		input = _getch();   // waits for key press
		if (input == 'r' || input == 'R') {
			return;         // restarts game
		}

		else if (input == 'q' || input == 'Q') {
			exit(0);        // quit game
		}


	}
}

// prints new score in the sde menue whenever the bullet hits the enemy.
void update_score() {

	change_color(10);
	move_to_coordinate(playareawidth + 7, 5); cout << "Score: " << score;
	change_color(7);

}

int sum_target_savedlevel(int L) {

	int num = 0;
	for (int i = 1; i <= L; i++) {

		num = num + (i * 10);

	}
	return num;

}


void save_game() {

	ofstream f("save.txt");

	if (!f) {
		return;
	}

	else
	{
		f << score << " " << lives << " " << level << " " << shooter_position << " " << enemycount << endl;

		for (int i = 0; i < enemycount; i++) {
			f << enemy_status[i] << " " << enemy_x[i] << " " << enemy_y[i] << endl;
		}

		f.close();
	}
}


bool load_game() {

	ifstream f("save.txt");

	if (!f) {
		return false;

	}

	else {
		f >> score >> lives >> level >> shooter_position >> enemycount;

		if (enemycount < 1) {
			enemycount = 2;
		}

		for (int i = 0; i < enemycount; i++) {
			if (!(f >> enemy_status[i] >> enemy_x[i] >> enemy_y[i])) {
				enemy_position(i);
			}
			else {
				if (enemy_y[i] > 3 || enemy_y[i] <= 0) {
					enemy_position(i);
				}
				else {
					enemy_status[i] = 1;
				}
			}
		}

		f.close();
		return true;
	}
}

void display_instructions() {

	system("cls");
	change_color(11);
	cout << "************ INSTRUCTIONS ************" << endl << endl;
	change_color(15);
	cout << "OBJECTIVE:" << endl;
	change_color(3);
	cout << "  1. Shoot enemies and reach level 5 ! " << endl;
	cout << "  2. Shoot enemies to survive and score points" << endl;
	cout << "  3. Each level ahs its own level target, reach that target to pass on to next level." << endl << endl;

	change_color(15);
	cout << "CONTROLS:" << endl;
	change_color(3);
	cout << "  A  - Move Left" << endl;
	cout << "  D  - Move Right" << endl;
	cout << "  SPACE - Shoot Bullet" << endl;
	cout << "  P  - Pause / Resume Game" << endl;
	cout << "  S  - Save Game" << endl << endl;
	cout << "  ESC - Quit to Main Menu" << endl << endl;

	change_color(15);
	cout << "LIVES SYSTEM:" << endl;
	change_color(3);
	cout << "  1. You start with 3 lives." << endl;
	cout << "  2. Collision with enemy or enemy bullet reduces a life." << endl << endl;
	cout << "  3. If you get a shield, you protect your one life if get hit." << endl << endl;

	change_color(15);
	cout << "LEVELS:" << endl;
	change_color(3);
	cout << "  1. Score goals increase each level." << endl;
	cout << "  2. Enemies move faster on higher levels." << endl;
	cout << "  3. Enemy bullets appear starting Level 2." << endl;
	cout << "  4. Shield Power-up appears from Level 2." << endl << endl;

	change_color(15);
	cout << "HIGH SCORE SYSTEM:" << endl;
	change_color(3);
	cout << "  1. Your highest score is saved in file named as highscore.txt." << endl;
	cout << "  2. High score updates automatically when beaten." << endl << endl;

	change_color(2);
	cout << "Press any key to return to menu...";

	change_color(7);
	_getche();   // ends the function and program returns to the point from wehre it was called e.g main()
}

// border may get erased by over writing like enemy generation or my erase functions, so this function ensures left borderis always visible

void protect_leftborder() {

	for (int i = 0; i < height; i++) {

		move_to_coordinate(0, i);
		cout << "±";

	}
}

void run_game(bool fresh = true) {

	// for background
	background_stars();

	// clearing enemies and bullets from a prevous run to avoid bugs or error when starting
	for (int i = 0; i < 5; ++i) {

		enemy_x[i] = 0;
		enemy_y[i] = 0;
		enemy_status[i] = 0;

	}

	for (int i = 0; i < 20; ++i) {

		enemybullet_status[i] = false;
		enemyXbullet[i] = 0;
		enemyYbullet[i] = 0;

	}

	// to decide if starting a fresh game or continuing a saved game.
	if (fresh) {

		shooter_position = playareawidth / 2;
		score = 0;
		lives = 3;
		level = 1;
		level_goal = 10;
		enemycount = 2;
		for (int i = 0; i < enemycount; i++) {

			enemy_position(i);

		}

	}
	else {
		if (enemycount < 2) {
			enemycount = 2;
		}

		for (int i = 0; i < enemycount; i++) {
			enemy_position(i);
		}
	}


	// clearing all player bullets and resetting  bullet index.
	for (int i = 0; i < 20; i++) {
		bullets[i][0] = 0;
		bullets[i][1] = 0;
	}
	bullet_index = 0;

	// initially enemy bullets are inactive and slow
	for (int i = 0; i < 20; i++) {

		enemybullet_status[i] = false;
		enemybullet_speed[i] = 1.0f;
	}

	system("cls");
	border();

	PlaySound(TEXT("freemusic.wav"), NULL, SND_LOOP | SND_ASYNC);

	update_score();

	change_color(14);


	move_to_coordinate(playareawidth + 7, 7);
	cout << "Lives: " << lives;

	move_to_coordinate(playareawidth + 7, 9);
	cout << "Level: " << level;

	move_to_coordinate(playareawidth + 7, 8);
	cout << "HighScore: " << highscore;

	move_to_coordinate(playareawidth + 7, 10);

	if (shield_taken) {
		cout << "Shield: ON ";
	}
	else {
		cout << "Shield: OFF";
	}

	move_to_coordinate(playareawidth + 5, 2);
	cout << "Space Shooter";

	move_to_coordinate(playareawidth + 7, 4);
	cout << "----------";

	move_to_coordinate(playareawidth + 7, 6);
	cout << "----------";

	move_to_coordinate(playareawidth + 7, 12);
	cout << "Control ";

	move_to_coordinate(playareawidth + 7, 13);
	cout << "-------- ";

	move_to_coordinate(playareawidth + 2, 14);
	cout << " A Key - Left";

	move_to_coordinate(playareawidth + 2, 15);
	cout << " D Key - Right";

	move_to_coordinate(playareawidth + 2, 16);
	cout << " Spacebar = Shoot";

	change_color(11);
	move_to_coordinate(10, 5);
	cout << "Press any key to start";

	change_color(7);

	_getch();
	move_to_coordinate(10, 5); cout << "                      ";   // to clear the “Press any key to start” message from screen
	draw_shooter();
	while (1)
	{
		erase_stars();
		move_stars();
		draw_stars();
		protect_leftborder();

		if (_kbhit()) {       // checks if a key is pressed

			char ch = _getch();

			if (ch == 'a' || ch == 'A' || ch == 75) {
				if (shooter_position > 3) {  // if bird is already on extreme left, then donot allow further left movement and a margain of 3 is kept from border
					eraseshooter_oldposition();
					shooter_position -= 4;
					draw_shooter();
				}
			}

			if (ch == 'd' || ch == 'D' || ch == 77) {
				if (shooter_position <= playareawidth - 9) {
					eraseshooter_oldposition();
					shooter_position += 4;
					draw_shooter();
				}
			}

			if (ch == 's' || ch == 'S') {
				save_game();
			}

			// when press p for first time, the game enters a loop that freezes everything and waits for another P, the main game loop (while (1)) is temporarily stopped. when p enter again, the loop breaks and game resumes.
			if (ch == 'p' || ch == 'P') {
				move_to_coordinate(30, 10); cout << "GAME PAUSED - Press P to Resume";

				while (true) {

					if (_kbhit()) {
						char t = _getch();
						if (t == 'p' || t == 'P') break;

					}
				}
				move_to_coordinate(30, 10); cout << "                                 ";
			}

			if (ch == 32) {  // ASCII code for spacebar
				bullet_coordinate();
			}

			if (ch == 27) {   // ASCII code for esc  
				break;
			}

		}


		

		for (int i = 0; i < enemycount; i++) {
			create_enemy(i);
		}

		create_bullet();

		if (level >= 2 && !shield_status && !shield_taken && rand() % 150 == 0) {
			shield_status = true;
			shieldX = rand() % (playareawidth - 2);
			shieldY = 1;

		}


		if (level >= 2) {

			for (int i = 0; i < enemycount; i++) {

				if (enemy_status[i] == 1 && enemy_y[i] < height - 5 && !enemybullet_status[i] && rand() % 20 == 0) {
					enemybullet_status[i] = true;
					if (enemy_x[i] + 1 > 1) {
						enemyXbullet[i] = enemy_x[i] + 1;
					}
					else {
						enemyXbullet[i] = 1;
					}
					enemyYbullet[i] = enemy_y[i] + 4;
					enemybullet_speed[i] = 1.0f + (level - 1) * 0.10f;
				}

			}
		}

		for (int i = 0; i < enemycount; i++) {
			if (enemybullet_status[i]) {
				move_to_coordinate(enemyXbullet[i], enemyYbullet[i]);
				cout << " ";
				float by = enemyYbullet[i];
				by += enemybullet_speed[i];
				enemyYbullet[i] = (int)by;
				if (enemyYbullet[i] >= 22 &&
					enemyXbullet[i] >= shooter_position &&
					enemyXbullet[i] <= shooter_position + 4)
				{
					if (shield_taken)
						shield_taken = false;
					else
						lives--;
					Beep(400, 300); // beep sound

					move_to_coordinate(playareawidth + 7, 10);
					if (shield_taken) {
						cout << "Shield: ON ";
					}
					else {
						cout << "Shield: OFF";
					}
					enemybullet_status[i] = false;
					move_to_coordinate(playareawidth + 7, 7); cout << "Lives: " << lives;
					if (lives == 0) {
						end_game();
						return;

					}
					continue;
				}
				if (enemyYbullet[i] >= height) {
					enemybullet_status[i] = false;
					continue;
				}

				change_color(13);
				move_to_coordinate(enemyXbullet[i], enemyYbullet[i]); cout << "|";
				change_color(7);

			}
		}

		if (shield_status) {
			move_to_coordinate(shieldX, shieldY); cout << " ";
			shieldY++;
			if (shieldY >= height) {
				shield_status = false;
			}
			else {
				change_color(10);
				move_to_coordinate(shieldX, shieldY); cout << "S";
				change_color(7);
			}
			if (shieldY == 22 && shieldX >= shooter_position && shieldX <= shooter_position + 4) {

				shield_taken = true;
				shield_status = false;

				Beep(500, 120);
				Beep(700, 120);
				Beep(600, 150);

				move_to_coordinate(playareawidth + 7, 10);
				cout << "Shield: ON  ";

			}
		}

		int hit = collision();
		if (hit != -1) {
			if (shield_taken) {
				shield_taken = false;
			}
			else {

				lives--;

				Beep(400, 300); // beep sound

			}

			move_to_coordinate(playareawidth + 7, 10);
			if (shield_taken == true) {
				cout << "Shield: ON ";
			}
			else if (shield_taken == false) {
				cout << "Shield: OFF";
			}
			move_to_coordinate(playareawidth + 7, 7); cout << "Lives: " << lives;
			if (lives == 0) {
				end_game();
				return;
			}
			enemy_refresh(hit);
		}

		if (bullet_collision() == 1) {
			score++;
			update_score();
			if (score > highscore) {
				highscore = score;
				ofstream fout("highscore.txt");
				if (fout) {
					fout << highscore;
					fout.close();
				}
				move_to_coordinate(playareawidth + 7, 8);
				cout << "HighScore: " << highscore;
			}
			if (score >= level_goal)
			{
				level++;

				Beep(700, 120);
				Beep(900, 120);
				Beep(1100, 150);
				Beep(900, 120);
				Beep(1200, 200);

				move_to_coordinate(playareawidth + 7, 9);
				cout << "Level: " << level;

				for (int k = 0; k < enemycount; k++) {
					remove_enemy(k);
					enemy_status[k] = 0;
				}
				for (int b = 0; b < 20; b++) {
					remove_collided_bullet(b);
				}
				if (level > 5)
				{
					system("cls");

					change_color(10);
					move_to_coordinate(30, 5);
					cout << "******************************************";

					move_to_coordinate(30, 6);
					cout << "              CONGRATULATIONS!             ";
					move_to_coordinate(30, 7);
					cout << "******************************************";

					move_to_coordinate(28, 9);
					cout << "   VICTORY !!! YOU HAVE SUCCESSFULLY COMPLETED ALL 5 LEVELS!   ";

					change_color(15);
					move_to_coordinate(32, 11);
					cout << "Press R to Restart or Q to Quit";
					change_color(7);

					char c = _getch();
					if (c == 'r' || c == 'R')
						return;
					else
						exit(0);
				}

				level_goal += level * 10;
				enemycount++;
				if (enemycount > 5)
				{
					enemycount = 5;
				}
				for (int i = 0; i < enemycount; i++) {
					enemy_y[i] = 4;
					enemy_position(i);
					enemy_status[i] = 1;
				}
				for (int i = 0; i < enemycount; i++) {
					enemy_speed[i] = 1.0f + (level - 1) * 0.2f;
				}
				for (int i = enemycount; i < 20; ++i) {
					enemybullet_status[i] = false;
					enemyXbullet[i] = 0;
					enemyYbullet[i] = 0;
				}
			}
		}
		
		int delay = 150 - (level * 15);
		if (delay < 40) delay = 40;
		Sleep(delay);
		
		erasebullet_oldposition();
		move_bullet();
		for (int i = 0; i < enemycount; i++)
		{
			if (enemy_status[i] == 1)
			{
				float newY = enemy_y[i];
				newY += enemy_speed[i];
				if ((int)newY != enemy_y[i]) {   // checks if the enemy has gone one row down or not
					remove_enemy(i);          
					enemy_y[i] = (int)newY;   
					create_enemy(i);          
				}
			}
			if (enemy_y[i] > height - 5)
			{
				remove_enemy(i);
				enemy_refresh(i);
				continue;
			}
		}
	}
}

int main()
{


	setcursor(0, 0);   // hides blinking cursor
	srand((unsigned)time(NULL));  // ensures every time game is run, enemies , stars and shield appear form random positions.



	ifstream fin("highscore.txt");
	if (fin)
		fin >> highscore;
	fin.close();
	do {
		system("cls");


		change_color(11);
		move_to_coordinate(30, 5);
		cout << "*********************************";

		move_to_coordinate(30, 6);
		cout << "          SPACE SHOOTER          ";

		move_to_coordinate(30, 7);
		cout << "*********************************";



		change_color(14);
		move_to_coordinate(32, 10);
		cout << "1. Start New Game";

		change_color(10);
		move_to_coordinate(32, 11);
		cout << "2. Load Saved Game";

		change_color(5);
		move_to_coordinate(32, 12);
		cout << "3. Instructions";

		change_color(12);
		move_to_coordinate(32, 13);
		cout << "4. Quit";


		change_color(15);
		move_to_coordinate(30, 15);
		cout << "Select option ( 1-4 ): ";

		change_color(7);


		char menu_option = _getch();
		if (menu_option == '1') {
			run_game(true);
		}
		else if (menu_option == '2') {
			if (load_game()) {
				level_goal = sum_target_savedlevel(level);
				run_game(false);
			}
			else {
				system("cls");
				cout << "No save file found.\n";
				_getche();
			}
		}
		else if (menu_option == '3') {
			display_instructions();
		}
		else if (menu_option == '4') {
			exit(0);
		}

	} while (1);
	return 0;
}
