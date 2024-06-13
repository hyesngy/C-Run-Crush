#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <mmsystem.h>
#include <process.h>
#pragma comment(lib,"winmm.lib")

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define BLANK ' ' // ' ' 로하면 흔적이 지워진다 
#define ESC 0x1b //  ESC 누르면 종료

#define SPACE ' '
#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d
#define SHOOT 0x4d

#define WIDTH 130
#define HEIGHT 35

#define GOLD	"ⓒ"
#define POTION  "@"
#define ATTACK "↗"

int map[HEIGHT][WIDTH] = { 0 };
int items[WIDTH][HEIGHT] = { 0 }; // 1이면 Gold 2면 Potion
int items_count = 0;
int iteminterval = 20; // ITEM 표시 간격

int Delay = 40; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int cnt = 0;

int score = 0;
int player_hp = 0;	
int monster_hp = 0;
int player_power = 1;	// 공격력

int player_x = 2;	
int player_y = 27;
int is_jumping = 0;
int jump_height = 0;
int is_monster_hit = 0; // 몬스터가 총알에 맞았는지 여부를 확인하는 변수
int is_player_hit = 0;	// 플레이어가 장애물에 맞았는지 확인하는 변수
int kill_count = 0;	// 몬스터 kill 횟수

#define MAXBULLET	100
int no_bullet = 0;
int bullet_front = 0; // Circular Queue를 이용하기 위함
int bullet_tail = 0;
COORD bullet;
COORD bullets[MAXBULLET] = { 0 };
int bullet_max_count = 5; // 연사 가능한 최대 총알 수

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos; // Windows.h 에 정의
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}
void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void showCursor(void) { // 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void cls(int text_color, int bg_color) // 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

void show_bullet_count()
{
	gotoxy(40, 1);
	printf("%01dshot", bullet_max_count - no_bullet);
}
void new_bullet(int x, int y) {
	if (no_bullet == bullet_max_count) // 연사 가능 수 제한
		return;
	bullets[bullet_tail].X = x;
	bullets[bullet_tail].Y = y;
	gotoxy(bullets[bullet_tail].X, bullets[bullet_tail].Y);
	printf("*");
	bullet_tail++;
	bullet_tail %= MAXBULLET; // Circular Queue 사용
	no_bullet++;
	show_bullet_count();
}
void move_bullet() {
	int i, bi;
	if (no_bullet == 0)
		return;
	for (i = bullet_front; i < bullet_front + no_bullet; i++) {
		bi = i % MAXBULLET; // Circular Queue 사용
		gotoxy(bullets[bi].X, bullets[bi].Y);
		printf(" ");

		if (bullets[bi].X < 81) {
			bullets[bi].X++;	// 기존의 코드에서 총알 방향(위->오른쪽)변경
			gotoxy(bullets[bi].X, bullets[bi].Y);
			printf("*");
		}
		else {
			if (bullets[bi].X == 81) {
				monster_hp -= player_power;
				is_monster_hit = 1;
				if (monster_hp < 0)
					monster_hp = 0;
			}
			bullet_front++; // 제일 앞에 있는 총알이 제일 위에 먼저 도착하므로 i번을 지운다.
			bullet_front %= MAXBULLET;
			no_bullet--; // 결국 bullet_front + no_bullet은 변하지 않는다.
		}
	}
	show_bullet_count();
}
void draw_box(int x1, int y1, int x2, int y2, char ch)
{
	int x, y;

	for (x = x1; x <= x2; x++) {
		gotoxy(x, y1);
		putchar(ch);
		gotoxy(x, y2);
		putchar(ch);
	}

	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		putchar(ch);
		gotoxy(x2, y);
		putchar(ch);
	}
}
void draw_box2(int x1, int y1, int x2, int y2, char* ch)
{
	int x, y;

	for (x = x1; x <= x2; x += 2) {
		gotoxy(x, y1);
		printf("─");
		gotoxy(x, y2);
		printf("─");
	}

	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("│");
		gotoxy(x2, y);
		printf("│");
	}

	gotoxy(x1, y1);
	printf("┌");
	gotoxy(x2, y1);
	printf("┐");
	gotoxy(x1, y2);
	printf("└");
	gotoxy(x2, y2);
	printf("┘");
}

int block_x = 80;
int block_y = 32;
int block_height = 5;
int block_speed = 1;

int fire_x = 78;
int fire_y = 20;
int fire_width = 1;
int fire_speed = 1;

void set_block_height() {
	block_height = rand() % 3 + 4;	// 블럭 높이 4~6 랜덤설정
}
void init_block()
{
	for (int i = 0; i < block_height; i++) {
		map[block_y - i][block_x] = 1;
	}
	//map[block_y][block_x] = 1;
	//map[block_y + 1][block_x] = 1;
	//map[block_y + 2][block_x] = 1;
	//map[block_y + 3][block_x] = 1;
	//map[block_y + 4][block_x] = 1;
}
void show_block()
{
	int x, y;
	for (x = 0; x < 80 - 1; x++) {
		for (y = 26; y < HEIGHT; y++) {
			if (map[y][x] == 1) {
				gotoxy(x, y);
				printf("■"); // ㅁ 한자.
			}
		}
	}
}
void erase_block()
{
	//int x, y;
	//for (x = 0; x < 80 - 1; x++) {
	//	for (y = 26; y < HEIGHT; y++) {
	//		if (map[y][x] == 1) {
	//			gotoxy(x, y);
	//			printf(" ");
	//		}
	//	}
	//}
	//map[block_y][block_x] = 0;
	//map[block_y + 1][block_x] = 0;
	//map[block_y + 2][block_x] = 0;
	//map[block_y + 3][block_x] = 0;
	//map[block_y + 4][block_x] = 0;

	for (int i = 0; i < block_height; i++) {
		map[block_y - i][block_x] = 0;
		gotoxy(block_x, block_y - i);
		printf(" ");	//블럭 지우기
	}

}
void move_block(dx, dy)
{
	int r;
	erase_block();
	block_x += dx;

	if (block_x <= 0) {
		block_x = 80 - 2;
		set_block_height();
		r = rand() % 100;

		if (r < 40) block_speed = 1;	// 40% 속도 1
		else if (r < 80)block_speed = 2;	// 40% 속도 2
		else block_speed = 3; // 20% 속도 3
	}
	//block_y += dy;
	init_block();
	show_block();
}

void init_fire() {
	for (int i = 0; i < fire_width; i++) {
		map[fire_y][fire_x - i] = 1; // fire_width 만큼
	}
}
void show_fire()
{
	textcolor(RED1, 3);
	int x;
	for (x = 0; x < 90; x++) {
		if (map[fire_y][x] == 1) {
			gotoxy(x, fire_y);
			printf("*");
			
		}
	}
	textcolor(WHITE, 3);
}
void erase_fire()
{
	int x;
	for (x = 0; x < 90; x++) {
		if (map[fire_y][x] == 1) {
			gotoxy(x, fire_y);
			printf(" ");
			map[fire_y][x] = 0;
		}
	}
	
}
void move_fire(dx, dy)
{
	int r;
	erase_fire();
	fire_x += dx;
	if (fire_x - fire_width <= 0) {
		fire_x = 78;
		r = rand() % 100;
		if (r < 50) fire_speed = 1;	// 50% 속도 1
		else if (r < 80) fire_speed = 2;	// 30% 속도 2
		else fire_speed = 3; // 20% 속도 3
		fire_y = rand() % 5 + 15; // 15~19
		fire_width = rand() % 5 + 1;	// 1~5
	}
	//fire_y += dy;
	init_fire();
	show_fire();
}

void draw_hline(int y, int x1, int x2, char ch)	//가로줄
{
	gotoxy(x1, y);
	for (; x1 <= x2; x1++)
		putchar(ch);
}

void draw_title() {
	cls(WHITE, 3);
	draw_box2(0, 0, 128, 34, "─"); // 화면에 box를 그린다.

	//초기화면-타이틀 출력
	gotoxy(21, 8);
	printf("                               .d8888b.                                            888      \n");
	gotoxy(21, 9);
	printf("                              d88P  \"88b                                           888      \n");
	gotoxy(21, 10);
	printf("                              Y88b. d88P                                           888      \n");
	gotoxy(21, 11);
	printf("888d888 888  888 88888b.       \"Y8888P\"          .d8888b 888d888 888  888 .d8888b  88888b.  \n");
	gotoxy(21, 12);
	printf("888P\"   888  888 888 \"88b     .d88P88K.d88P     d88P\"    888P\"   888  888 88K      888 \"88b \n");
	gotoxy(21, 13);
	printf("888     888  888 888  888     888\"  Y888P\"      888      888     888  888 \"Y8888b. 888  888 \n");
	gotoxy(21, 14);
	printf("888     Y88b 888 888  888     Y88b .d8888b      Y88b.    888     Y88b 888      X88 888  888 \n");
	gotoxy(21, 15);
	printf("888      \"Y88888 888  888      \"Y8888P\" Y88b     \"Y8888P 888      \"Y88888  88888P' 888  888 \n");
	gotoxy(44, 32);
	printf("방향 키↑↓와 엔터 키로 메뉴를 선택해 주세요!");
}
void draw_menu(int selected) {
	gotoxy(58, 20);
	textcolor(selected == 0 ? BLACK : WHITE, 3);
	printf("▶ 게임시작 ◀");

	gotoxy(58, 23);
	textcolor(selected == 1 ? BLACK : WHITE, 3);
	printf("▶ 게임방법 ◀");

	gotoxy(58, 26);
	textcolor(selected == 2 ? BLACK : WHITE, 3);
	printf("▶ 게임종료 ◀");

	textcolor(WHITE, 3);
}
void draw_howToPlay() {
	unsigned char ch;
	int count = 0;
	cls(WHITE, 3);
	draw_box2(0, 0, 128, 34, "─");

	textcolor(WHITE, WHITE);
	for (int k = 5; k < 31; k++) {
		draw_hline(k, 67, 125, ' ');
	}
	textcolor(BLACK, WHITE);
	draw_box(67, 5, 125, 30, '\'');
	draw_box2(75, 7, 117, 10, '─');

	gotoxy(77, 8);
	printf("♡ player ♡");
	gotoxy(77, 9);
	textcolor(RED1, WHITE);
	printf("■■■□□□□□□□□□□□□□□□□□");

	textcolor(BLACK, WHITE);
	gotoxy(73, 12);
	printf("체력이 떨어지기 전까지 최대한 오래 살아남으세요!");

	gotoxy(78, 16);
	textcolor(BLACK, WHITE);
	printf(" 점프(스페이스바)로 장애물을 피하고");
	gotoxy(78, 18);
	printf(" 슈팅(→) 으로 몬스터를 공격하세요!");

	gotoxy(80, 23);
	printf("골드");
	gotoxy(84, 23);
	textcolor(YELLOW1, WHITE);
	printf("ⓒ");
	textcolor(BLACK, WHITE);
	gotoxy(86, 23);
	printf(" : 먹으면 점수가 올라갑니다!");

	gotoxy(80, 25);
	printf("물약 ");
	textcolor(RED2, WHITE);
	printf("@");
	textcolor(BLACK, WHITE);
	gotoxy(86, 25);
	printf(" : 먹으면 체력이 올라갑니다!");

	gotoxy(80, 27);
	printf("어택↗: 먹으면 공격력이 올라갑니다!");

	textcolor(WHITE, 3);
	gotoxy(27, 29);
	printf("< 게임 방법 >");

	textcolor(WHITE, 3);
	gotoxy(49, 32);
	printf("ESC 키를 눌러 메뉴로 돌아갑니다!");

	while (1) {

		if (count % 2 == 0) {
			gotoxy(77, 9);
			textcolor(RED1, WHITE);
			printf("□□□□□□□□□□□□□□□□□□□□");
			textcolor(WHITE, 3);
		}
		else {
			gotoxy(77, 9);
			textcolor(RED1, WHITE);
			printf("■■■■■■■■■■■■■■■■■■■■");
			textcolor(RED1, 3);
		}

		gotoxy(8, 7);
		printf(" d8b  "); gotoxy(8, 8);
		printf(" ?88                                    d8P "); gotoxy(8, 9);
		printf("  88b                                d888888P   "); gotoxy(8, 10);
		printf("  888888b  d8888b  ?88   d8P  d8P      ?88'   d8888b   "); gotoxy(8, 11);
		printf("  88P `?8bd8P' ?88 d88  d8P' d8P'      88P   d8P' ?88  "); gotoxy(8, 12);
		printf(" d88   88P88b  d88 ?8b ,88b ,88'       88b   88b  d88  "); gotoxy(8, 13);
		printf("d88'   88b`?8888P' `?888P'888P'        `?8b  `?8888P'  ");

		gotoxy(16, 16);
		printf("           d8b "); gotoxy(16, 17);
		printf("           88P "); gotoxy(16, 18);
		printf("          d88  "); gotoxy(16, 19);
		printf("?88,.d88b,888   d888b8b  ?88   d8P   "); gotoxy(16, 20);
		printf("`?88'  ?88?88  d8P' ?88  d88   88    "); gotoxy(16, 21);
		printf("  88b  d8P 88b 88b  ,88b ?8(  d88    "); gotoxy(16, 22);
		printf("  888888P'  88b`?88P'`88b`?88P'?8b   "); gotoxy(16, 23);
		printf("  88P'                          )88  "); gotoxy(16, 24);
		printf(" d88                           ,d8P  "); gotoxy(16, 25);
		printf(" ?8P                        `?888P'  ");

		count++;
		Sleep(250);
		if (kbhit() == 1) {
			ch = getch();
			if (ch == ESC) {  // ESC 키
				break;
			}
		}

	}
	draw_title();  // 메뉴로 돌아가기
}

int draw_gameover() {
	int count = 0;
	unsigned char ch;
	cls(WHITE, 3);
	draw_box2(0, 0, 128, 34, "─"); // 화면에 box를 그린다.

	textcolor(BLACK, YELLOW2);
	gotoxy(55, 21);
	printf("Your Score : %d점", score);

	textcolor(WHITE, 3);
	gotoxy(55, 26);
	printf(" 'ESC' 메인화면 ");

	while (1) {
		if (count % 2 == 0)
			textcolor(WHITE, 3);
		else
			textcolor(RED1, 3);

		gotoxy(16, 9);
		printf(" .d8888b.         d8888 888b     d888 8888888888      .d88888b.  888     888 8888888888 8888888b."); gotoxy(16, 10);
		printf("d88P  Y88b       d88888 8888b   d8888 888            d88P\" \"Y88b 888     888 888        888   Y88b"); gotoxy(16, 11);
		printf("888    888      d88P888 88888b.d88888 888            888     888 888     888 888        888    888"); gotoxy(16, 12);
		printf("888            d88P 888 888Y88888P888 8888888        888     888 Y88b   d88P 8888888    888   d88P"); gotoxy(16, 13);
		printf("888  88888    d88P  888 888 Y888P 888 888            888     888  Y88b d88P  888        8888888P\""); gotoxy(16, 14);
		printf("888    888   d88P   888 888  Y8P  888 888            888     888   Y88o88P   888        888 T88b"); gotoxy(16, 15);
		printf("Y88b  d88P  d8888888888 888   \"   888 888            Y88b..d88P    Y888P    888        888  T88b"); gotoxy(16, 16);
		printf(" \"Y8888P88 d88P     888 888       888 8888888888      \"Y88888P\"      Y8P     8888888888 888   T88b");

		count++;

		Sleep(250);

		if (kbhit() == 1) {
			ch = getch();
			if (ch == ESC) {
				draw_title();
				draw_menu(0);
				return 0;
			}
		}
	}
}

void show_monster() {
	if (is_monster_hit == 1) {	// 몬스터가 맞았으면
		textcolor(RED1, 3);		// 빨강색으로 표시
		is_monster_hit = 0;
	}
	else {
		textcolor(WHITE, 3);
	}
	int monster_y = 6;

	if (cnt % 10 == 0) {		
		monster_y = 5;	
		gotoxy(82, monster_y +27);
		printf("                                             ");	
	}
	else {
		monster_y = 6;
		gotoxy(82, monster_y - 1);
		printf("                       ");
	}
	
	gotoxy(82, monster_y);
	printf("       /-/--\                                   "); gotoxy(82, monster_y+1);
	printf("     (@~@) ' )/\                                "); gotoxy(82, monster_y+2);
	printf(" ___/--  ,` '\                                  "); gotoxy(82, monster_y+3);
	printf("(oo)__ _  ,` '}/                                "); gotoxy(82, monster_y+4);
	printf(" ^^___/ '   ' \  _                              "); gotoxy(82, monster_y+5);
	printf("       \ ' , , }/ \                             "); gotoxy(82, monster_y+6);
	printf("        (,     )   }                            "); gotoxy(82, monster_y+7);
	printf("        | , ' , \_/                             "); gotoxy(82, monster_y+8);
	printf("        /   ,  ` \  __                          "); gotoxy(82, monster_y+9);
	printf("  ,     ( ,  , `  )/  \                         "); gotoxy(82, monster_y+10);
	printf("__)----/ ' (  ) ' |    }                        "); gotoxy(82, monster_y+11);
	printf("==-___( , / '`/` , \__/                         "); gotoxy(82, monster_y+12);
	printf(" '  , { _^ ',) ,  ' } __                        "); gotoxy(82, monster_y+13);
	printf("  __)---  , _;  ,' ,\/  \                       "); gotoxy(82, monster_y+14);
	printf("  ==-______/;        |   }            _/-\_      "); gotoxy(82, monster_y+15);
	printf("   '  { ' , ', ' '  , \_/            / ' , \_    "); gotoxy(82, monster_y+16);
	printf("      | ,       ' `    )          __| ' _ `  |   "); gotoxy(82, monster_y+17);
	printf("      |   ,(' ,')  ' ,  }-\       \/ ' ( ) ` \   "); gotoxy(82, monster_y+18);
	printf("       (` (  ,  ,)      \  }      {','/   \ ' }  "); gotoxy(82, monster_y+19);
	printf("       | ,( `   ,) '  ,` \/    __{ , {     {`,|  "); gotoxy(82, monster_y+20);
	printf("       \  ( , ', )  '  '  |    \/   /      (  )  "); gotoxy(82, monster_y+21);
	printf("     ___\( `,   {  ,  ' ' )/\__{ ', }       `'  "); gotoxy(82, monster_y+22);
	printf("   =-     (, ; (__  ,  ,  '` '   ,,'            "); gotoxy(82, monster_y+23);
	printf("   =_____-(   )   \__ `  '  , `  /              "); gotoxy(82, monster_y+24);
	printf("       ___\ ' ;)     `---_____,-'               "); gotoxy(82, monster_y+25);
	printf("     =-    '___/                                "); gotoxy(82, monster_y+26);
	printf("     =_____---                                  "); 
	//gotoxy(82, monster_y + 27);
	//printf("                                                     ");
}

//게임 초기화, 처음 시작과 Restart때 호출
void init_game(int p_hp, int m_hp, int s,int power)
{
	char cmd[100];

	srand(time(NULL));

	player_hp = p_hp;
	monster_hp = m_hp;
	score = s;
	player_power = power;
	no_bullet = 0;
	bullet_front = 0;
	bullet_tail = 0;

	cls(WHITE, 3);

	textcolor(WHITE, WHITE);
	draw_hline(0, 0, 130, ' ');
	draw_hline(1, 0, 130, ' ');
	draw_hline(2, 0, 130, ' ');
	draw_hline(3, 0, 130, ' ');
	draw_hline(34, 0, 130, ' ');

	textcolor(BLACK, WHITE);

	draw_box2(0, 0, 46, 3, '─');
	draw_box2(82, 0, 128, 3, '─');
	draw_box(50, 0, 79, 3, '\'');

	textcolor(WHITE, BLUE2);
	gotoxy(3, 1);
	printf("♡ player ♡");

	textcolor(WHITE, 3);
	gotoxy(24, 1);
	printf("attack↗:            ");
	show_bullet_count();
	textcolor(WHITE, RED1);
	gotoxy(115, 1);
	printf("※ monster ※");


	textcolor(BLACK, YELLOW2);
	gotoxy(51, 1);
	printf("        ☆ score ☆         ");

	textcolor(WHITE, 3);
	draw_hline(4, 0, 130, '=');
	draw_hline(33, 0, 130, '=');
	draw_hline(34, 0, 130, '=');

	//sprintf(cmd, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	//system(cmd);
}
void show_score() {
	//점수
	textcolor(BLACK, WHITE);
	gotoxy(61, 2);
	printf("%6d 점", score);
	textcolor(WHITE, 3);
}
void show_player_hp_power()
{
	//플레이어 체력
	textcolor(RED1, WHITE);
	int hp_x = 1;	// 출력 x좌표
	for (int i = 0; i <= (player_hp / 5); i++) {
		gotoxy(hp_x += 2, 2); // x 좌표 이동하면서 
		printf("■");	// 남은 체력만큼 체력바 ■ 출력
	}
	for (int i = (player_hp / 5); i < 20; i++) {
		gotoxy(hp_x += 2, 2);
		printf("□");			//  ■ 지우기 (공백 출력)
	}
	//gotoxy(5, 0);
	//printf("체력 %d", player_hp);	//체력확인

	//공격력 출력
	textcolor(WHITE, 3);
	gotoxy(34, 1);
	printf("%1d%%", player_power*100);

}
void show_monster_hp() {
	//몬스터체력
	textcolor(RED1, WHITE);
	int hp_x = 128;	//체력바 x 좌표 끝
	for (int i = 0; i <= (monster_hp / 5*2); i++) {
		gotoxy(hp_x -= 2, 2);
		printf("■");
	}
	
	for (int i = (monster_hp / 5*2); i <= 20; i++) {
		gotoxy(hp_x -= 2, 2);
		printf("□");
	}
	gotoxy(84, 1);
	textcolor(WHITE, 3);
	printf("%d kill", kill_count);

}

int check_player() {
	//플레이어가 장애물 부딪혔는지 체크
	// 아이템도 체크 추가 
	for (int i = 8; i <= 12; i++) {
		for (int j = 0; j <= 5; j++) {
			if (map[player_y + j][player_x + i] == 1) {
				if (!is_player_hit) {	// 플레이어가 맞은 상태가 아니어야함
					player_hp -= 10;
					is_player_hit = 1;	// 맞은 상태
				}
				return 1;
			}

			//아이템 먹기
			else if (items[player_x + i][player_y + j] == 1) {	//gold
				score += 100;	//점수 추가
				items[player_x + i][player_y + j] = 0;	//골드 제거
			}
			else if (items[player_x + i][player_y + j] == 2) {	//potion
				player_hp += 5;	//체력 증가
				items[player_x + i][player_y + j] = 0;	//포션 제거
			}
			else if (items[player_x + i][player_y + j] == 3) {	//potion
				player_power += 1;	//공격력 증가
				items[player_x + i][player_y + j] = 0;	//어택 제거
			}
		}
	}
	is_player_hit = 0;
	return 0;
}
void show_player(int x, int y) {
	gotoxy(x, y);
	check_player();
	if (is_player_hit == 1)
		textcolor(RED1, 3);
	else
		textcolor(WHITE, 3);

	if (cnt % 2 == 0) {
		printf("	∠￣＼  "); gotoxy(x, y + 1);
		printf("	| 'U' | "); gotoxy(x, y + 2);
		printf("     （｀ 二⊃ "); gotoxy(x, y + 3);
		printf("     ~（　` / "); gotoxy(x, y + 4);
		printf("       / >ノ "); gotoxy(x, y + 5);
		printf("       U     ");
	}
	else {
		printf("	∠￣＼  "); gotoxy(x, y + 1);
		printf("	| 'U' | "); gotoxy(x, y + 2);
		printf("     （｀  >   "); gotoxy(x, y + 3);
		printf("       |    ) "); gotoxy(x, y + 4);
		printf("     ~  し> | "); gotoxy(x, y + 5);
		printf("           U");
	}
	textcolor(WHITE, 3);
}
void player_jump() {
	if (is_jumping) {
		if (jump_height < 10) {
			player_y--;	//점프
			jump_height++;
			// 발 지우기
			gotoxy(player_x, player_y + 6);
			printf("              ");
		}
		else if (jump_height < 20) {
			player_y++;	//점프 후 내려오기
			jump_height++;
			// 머리 지우기
			gotoxy(player_x, player_y - 1);
			printf("              ");
		}
		else {
			is_jumping = 0; // 점프 종료
			jump_height = 0;
		}
	}
}

// 임의의 위치에 gold 또는 potion 또는 attack 를 표시한다.
void show_item()
{
	int x, y;
	x = rand() % 15 + 60;
	y = rand() % 15 + 17;

	int r = rand() % 100;
	if (r < 25) {	// 25% potion 생성 : 체력증가
		textcolor(RED2, 3);
		gotoxy(x, y);
		printf(POTION);
		items[x][y] = 2;	// 포션:2
		items_count++;
	}
	else if (r < 35) {	// 10% attack 생성 : 공격력증가
		textcolor(BLACK, 3);
		gotoxy(x, y);
		printf(ATTACK);
		items[x][y] = 3;	// 어택:3 
		items_count++;
	}
	// 65% gold 생성 : 점수증가
	else {
		textcolor(YELLOW2, 3);
		gotoxy(x, y);
		printf(GOLD);
		items[x][y] = 1;	// 골드:1
		items_count++;
	}
	textcolor(WHITE, 3);
}
void move_erase_item() {
	for (int x = 0; x < 80; x++) {
		for (int y = 15; y < HEIGHT; y++) {
			if (items[x][y] == 1) {	//gold
				items[x][y] = 0;
				textcolor(WHITE, 3);
				gotoxy(x, y);
				printf(" ");	//지우기
				if (x != 0) {
					textcolor(YELLOW2, 3);
					items[x - 1][y] = 1;
					gotoxy(x - 1, y);
					printf(GOLD);
					textcolor(WHITE, 3);

				}
			}
			else if (items[x][y] == 2) {	//potion
				items[x][y] = 0;
				textcolor(WHITE, 3);
				gotoxy(x, y);
				printf(" ");
				if (x-2 > 0) {
					textcolor(RED2, 3);
					items[x - 2][y] = 2;
					gotoxy(x - 2, y);
					printf(POTION);
					textcolor(WHITE, 3);
				}
			}
			else if (items[x][y] == 3) { // attack
				items[x][y] = 0;
				textcolor(WHITE, 3);
				gotoxy(x, y);
				printf(" ");
				if (x-3 > 0) {
					textcolor(BLACK, 3);
					items[x - 3][y] = 3;
					gotoxy(x - 3, y);
					printf(ATTACK);
					textcolor(WHITE, 3);
				}
			}
		}
	}
}

void draw_kill() {
	PlaySound(TEXT("bonustime.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

	cls(WHITE, 3);
	draw_box2(0, 0, 128, 34, "─"); // 화면에 box를 그린다.
	gotoxy(56, 25);
	printf("! 몬스터 처치 !");

	int x=27, y=5;

	for (int i = 0; i < 12; i++) {
		if (i % 2 == 0)
			textcolor(BLACK, 3);
		else
			textcolor(YELLOW2, 3);

		x = 27, y = 5;
		gotoxy(x, y);
		printf("                                                      .                       "); gotoxy(x, y + 1);
		printf("                                                    .o8                       "); gotoxy(x, y + 2);
		printf(" ooo. .oo.  .oo.    .ooooo.  ooo. .oo.    .oooo.o .o888oo  .ooooo.  oooo d8b  "); gotoxy(x, y + 3);
		printf(" `888P\"Y88bP\"Y88b  d88' `88b `888P\"Y88b  d88(  \"8   888   d88' `88b `888\"\"8P  "); gotoxy(x, y + 4);
		printf("  888   888   888  888   888  888   888  `\"Y88b.    888   888ooo888  888      "); gotoxy(x, y + 5);
		printf("  888   888   888  888   888  888   888  o.  )88b   888 . 888    .o  888      "); gotoxy(x, y + 6);
		printf(" o888o o888o o888o `Y8bod8P' o888o o888o 8\"\"888P'   \"888\" `Y8bod8P' d888b     ");
		x = 48; y = 15;
		gotoxy(x, y);
		printf(" oooo         o8o  oooo  oooo "); gotoxy(x, y + 1);
		printf(" `888         `\"'  `888  `888 "); gotoxy(x, y + 2);
		printf("  888  oooo  oooo   888   888 "); gotoxy(x, y + 3);
		printf("  888 .8P'   `888   888   888 "); gotoxy(x, y + 4);
		printf("  888888.     888   888   888 "); gotoxy(x, y + 5);
		printf("  888 `88b.   888   888   888 "); gotoxy(x, y + 6);
		printf(" o888o o888o o888o o888o o888o ");
		gotoxy(50, 32);
		printf("** 스피드가 더 빨라집니다 **");
		
		Sleep(300);
	}

	cls(WHITE, 3);
	init_game(player_hp, monster_hp, score, player_power);
	show_score();
	show_player_hp_power();
	show_monster_hp();
	PlaySound(TEXT("game.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

}

void main()
{
	int i, x, y;
	int selected = 0;	// 메뉴 선택 변수 0:시작 1:방법 2:종료
	unsigned char ch; // 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	int oldx, oldy, newx, newy;
	int keep_moving;

	removeCursor(); // 커서를 안보이게 한다
	draw_title();
	draw_menu(selected);
	PlaySound(TEXT("lobby.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	while (1) {
		ch = getch();
		if (ch == UP) {
			selected = (selected - 1 + 3)  % 3;
			draw_menu(selected);
		}

		else if (ch == DOWN) {
			selected = (selected + 1) % 3;
			draw_menu(selected);
		}

		if (ch == 13) {
			if (selected == 0) {	// 게임 시작
				PlaySound(TEXT("game.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

				init_game(100, 50, 0, 1);
				show_score();
				show_player_hp_power();
				show_monster_hp();
				
				init_block();
				show_block();
				init_fire();
				show_fire();
				oldx = newx = player_x;
				oldy = newy = player_y;

				while (1) {
					if (kbhit() == 1) { // 키보드가 눌려져 있으면
						ch = getch(); // key 값을 읽는다

						if (ch == ESC) { //ESC 누르면 프로그램 종료 
							break;
						}
						if (ch == SPACE && is_jumping != 1) { // 점프
							is_jumping = 1;
							jump_height = 0;
						}
						if (ch == SHOOT) { // 슈팅
							new_bullet(player_x + 14, player_y + 2); // 플레이어 오른쪽에서 총알 발사
						}
					}
					if (is_jumping) player_jump();

					show_player(player_x, player_y);
					show_monster();

					if (cnt % iteminterval == 0) {
						show_item();
						
					}

					if (cnt % 10 == 0) {
						score += 10;
						player_hp--;
					}

					//oldx = player_x;
					//oldy = player_y;

					show_score();
					show_player_hp_power();
					show_monster_hp();

					move_block(-block_speed, 0); //x 좌표를 - 1씩, -- > 왼쪽으로 이동
					move_fire(-fire_speed, 0);
					move_bullet();
					
					move_erase_item();

					Sleep(Delay); // Delay를 줄이면 속도가 빨라진다.
					cnt++;

					if (monster_hp <= 0) {	// 몬스터 처치
						score += 3000;
						kill_count++;
						draw_kill();
						bullet_max_count += 2;	
						monster_hp = 50;
						Delay -= 5;
					}
					if (player_hp <= 0) {	// 게임오버
						Sleep(500);
						PlaySound(TEXT("gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
						draw_gameover();
						PlaySound(TEXT("lobby.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
						break;
					}
				}
			}
			else if (selected == 1) {	// 게임 방법
				draw_howToPlay();
				draw_title();
				draw_menu(selected);
			}
			else if (selected == 2) {	// 게임 종료
				gotoxy(0, 36);	// 끝날때 위치
				exit(0);
			}
		}
	}
}