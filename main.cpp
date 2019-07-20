//#include <stdio.h>
//#include <io.h>
//#include <fcntl.h>
#include<windows.h>
#include<fstream>
#include<math.h>
#include "gl/gl.h"
#include "gl/glu.h"
#include "gl/glaux.h"
#include "gl/glut.h"
#include <ctime>
#include "resource.h"
#include "audiere.h"
using namespace audiere;
using namespace std;
#pragma warning(disable: 4996)
#pragma comment (lib, "glaux.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")
#pragma comment (lib, "audiere.lib")
//#pragma comment (linker, "/ENTRY:WinMainCRTStartup")
//#pragma comment (linker, "/SUBSYSTEM:CONSOLE")
#define pi  3.1415926
#define CM_OPEN 4000
#define CM_SAVE 4001
#define CM_SAVEAS 4002
#define CM_EXIT 4004
#define CM_DELETE 4005
HINSTANCE _hInst;
HWND hWnd;
HGLRC hGLRC;
HDC hDC;
//Разлиные кисти
HBRUSH fon_brush = CreateSolidBrush(RGB(0, 204, 204));
HBRUSH while_brush = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH grey_brush = CreateSolidBrush(RGB(192, 192, 192));
HBRUSH orange_brush = CreateSolidBrush(RGB(255, 128, 0));
HBRUSH blue_brush = CreateSolidBrush(RGB(0, 255, 255));
HBRUSH green_brush = CreateSolidBrush(RGB(0, 255, 0));
HBRUSH dark_blue_brush = CreateSolidBrush(RGB(0, 0, 255));
HBRUSH purple_brush = CreateSolidBrush(RGB(204, 0, 204));
HBRUSH yellow_brush = CreateSolidBrush(RGB(255, 255, 0));
HBRUSH red_brush = CreateSolidBrush(RGB(255, 0, 0));
//________________________________________________________
//Обрабочики для окон
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcADD(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcRecords(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//______________________________________________________________________________
int STARTtime;
int STARTtimeS;
float speed = 0.5;
float speedBALL = 10;
bool keys[256];
bool story;
int indexMenu;
char NickName[20];
char path[MAX_PATH];
struct GAME {
	int score;
	bool game;
	int life;
	int lvl;
	bool menu;
	bool win;
};
struct PLATFORM {
	float lx;
	float rx;
	int bonus;
	bool destroy;
};
struct BALL {
	float r;
	float x, y;
	bool glued;
	int angle;
	bool up;

};
struct MAP {
	float x, y;
	int state;
	int MAXstate;
	int index;
	int point;
	MAP* next;
	MAP* back;
};
struct MAP_SAVE {
	float x, y;
	int state;
	int MAXstate;
	int index;
	int point;
};
struct SHOOT {
	float x;
	float y;
	bool l, r;
	int index;
	SHOOT* next;
	SHOOT* back;
};
struct RECORDS {
	char NickName[20];
	int point;
	bool active;
};
struct BONUS
{
	float x, y;
	int bonus;
	int index;
	BONUS *next, *back;
};
GAME *game = NULL;
PLATFORM *platform = NULL;
BALL *ball = NULL;
MAP *block = NULL;
SHOOT *shoot = NULL;
BONUS *bonus = NULL;
RECORDS records[11];
//ерунда связанная со звуком
AudioDevicePtr device;
OutputStreamPtr soundMenu, soundHistory, soundGame, soundHit;
//________________________________________________________________________________
//текстуры
unsigned int arkanoid, Fon, BorderL, BorderR, BorderT, Shoot, Destroy[30],Menu[5], Block[36],Story[76],Load1, ArkanoidS,TableR,TableO,TableGr,TableB,TableG;
void delBlock(int index); //удаление блока из очереди
void delShoot(int index); //удалчение выстрела из очереди
void delBonus(int index); //удаление бонуса из очереди
//загрузка рекордов
void LoadRecords() {
	int i = 0;
	char buf[MAX_PATH];
	buf[0] = 0;
	strcat(buf, path);
	strcat(buf, "records.rk");
	ifstream in(buf, ios::binary | ios::in);
	if (in)
	while (true) {
			in.read((char*)&records[i], sizeof(RECORDS));
			if (in.eof()) {break;}
			i++;
		}
}
//сохранение рекордов в файл
void SaveRecords() {
	LoadRecords();
	for (int i = 0; i < 10; i++) {
		if (game->score > records[i].point) {
			for (int j = 9; j >= i+1; j--) {
				records[j].active = records[j - 1].active;
				strcpy(records[j].NickName, records[j - 1].NickName);
				records[j].point = records[j - 1].point;
			}
			DialogBox(_hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DlgProcADD);
			records[i].point = game->score;
			records[i].active = true;
			strcpy(records[i].NickName, NickName);
			NickName[0] = 0;
			break;
		}
	}
	char buf[MAX_PATH];
	buf[0] = 0;
	strcat(buf, path);
	strcat(buf, "records.rk");
	ofstream f(buf, ios::binary | ios::out);
	for (int i = 0; i<10; i++) {
		f.write((char*)&records[i], sizeof(RECORDS));
	}
	f.close();
}
//отрисовка загрузки
void DrawLoading() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, Load1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(0,1); glVertex3f(0,680,0);
	glTexCoord2f(1,1); glVertex3f(570,680,0);
	glTexCoord2f(1,0); glVertex3f(570,0,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	SwapBuffers(wglGetCurrentDC());
}
//функция загрузки карты
void LoadMap(char *file) {
		ifstream in;
		char pass[17];
		pass[16] = 0;
		int i = 0;
		char buf[MAX_PATH];
		buf[0] = 0;
		strcat(buf, path);
		//выбор карты в зависимости от уровня, если -1, то загрузить кастомную
		switch (game->lvl) {
		case 0:
			strcat(buf, "\\map\\map1.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case 1:
			strcat(buf, "\\map\\map2.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case 2:
			strcat(buf, "\\map\\map3.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case 3:
			strcat(buf, "\\map\\map4.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case 4:
			strcat(buf, "\\map\\map5.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case 5:
			strcat(buf, "\\map\\map6.ark");
			in.open(buf, ios::binary | ios::in);
			break;
		case -1:
			in.open(file, ios::binary | ios::in);
		}


		MAP_SAVE *tempMAP = (MAP_SAVE*)malloc(0);
		//считываем 16 байт
		in.read(pass, 16);
		//если считанные 16 байт не совпадают с теми, что дописываются всегда в  начале карты, то открыт не тот файл
		if (strcmp(pass, "6Q91Ne3MQ2foqnD4") == 0) {
			while (block != NULL) delBlock(0); //очистка всех блоков
			MAP_SAVE *cur = new MAP_SAVE;
			//считывание данных с файла
			while (true) {
				in.read((char*)&*cur, sizeof(MAP_SAVE));
				if (in.eof()) { delete cur; break; }
				i++;
				tempMAP = (MAP_SAVE*)realloc(tempMAP, i * sizeof(MAP_SAVE));
				tempMAP[i - 1].index = cur->index;
				tempMAP[i - 1].MAXstate = cur->MAXstate;
				tempMAP[i - 1].point = cur->point;
				tempMAP[i - 1].state = 0;
				tempMAP[i - 1].x = cur->x;
				tempMAP[i - 1].y = cur->y;
			}
			MAP *temp = block;
			for (int j = 0; j < i; j++) {
				MAP *cur = new MAP;
				cur->index = j;
				cur->MAXstate = tempMAP[j].MAXstate;
				cur->point = tempMAP[j].point;
				cur->state = tempMAP[j].state;
				cur->x = tempMAP[j].x;
				cur->y = tempMAP[j].y;
				cur->next = NULL;
				if (temp == NULL) {
					cur->back = NULL;
					block = cur;
					temp = block;
				}
				else {
					while (temp->next != NULL)
						temp = temp->next;
					temp->next = cur;
					cur->back = temp;
				}
			}
			//___________________________________________________
		}
		else {
			MessageBox(0, "Файл не поддерживается", "Ошибка", MB_OK);
			game->menu = true;
		}
		free(tempMAP);
		in.close();
}
//обновляем клавиатуру
void RefreshKeyboard() {
	for (int i = 0; i < 256; i++)
		keys[i] = false;
}
//обработка клавиш клавиатуды
void keyboard() {
	//перемещение платформу вправо
	if (keys[VK_RIGHT] == true) {
		if ((platform->rx + speed + 10.0 / 256 < 552)&&(ball->x+ball->r + speed<552)) {
			platform->lx = platform->lx + speed;
			platform->rx = platform->rx + speed;
			if (ball->glued == true)
				ball->x = ball->x + speed;
		}
	}
	//влево
	if (keys[VK_LEFT] == true) {
		if ((platform->lx - speed - 10.0 / 256 > 17)&&(ball->x - speed - ball->r>17)) {
			platform->lx = platform->lx - speed;
			platform->rx = platform->rx - speed;
			if (ball->glued == true)
				ball->x = ball->x - speed;
		}
	}
	//нажатие пробела
	if (keys[VK_SPACE] == true) {
		//если прилеплен мяч, то отпустить его
		if (ball->glued == true) {
			ball->glued = false;
			keys[VK_SPACE] = false;
		}
		else
			//если есть бонус стрельбы то совершить выстрел
			if (platform->bonus == 1) {	
				if (GetTickCount() - STARTtimeS >= 300) {
					SHOOT *cur = new SHOOT;
					cur->x = platform->lx + 9;
					cur->y = 42;
					cur->l = true;
					cur->r = true;
					if (shoot == NULL) {
						shoot = cur;
						shoot->back = NULL;
						shoot->next = NULL;
						shoot->index = 0;
					}
					else
					{
						SHOOT *temp = shoot;
						while (temp->next != NULL) temp = temp->next;
						temp->next = cur;
						cur->next = NULL;
						cur->back = temp;
						cur->index = cur->back->index + 1;
					}
					STARTtimeS = GetTickCount();
				}
			}
	}
}

//ну тут всякая отрисовка цифр и букв, они вписываются в прямоугольник, которые задают заданные координаты
void null(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 5.0, oneY = (y1 - y2) / 8.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y2 + oneY, 0);
	glVertex3f(x1, y2 + oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX * 4, y1, 0);
	glVertex3f(x1 + oneX * 4, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX * 4, y1 - oneY, 0);
	glVertex3f(x1 + oneX * 5, y1 - oneY, 0);
	glVertex3f(x2, y2 + oneY, 0);
	glVertex3f(x2 - oneX, y2 + oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y2, 0);
	glVertex3f(x2 - oneX, y2 + oneY, 0);
	glVertex3f(x2 - oneX * 4, y2 + oneY, 0);
	glVertex3f(x1 + oneX, y2, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX * 4, y2 + oneY * 2, 0);
	glVertex3f(x2 - oneX * 4, y2 + oneY * 3, 0);
	glVertex3f(x2 - oneX * 3, y2 + oneY * 3, 0);
	glVertex3f(x2 - oneX * 3, y2 + oneY * 2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX * 3, y2 + oneY * 3, 0);
	glVertex3f(x2 - oneX * 3, y2 + oneY * 4, 0);
	glVertex3f(x2 - oneX * 2, y2 + oneY * 4, 0);
	glVertex3f(x2 - oneX * 2, y2 + oneY * 3, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX * 2, y2 + oneY * 4, 0);
	glVertex3f(x2 - oneX * 2, y2 + oneY * 5, 0);
	glVertex3f(x2 - oneX * 1, y2 + oneY * 5, 0);
	glVertex3f(x2 - oneX * 1, y2 + oneY * 4, 0);
	glEnd();
}
void one(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 8.0;
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x2-oneX,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-2*oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1, y1 - 3 * oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+2*oneX,y1-oneY,0);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glEnd();
}
void two(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x1 + 3 * oneX,y1,0);
	glVertex3f(x1 + 3 * oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1 - oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX*3,y1-oneY,0);
	glVertex3f(x1+oneX*4,y1-oneY,0);
	glVertex3f(x1+oneX*4,y1-3*oneY,0);
	glVertex3f(x1+oneX*3,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX*2,y1-3*oneY,0);
	glVertex3f(x1+oneX*3,y1-3*oneY,0);
	glVertex3f(x1+oneX*3,y1-4*oneY,0);
	glVertex3f(x1+oneX*2,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+3*oneY,0);
	glVertex3f(x1+2*oneX,y2+3*oneY,0);
	glVertex3f(x1+2*oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
}
void three(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x1+3*oneX,y1,0);
	glVertex3f(x1+3*oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+2*oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-4*oneY,0);
	glVertex3f(x1+2*oneX,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+3*oneY,0);
	glVertex3f(x2,y2+3*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y1-3*oneY,0);
	glVertex3f(x2-oneX,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
}
void four(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x2-oneX,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX*2,y1-oneY,0);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2-oneX,y1-2*oneY,0);
	glVertex3f(x2-2*oneX,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1 + 2 * oneX,y1-3*oneY,0);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 - 3 * oneY,0);
	glVertex3f(x1+oneX, y1 - 3 * oneY,0);
	glVertex3f(x1 + oneX,y1-4*oneY,0);
	glVertex3f(x1,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+3*oneY,0);
	glVertex3f(x2-oneX,y2+3*oneY,0);
	glVertex3f(x2-oneX,y2+2*oneY,0);
	glVertex3f(x1, y2 + 2 * oneY,0);
	glEnd();
	

}
void five(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-4*oneY,0);
	glVertex3f(x1+oneX,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+3*oneY,0);
	glVertex3f(x2,y2+3*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
}
void six(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x2-oneX,y1,0);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX, y1 - 3 * oneY, 0);
	glVertex3f(x1 + 3 * oneX, y1 - 3 * oneY, 0);
	glVertex3f(x1 + 3 * oneX, y1 - 4 * oneY, 0);
	glVertex3f(x1 + oneX, y1 - 4 * oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y1-2*oneY,0);
	glVertex3f(x2-oneX,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+3*oneY,0);
	glVertex3f(x2,y2+3*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
}
void seven(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x1+3*oneX,y1,0);
	glVertex3f(x1+3*oneX,y1-oneY,0);
	glVertex3f(x1,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y1-oneY*3,0);
	glVertex3f(x2-oneX,y1-oneY*3,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-2*oneX,y1-3*oneY,0);
	glVertex3f(x2-oneX,y1-3*oneY,0);
	glVertex3f(x2-oneX,y1-4*oneY,0);
	glVertex3f(x2-2*oneX,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+3*oneY,0);
	glVertex3f(x1+2*oneX,y2+3*oneY,0);
	glVertex3f(x1+2*oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
}
void eight(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x2-oneX,y1,0);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y1-3*oneY,0);
	glVertex3f(x2-oneX,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-4*oneY,0);
	glVertex3f(x1+oneX,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+3*oneY,0);
	glVertex3f(x1+oneX,y2+3*oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+3*oneY,0);
	glVertex3f(x2,y2+3*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
}
void nine(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 7.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x1+3*oneX,y1,0);
	glVertex3f(x1+3*oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+3*oneX,y1-4*oneY,0);
	glVertex3f(x1+oneX,y1-4*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneY,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glVertex3f(x2-oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
}
void s(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x1,y1-oneY,0);
	glEnd();
    glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY*2,0);
	glVertex3f(x2,y1-oneY*2,0);
	glVertex3f(x2,y2+2*oneY,0);
	glVertex3f(x1,y2+2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+2*oneY,0);
	glVertex3f(x2,y2+2*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
}
void c(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x1,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x1,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y2+oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
}
void o(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y1 - oneY, 0);
	glVertex3f(x1, y1 - oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y2 + oneY, 0);
	glVertex3f(x1, y2 + oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y2 + oneY, 0);
	glVertex3f(x2, y2 + oneY, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x1, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
}
void r(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y1 - oneY, 0);
	glVertex3f(x1, y1 - oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y2 , 0);
	glVertex3f(x1, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-oneY*2,0);
	glVertex3f(x1+2*oneX,y1-oneY*2,0);
	glVertex3f(x1+2*oneX,y1-oneY*3,0);
	glVertex3f(x1+oneX,y1-oneY*3,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2,y1-oneY,0);
	glVertex3f(x2,y1-2*oneY,0);
	glVertex3f(x2-oneX,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y2+oneY*2,0);
	glVertex3f(x2,y2+2*oneY,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x2-oneX,y2,0);
	glEnd();
}
void e(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y1 - oneY, 0);
	glVertex3f(x1, y1 - oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glVertex3f(x1 + oneX, y2 + oneY, 0);
	glVertex3f(x1, y2 + oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1, y2 + oneY, 0);
	glVertex3f(x2, y2 + oneY, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x1, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX, y1 - oneY * 2, 0);
	glVertex3f(x1 + 2 * oneX, y1 - oneY * 2, 0);
	glVertex3f(x1 + 2 * oneX, y1 - oneY * 3, 0);
	glVertex3f(x1 + oneX, y1 - oneY * 3, 0);
	glEnd();
}
void g(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x1+4*oneX,y1,0);
	glVertex3f(x1+4*oneX,y1-oneY,0);
	glVertex3f(x1,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1-2*oneY,0);
	glVertex3f(x2,y1-2*oneY,0);
	glVertex3f(x2,y2+oneY,0);
	glVertex3f(x2-oneX,y2+oneY,0);
	glEnd();
    glBegin(GL_POLYGON);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1+3*oneX,y1-2*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+2*oneX,y1-3*oneY,0);
	glEnd();

}
void a(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1,y1,0);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x1+oneX,y2,0);
	glVertex3f(x1,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1,0);
	glVertex3f(x1+2*oneX,y1,0);
	glVertex3f(x1+2*oneX,y1-oneY,0);
	glVertex3f(x1+oneX,y1-oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y2,0);
	glVertex3f(x2-oneX,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX, y1 - 2 * oneY, 0);
	glVertex3f(x1 + 2 * oneX, y1 - 2 * oneY, 0);
	glVertex3f(x1 + 2 * oneX, y1 - 3*oneY, 0);
	glVertex3f(x1 + oneX, y1 - 3*oneY, 0);
	glEnd();
}
void m(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 5.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX, y2, 0);
	glVertex3f(x1, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-oneY,0);
	glVertex3f(x1+2*oneX,y1-oneY,0);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1+3*oneX,y1-2*oneY,0);
	glVertex3f(x1+3*oneX,y1-3*oneY,0);
	glVertex3f(x1+2*oneX,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-2*oneX,y1-oneY,0);
	glVertex3f(x2-oneX,y1-oneY,0);
	glVertex3f(x2-oneX,y1-2*oneY,0);
	glVertex3f(x2-2*oneX,y1-2*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glEnd();
}
void v(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX, y2+oneY, 0);
	glVertex3f(x1, y2+oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2+oneY, 0);
	glVertex3f(x2 - oneX, y2+oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - 2*oneX, y2+2*oneY, 0);
	glVertex3f(x2-oneX, y2 + 2 * oneY, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glVertex3f(x2 - 2 * oneX, y2, 0);
	glEnd();
}
void y(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX, y1 - 3 * oneY, 0);
	glVertex3f(x1, y1 - 3 * oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y1-2*oneY,0);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x1+2*oneX,y2,0);
	glVertex3f(x1+oneX,y2,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2-oneX,y1,0);
	glVertex3f(x2,y1,0);
	glVertex3f(x2,y2+2*oneY,0);
	glVertex3f(x2-oneX,y2+2*oneY,0);
	glEnd();
}
void u(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1 , 0);
	glVertex3f(x1 + oneX, y1 , 0);
	glVertex3f(x1 + oneX, y2 , 0);
	glVertex3f(x1, y2 , 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX, y2+oneY, 0);
	glVertex3f(x2-oneX, y2 + oneY, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glVertex3f(x1 + oneX, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glEnd();

}
void w(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 5.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX, y2, 0);
	glVertex3f(x1, y2, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX,y2+2*oneY,0);
	glVertex3f(x1+2*oneX,y2+2*oneY,0);
	glVertex3f(x1+2*oneX,y2+oneY,0);
	glVertex3f(x1+oneX,y2+oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1+2*oneX,y1-2*oneY,0);
	glVertex3f(x2-2*oneX,y1-2*oneY,0);
	glVertex3f(x2-2*oneX,y1-3*oneY,0);
	glVertex3f(x1+2*oneX,y1-3*oneY,0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y2 + 2 * oneY, 0);
	glVertex3f(x2 - 2 * oneX, y2 + 2 * oneY, 0);
	glVertex3f(x2 - 2 * oneX, y2 + oneY, 0);
	glVertex3f(x2 - oneX, y2 + oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glEnd();
}
void i(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 3.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1+oneX, y1, 0);
	glVertex3f(x1 + 2*oneX, y1, 0);
	glVertex3f(x1 + 2*oneX, y2, 0);
	glVertex3f(x1+oneX, y2, 0);
	glEnd();
}
void n(float x1, float y1, float x2, float y2) {
	float oneX = (x2 - x1) / 4.0, oneY = (y1 - y2) / 5.0;
	glBegin(GL_POLYGON);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1 + oneX, y1, 0);
	glVertex3f(x1 + oneX, y2, 0);
	glVertex3f(x1, y2, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(x1 + oneX, y1 - oneY, 0);
	glVertex3f(x1 + 2 * oneX, y1 - oneY, 0);
	glVertex3f(x1 + 2 * oneX, y1-2* oneY, 0);
	glVertex3f(x1 + oneX, y1 - 2 * oneY, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x1 + 2 * oneX, y1 - 2 * oneY, 0);
	glVertex3f(x2 -   oneX, y1 - 2 * oneY, 0);
	glVertex3f(x2 -   oneX, y1 - 3 * oneY, 0);
	glVertex3f(x1 + 2 * oneX, y1 - 3 * oneY, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(x2 - oneX, y1, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2 - oneX, y2, 0);
	glEnd();
}
//тут закончилось, вот уже и минус 100 строк :D

//инициализация игры
void InitializeGame(char *file) {
	while (shoot != NULL) delShoot(0);
	while (bonus != NULL) delBonus(0);
	STARTtime = GetTickCount();
	STARTtimeS = GetTickCount();

	platform->lx = 242.5;
	platform->rx = 327.5;
	platform->destroy = false;
	platform->bonus = 0;
	
	game->win = false;
	game->score = 0;
	game->game = true;
	game->life = 3;

	if (strcmp(file,"")==0)
		game->lvl = 0;
	else
		game->lvl = -1;

	//инициализация мяча
	{
		ball->r = 10;
		ball->x = 285.5;
		ball->y = 50.5;
		ball->glued = true;
		ball->angle = 0;
		ball->up = true; 
	}
	//очистка рекордов
	for (int i = 0; i < 10; i++) {
		records[i].point = 0;
	}
	LoadMap(file);
	RefreshKeyboard();
}
//загрузка всех текстур
void LoadTextures() {
	static bool loaded=false;
	//проверяем былали загрузка
	if (loaded == false) {
		AUX_RGBImageRec *load1 = auxDIBImageLoadA("textures/Load1.bmp");
		{
			glGenTextures(1, &Load1);
			glBindTexture(GL_TEXTURE_2D, Load1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, load1->sizeX, load1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, load1->data);
		}
		DrawLoading(); // отрисовываем экран загрузки
		//инициализируем звук
		device = OpenDevice();
		soundMenu = OpenSound(device, "music\\menu.mp3", false);
		soundHistory = OpenSound(device, "music\\history.mp3", false);
		soundGame = OpenSound(device, "music\\game.mp3", false);
		soundHit = OpenSound(device, "music\\hit.wav", false);
		soundMenu->setRepeat(true);
		soundGame->setRepeat(true);
		//__________________________________
		AUX_RGBImageRec *tableR = auxDIBImageLoadA("textures/table/tableR.bmp");
		AUX_RGBImageRec *tableO = auxDIBImageLoadA("textures/table/tableO.bmp");
		AUX_RGBImageRec *tableGr = auxDIBImageLoadA("textures/table/tableGr.bmp");
		AUX_RGBImageRec *tableB = auxDIBImageLoadA("textures/table/tableB.bmp");
		AUX_RGBImageRec *tableG = auxDIBImageLoadA("textures/table/tableG.bmp");
		AUX_RGBImageRec *Ltexture = auxDIBImageLoadA("textures/arkanoid/arkanoid.bmp");
		AUX_RGBImageRec *arkanoidS = auxDIBImageLoadA("textures/arkanoid/ShootArk.bmp");
		AUX_RGBImageRec *fon = auxDIBImageLoadA("textures/fon.bmp");
		AUX_RGBImageRec *borderL = auxDIBImageLoadA("textures/border/borderL.bmp");
		AUX_RGBImageRec *borderR = auxDIBImageLoadA("textures/border/borderR.bmp");
		AUX_RGBImageRec *borderT = auxDIBImageLoadA("textures/border/borderT.bmp");
		AUX_RGBImageRec *shoot = auxDIBImageLoadA("textures/shoot.bmp");
		AUX_RGBImageRec **destroy = (AUX_RGBImageRec**)malloc(30 * sizeof(AUX_RGBImageRec*));
		{
			destroy[0] = auxDIBImageLoadA("textures/destroyed/Destroy1.bmp");
			destroy[1] = auxDIBImageLoadA("textures/destroyed/Destroy2.bmp");
			destroy[2] = auxDIBImageLoadA("textures/destroyed/Destroy3.bmp");
			destroy[3] = auxDIBImageLoadA("textures/destroyed/Destroy4.bmp");
			destroy[4] = auxDIBImageLoadA("textures/destroyed/Destroy5.bmp");
			destroy[5] = auxDIBImageLoadA("textures/destroyed/Destroy6.bmp");
			destroy[6] = auxDIBImageLoadA("textures/destroyed/Destroy7.bmp");
			destroy[7] = auxDIBImageLoadA("textures/destroyed/Destroy8.bmp");
			destroy[8] = auxDIBImageLoadA("textures/destroyed/Destroy9.bmp");
			destroy[9] = auxDIBImageLoadA("textures/destroyed/Destroy10.bmp");
			destroy[10] = auxDIBImageLoadA("textures/destroyed/Destroy11.bmp");
			destroy[11] = auxDIBImageLoadA("textures/destroyed/Destroy12.bmp");
			destroy[12] = auxDIBImageLoadA("textures/destroyed/Destroy13.bmp");
			destroy[13] = auxDIBImageLoadA("textures/destroyed/Destroy14.bmp");
			destroy[14] = auxDIBImageLoadA("textures/destroyed/Destroy15.bmp");
			destroy[15] = auxDIBImageLoadA("textures/destroyed/Destroy16.bmp");
			destroy[16] = auxDIBImageLoadA("textures/destroyed/Destroy17.bmp");
			destroy[17] = auxDIBImageLoadA("textures/destroyed/Destroy18.bmp");
			destroy[18] = auxDIBImageLoadA("textures/destroyed/Destroy19.bmp");
			destroy[19] = auxDIBImageLoadA("textures/destroyed/Destroy20.bmp");
			destroy[20] = auxDIBImageLoadA("textures/destroyed/Destroy21.bmp");
			destroy[21] = auxDIBImageLoadA("textures/destroyed/Destroy22.bmp");
			destroy[22] = auxDIBImageLoadA("textures/destroyed/Destroy23.bmp");
			destroy[23] = auxDIBImageLoadA("textures/destroyed/Destroy24.bmp");
			destroy[24] = auxDIBImageLoadA("textures/destroyed/Destroy25.bmp");
			destroy[25] = auxDIBImageLoadA("textures/destroyed/Destroy26.bmp");
			destroy[26] = auxDIBImageLoadA("textures/destroyed/Destroy27.bmp");
			destroy[27] = auxDIBImageLoadA("textures/destroyed/Destroy28.bmp");
			destroy[28] = auxDIBImageLoadA("textures/destroyed/Destroy29.bmp");
			destroy[29] = auxDIBImageLoadA("textures/destroyed/Destroy30.bmp");
		}
		AUX_RGBImageRec **block = (AUX_RGBImageRec**)malloc(36 * sizeof(AUX_RGBImageRec*));
		{
			block[0] = auxDIBImageLoadA("textures/blocks/1/1.bmp");
			block[1] = auxDIBImageLoadA("textures/blocks/2/1.bmp");
			block[2] = auxDIBImageLoadA("textures/blocks/2/2.bmp");
			block[3] = auxDIBImageLoadA("textures/blocks/3/1.bmp");
			block[4] = auxDIBImageLoadA("textures/blocks/3/2.bmp");
			block[5] = auxDIBImageLoadA("textures/blocks/3/3.bmp");
			block[6] = auxDIBImageLoadA("textures/blocks/4/1.bmp");
			block[7] = auxDIBImageLoadA("textures/blocks/4/2.bmp");
			block[8] = auxDIBImageLoadA("textures/blocks/4/3.bmp");
			block[9] = auxDIBImageLoadA("textures/blocks/4/4.bmp");
			block[10] = auxDIBImageLoadA("textures/blocks/5/1.bmp");
			block[11] = auxDIBImageLoadA("textures/blocks/5/2.bmp");
			block[12] = auxDIBImageLoadA("textures/blocks/5/3.bmp");
			block[13] = auxDIBImageLoadA("textures/blocks/5/4.bmp");
			block[14] = auxDIBImageLoadA("textures/blocks/5/5.bmp");
			block[15] = auxDIBImageLoadA("textures/blocks/6/1.bmp");
			block[16] = auxDIBImageLoadA("textures/blocks/6/2.bmp");
			block[17] = auxDIBImageLoadA("textures/blocks/6/3.bmp");
			block[18] = auxDIBImageLoadA("textures/blocks/6/4.bmp");
			block[19] = auxDIBImageLoadA("textures/blocks/6/5.bmp");
			block[20] = auxDIBImageLoadA("textures/blocks/6/6.bmp");
			block[21] = auxDIBImageLoadA("textures/blocks/7/1.bmp");
			block[22] = auxDIBImageLoadA("textures/blocks/7/2.bmp");
			block[23] = auxDIBImageLoadA("textures/blocks/7/3.bmp");
			block[24] = auxDIBImageLoadA("textures/blocks/7/4.bmp");
			block[25] = auxDIBImageLoadA("textures/blocks/7/5.bmp");
			block[26] = auxDIBImageLoadA("textures/blocks/7/6.bmp");
			block[27] = auxDIBImageLoadA("textures/blocks/7/7.bmp");
			block[28] = auxDIBImageLoadA("textures/blocks/8/1.bmp");
			block[29] = auxDIBImageLoadA("textures/blocks/8/2.bmp");
			block[30] = auxDIBImageLoadA("textures/blocks/8/3.bmp");
			block[31] = auxDIBImageLoadA("textures/blocks/8/4.bmp");
			block[32] = auxDIBImageLoadA("textures/blocks/8/5.bmp");
			block[33] = auxDIBImageLoadA("textures/blocks/8/6.bmp");
			block[34] = auxDIBImageLoadA("textures/blocks/8/7.bmp");
			block[35] = auxDIBImageLoadA("textures/blocks/8/8.bmp");
		}
		AUX_RGBImageRec **story = (AUX_RGBImageRec**)malloc(76 * sizeof(AUX_RGBImageRec*));
		{
			story[0] = auxDIBImageLoadA("textures/story/story.bmp");
			story[1] = auxDIBImageLoadA("textures/story/story1.bmp");
			story[2] = auxDIBImageLoadA("textures/story/story2.bmp");
			story[3] = auxDIBImageLoadA("textures/story/story3.bmp");
			story[4] = auxDIBImageLoadA("textures/story/story4.bmp");
			story[5] = auxDIBImageLoadA("textures/story/story5.bmp");
			story[6] = auxDIBImageLoadA("textures/story/story6.bmp");
			story[7] = auxDIBImageLoadA("textures/story/story7.bmp");
			story[8] = auxDIBImageLoadA("textures/story/story8.bmp");
			story[9] = auxDIBImageLoadA("textures/story/story9.bmp");
			story[10] = auxDIBImageLoadA("textures/story/story10.bmp");
			story[11] = auxDIBImageLoadA("textures/story/story11.bmp");
			story[12] = auxDIBImageLoadA("textures/story/story12.bmp");
			story[13] = auxDIBImageLoadA("textures/story/story13.bmp");
			story[14] = auxDIBImageLoadA("textures/story/story14.bmp");
			story[15] = auxDIBImageLoadA("textures/story/story15.bmp");
			story[16] = auxDIBImageLoadA("textures/story/story16.bmp");
			story[17] = auxDIBImageLoadA("textures/story/story17.bmp");
			story[18] = auxDIBImageLoadA("textures/story/story18.bmp");
			story[19] = auxDIBImageLoadA("textures/story/story19.bmp");
			story[20] = auxDIBImageLoadA("textures/story/story20.bmp");
			story[21] = auxDIBImageLoadA("textures/story/story21.bmp");
			story[22] = auxDIBImageLoadA("textures/story/story22.bmp");
			story[23] = auxDIBImageLoadA("textures/story/story23.bmp");
			story[24] = auxDIBImageLoadA("textures/story/story24.bmp");
			story[25] = auxDIBImageLoadA("textures/story/story25.bmp");
			story[26] = auxDIBImageLoadA("textures/story/story26.bmp");
			story[27] = auxDIBImageLoadA("textures/story/story27.bmp");
			story[28] = auxDIBImageLoadA("textures/story/story28.bmp");
			story[29] = auxDIBImageLoadA("textures/story/story29.bmp");
			story[30] = auxDIBImageLoadA("textures/story/story30.bmp");
			story[31] = auxDIBImageLoadA("textures/story/story31.bmp");
			story[32] = auxDIBImageLoadA("textures/story/story32.bmp");
			story[33] = auxDIBImageLoadA("textures/story/story33.bmp");
			story[34] = auxDIBImageLoadA("textures/story/story34.bmp");
			story[35] = auxDIBImageLoadA("textures/story/story35.bmp");
			story[36] = auxDIBImageLoadA("textures/story/story36.bmp");
			story[37] = auxDIBImageLoadA("textures/story/story37.bmp");
			story[38] = auxDIBImageLoadA("textures/story/story38.bmp");
			story[39] = auxDIBImageLoadA("textures/story/story39.bmp");
			story[40] = auxDIBImageLoadA("textures/story/story40.bmp");
			story[41] = auxDIBImageLoadA("textures/story/story41.bmp");
			story[42] = auxDIBImageLoadA("textures/story/story42.bmp");
			story[43] = auxDIBImageLoadA("textures/story/story43.bmp");
			story[44] = auxDIBImageLoadA("textures/story/story44.bmp");
			story[45] = auxDIBImageLoadA("textures/story/story45.bmp");
			story[46] = auxDIBImageLoadA("textures/story/story46.bmp");
			story[47] = auxDIBImageLoadA("textures/story/story47.bmp");
			story[48] = auxDIBImageLoadA("textures/story/story48.bmp");
			story[49] = auxDIBImageLoadA("textures/story/story49.bmp");
			story[50] = auxDIBImageLoadA("textures/story/story50.bmp");
			story[51] = auxDIBImageLoadA("textures/story/story51.bmp");
			story[52] = auxDIBImageLoadA("textures/story/story52.bmp");
			story[53] = auxDIBImageLoadA("textures/story/story53.bmp");
			story[54] = auxDIBImageLoadA("textures/story/story54.bmp");
			story[55] = auxDIBImageLoadA("textures/story/story55.bmp");
			story[56] = auxDIBImageLoadA("textures/story/story56.bmp");
			story[57] = auxDIBImageLoadA("textures/story/story57.bmp");
			story[58] = auxDIBImageLoadA("textures/story/story58.bmp");
			story[59] = auxDIBImageLoadA("textures/story/story59.bmp");
			story[60] = auxDIBImageLoadA("textures/story/story60.bmp");
			story[61] = auxDIBImageLoadA("textures/story/story61.bmp");
			story[62] = auxDIBImageLoadA("textures/story/story62.bmp");
			story[63] = auxDIBImageLoadA("textures/story/story63.bmp");
			story[64] = auxDIBImageLoadA("textures/story/story64.bmp");
			story[65] = auxDIBImageLoadA("textures/story/story65.bmp");
			story[66] = auxDIBImageLoadA("textures/story/story66.bmp");
			story[67] = auxDIBImageLoadA("textures/story/story67.bmp");
			story[68] = auxDIBImageLoadA("textures/story/story68.bmp");
			story[69] = auxDIBImageLoadA("textures/story/story69.bmp");
			story[70] = auxDIBImageLoadA("textures/story/story70.bmp");
			story[71] = auxDIBImageLoadA("textures/story/story71.bmp");
			story[72] = auxDIBImageLoadA("textures/story/story72.bmp");
			story[73] = auxDIBImageLoadA("textures/story/story73.bmp");
			story[74] = auxDIBImageLoadA("textures/story/story74.bmp");
			story[75] = auxDIBImageLoadA("textures/story/story75.bmp");
		}
		AUX_RGBImageRec **menu = (AUX_RGBImageRec**)malloc(5 * sizeof(AUX_RGBImageRec*));
		{
			menu[0] = auxDIBImageLoadA("textures/menu/menu1.bmp");
			menu[1] = auxDIBImageLoadA("textures/menu/menu2.bmp");
			menu[2] = auxDIBImageLoadA("textures/menu/menu3.bmp");
			menu[3] = auxDIBImageLoadA("textures/menu/menu4.bmp");
			menu[4] = auxDIBImageLoadA("textures/menu/menu5.bmp");
		}
		//меню
		{
			glGenTextures(5, &Menu[0]);
			for (int i = 0; i < 5; i++) {
				glBindTexture(GL_TEXTURE_2D, Menu[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, menu[i]->sizeX, menu[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, menu[i]->data);
			}
		}

		//арканоид
		{
			glGenTextures(1, &arkanoid);
			glBindTexture(GL_TEXTURE_2D, arkanoid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, Ltexture->sizeX, Ltexture->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Ltexture->data);

			glGenTextures(1, &ArkanoidS);
			glBindTexture(GL_TEXTURE_2D, ArkanoidS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, arkanoidS->sizeX, arkanoidS->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, arkanoidS->data);
		}

		//фон
		{
			glGenTextures(1, &Fon);
			glBindTexture(GL_TEXTURE_2D, Fon);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, fon->sizeX, fon->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, fon->data);
		}

		//левая грань
		{
			glGenTextures(1, &BorderL);
			glBindTexture(GL_TEXTURE_2D, BorderL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, borderL->sizeX, borderL->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, borderL->data);
		}

		//правая грань
		{
			glGenTextures(1, &BorderR);
			glBindTexture(GL_TEXTURE_2D, BorderR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, borderR->sizeX, borderR->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, borderR->data);
		}

		//верхняя грань
		{
			glGenTextures(1, &BorderT);
			glBindTexture(GL_TEXTURE_2D, BorderT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, borderT->sizeX, borderT->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, borderT->data);
		}

		//выстрел
		{
			glGenTextures(1, &Shoot);
			glBindTexture(GL_TEXTURE_2D, Shoot);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, shoot->sizeX, shoot->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, shoot->data);
		}

		//разрушение платформы
		{
			glGenTextures(36, &Block[0]);
			for (int i = 0; i < 36; i++) {
				glBindTexture(GL_TEXTURE_2D, Block[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, block[i]->sizeX, block[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, block[i]->data);
			}
		}

		//блоки
		{
			glGenTextures(30, &Destroy[0]);
			for (int i = 0; i < 30; i++) {
				glBindTexture(GL_TEXTURE_2D, Destroy[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, destroy[i]->sizeX, destroy[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, destroy[i]->data);
			}
		}
		//
		{
			glGenTextures(76, &Story[0]);
			for (int i = 0; i < 76; i++) {
				glBindTexture(GL_TEXTURE_2D, Story[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, story[i]->sizeX, story[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, story[i]->data);
			}
		}
		//бонусы
		{
			glGenTextures(1, &TableR);
			glBindTexture(GL_TEXTURE_2D, TableR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, tableR->sizeX, tableR->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tableR->data);

			glGenTextures(1, &TableO);
			glBindTexture(GL_TEXTURE_2D, TableO);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, tableO->sizeX, tableO->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tableO->data);

			glGenTextures(1, &TableGr);
			glBindTexture(GL_TEXTURE_2D, TableGr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, tableGr->sizeX, tableGr->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tableGr->data);

			glGenTextures(1, &TableB);
			glBindTexture(GL_TEXTURE_2D, TableB);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, tableB->sizeX, tableB->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tableB->data);

			glGenTextures(1, &TableG);
			glBindTexture(GL_TEXTURE_2D, TableG);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, tableG->sizeX, tableG->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tableG->data);
	
		}
		loaded = true;
	}
}
//просто тестовая функия перемещения мяча, можно вырезать, -200 строк
void MoveBall2() {
	bool hit = false;
	bool down=false;
	int x;
	for (int i = 0; i < speedBALL; i++) {
		//сдвинуть мяч в его направлении
		if ((ball->glued == false) && (ball->up == true)) {
			ball->x = ball->x + sin(ball->angle*pi / 180);
			ball->y = ball->y + cos(ball->angle*pi / 180);
			if (ball->y + ball->r >= 599) {
				ball->up = false;
				ball->angle = -ball->angle;
			}
			else
				if ((ball->x + ball->r >= 555) || (ball->x - ball->r <= 15)) {
					ball->angle = -ball->angle;
				}
		}
		if ((ball->glued == false) && (ball->up == false)) {
			ball->x = ball->x - sin(ball->angle*pi / 180);
			ball->y = ball->y - cos(ball->angle*pi / 180);
		}
		//столкновение с платформой
		if ((ball->y - ball->r <= 40) && (ball->y - ball->r > 35)) {
			//проверяем попадает ли шарик на один из секторов на платформе
			if ((ball->x <= platform->rx) && (ball->x >= platform->lx))
			{
				if ((ball->x >= platform->lx + 8) && (ball->x < platform->lx + 12.3125)) {
					ball->angle = -40;
					hit = true;
				}
				if ((ball->x >= platform->lx + 12.3125) && (ball->x < platform->lx + 16.6249)) {
					ball->angle = -35;
					hit = true;
				}
				if ((ball->x >= platform->lx + 16.6249) && (ball->x < platform->lx + 20.9374)) {
					ball->angle = -30;
					hit = true;
				}
				if ((ball->x >= platform->lx + 20.9374) && (ball->x < platform->lx + 25.2499)) {
					ball->angle = -25;
					hit = true;
				}
				if ((ball->x >= platform->lx + 25.2499) && (ball->x < platform->lx + 29.5624)) {
					ball->angle = -20;
					hit = true;
				}
				if ((ball->x >= platform->lx + 29.5624) && (ball->x < platform->lx + 33.8749)) {
					ball->angle = -15;
					hit = true;
				}
				if ((ball->x >= platform->lx + 33.8749) && (ball->x < platform->lx + 38.1874)) {
					ball->angle = -10;
					hit = true;
				}
				if ((ball->x >= platform->lx + 38.1874) && (ball->x < platform->lx + 45.4999)) {
					ball->angle = -5;
					hit = true;
				}

				if ((ball->x >= platform->lx + 45.4999) && (ball->x < platform->lx + 47.5)) {
					ball->angle = -2;
					hit = true;
				}
				if ((ball->x >= platform->lx + 47.5) && (ball->x < platform->lx + 52.7001)) {
					ball->angle = 2;
					hit = true;
				}


				if ((ball->x <= platform->rx - 8) && (ball->x > platform->rx - 12.3125)) {
					ball->angle = 40;
					hit = true;
				}
				if ((ball->x <= platform->rx - 12.3125) && (ball->x > platform->rx - 16.6249)) {
					ball->angle = 35;
					hit = true;
				}
				if ((ball->x <= platform->rx - 16.6249) && (ball->x > platform->rx - 20.9374)) {
					ball->angle = 30;
					hit = true;
				}
				if ((ball->x <= platform->rx - 20.9374) && (ball->x > platform->rx - 25.2499)) {
					ball->angle = 25;
					hit = true;
				}
				if ((ball->x <= platform->rx - 25.2499) && (ball->x > platform->rx - 29.5624)) {
					ball->angle = 20;
					hit = true;
				}
				if ((ball->x <= platform->rx - 29.5624) && (ball->x > platform->rx - 33.8749)) {
					ball->angle = 15;
					hit = true;
				}
				if ((ball->x <= platform->rx - 33.8749) && (ball->x > platform->rx - 38.1874)) {
					ball->angle = 10;
					hit = true;
				}
				if ((ball->x <= platform->rx - 38.1874) && (ball->x > platform->rx - 45.4999)) {
					ball->angle = 5;
					hit = true;
				}
				ball->up = true;
				if (platform->bonus == 5)
					ball->glued = true;
			}
			//проверка на попадание на угл платформы
			if ((ball->y-ball->r <= 40) && (ball->y>30)) {
				if ((ball->x + ball->r >= platform->lx - 10.0 / 256) && (ball->x < platform->lx + 8)) {
					ball->angle = -45;
					hit = true;
					ball->up = true;
					if (platform->bonus == 5)
						ball->glued = true;
				}
				if ((ball->x - ball->r - 2 <= platform->rx+ 10.0 / 256) && (ball->x > platform->rx - 8)) {
					ball->angle = 45;
					hit = true;
					ball->up = true;
					if (platform->bonus == 5)
						ball->glued = true;
				}
			}
		}
			else
					//проверка на попадание на стенку платфомы
					if (ball->y <= 30) {
						if ((ball->x + ball->r >= platform->lx-1 ) && (ball->x + ball->r <= platform->lx + 1)) {
							ball->angle = -ball->angle;
							hit = true;
						}
						else
							if ((ball->x - ball->r <= platform->rx+1) && (ball->x - ball->r >= platform->rx - 1)) {
								ball->angle = -ball->angle;
								hit = true;
							}
					}
					else
						//столкновение с стенкой
						if ((ball->x + ball->r >= 560) || (ball->x - ball->r <= 10)) {
							ball->angle = -ball->angle;
						}
		//проверка на столкновение с блоком
		if (ball->glued == false) {
			MAP *temp = block;
			bool collision = false;
			while (temp != NULL) {
				if ((ball->x + ball->r+0.5 >= temp->x) && (ball->x - ball->r-0.5<= temp->x + 40)) {
					if ((ball->y + ball->r >= temp->y - 20.5) && (ball->y + ball->r <= temp->y - 19)) {
						ball->angle = -ball->angle;
						temp->state += 1;
						ball->up = false;
						down = true;
						x = temp->x;
					}
					else
						if ((ball->y - ball->r >= temp->y - 0.5) && (ball->y - ball->r <= temp->y + 0.5)) {
							ball->angle = -ball->angle;
							temp->state += 1;
							ball->up = true;
							down = true;
							x = temp->x;
						}
						else
							if ((ball->y <= temp->y) && (ball->y >= temp->y - 20)) {
								ball->angle = -ball->angle;
								temp->state += 1;
								down = true;
								x = temp->x;
							}
				}
				if (temp->state == temp->MAXstate) {
					game->score += temp->point;
					MAP *cur = temp;
					temp = temp->next;
					delBlock(cur->index);
				}
				else
					temp = temp->next;
			}
		}
	}
	if (ball->y + ball->r <= 0) {
		platform->destroy = true;
		game->life = game->life - 1;
	}
	if (hit==true)
		if (ball->glued==false)
			soundHit->play();
	if (down == true) {
			soundHit->play();
			if (rand() % 100 <= 40) {
				BONUS *cur = new BONUS;
				cur->x = x;
				cur->y = ball->y;
				cur->bonus = 1 + rand() % 5;
				if ((game->life == 4) && (cur->bonus == 3))
					while (cur->bonus == 3)
						cur->bonus = 1 + rand() % 5;
				cur->next = NULL;
				if (bonus == NULL) {
					cur->index = 0;
					cur->back = NULL;
					bonus = cur;
				}
				else {
					BONUS *temp = bonus;
					while (temp->next != NULL)
						temp = temp->next;
					cur->back = temp;
					cur->index = temp->index + 1;
					temp->next = cur;
				}
			}
		}
}
//перемещение мяча
void MoveBall() {
	bool down = false;
	bool hit = false;
	int x;
	
	for (int i = 0; i < speedBALL; i++) {
		//перемещаем мяч
		//если не прилеплен к платформе
		if ((ball->glued == false) && (ball->up == true)) {
			ball->x = ball->x + sin(ball->angle*pi / 180);
			ball->y = ball->y + cos(ball->angle*pi / 180);
			if (ball->y + ball->r >= 599) {
				ball->up = false;
				ball->angle = -ball->angle;
			}
			else
				//прилеплен
				if ((ball->x + ball->r >= 555) || (ball->x - ball->r <= 15)) {
					ball->angle = -ball->angle;
					if (ball->x + ball->r >= 555) ball->x -= 1;
					else ball->x += 1;
				}
		}
		//______________________________
		//ну там дальше всякие проверочки прям и так все было написано
		if ((ball->glued == false) && (ball->up == false)) {
			ball->x = ball->x - sin(ball->angle*pi / 180);
			ball->y = ball->y - cos(ball->angle*pi / 180);
			//столкновение с платформой

			if ((ball->y - ball->r <= 40) && (ball->y - ball->r > 35)) {
				//проверяем попадает ли шарик на один из секторов на платформе
				if ((ball->x <= platform->rx) && (ball->x >= platform->lx))
				{
					hit = true;
					if ((ball->x >= platform->lx + 8) && (ball->x < platform->lx + 12.3125))
						ball->angle = -40;
					if ((ball->x >= platform->lx + 12.3125) && (ball->x < platform->lx + 16.6249))
						ball->angle = -35;
					if ((ball->x >= platform->lx + 16.6249) && (ball->x < platform->lx + 20.9374))
						ball->angle = -30;
					if ((ball->x >= platform->lx + 20.9374) && (ball->x < platform->lx + 25.2499))
						ball->angle = -25;
					if ((ball->x >= platform->lx + 25.2499) && (ball->x < platform->lx + 29.5624))
						ball->angle = -20;
					if ((ball->x >= platform->lx + 29.5624) && (ball->x < platform->lx + 33.8749))
						ball->angle = -15;
					if ((ball->x >= platform->lx + 33.8749) && (ball->x < platform->lx + 38.1874))
						ball->angle = -10;
					if ((ball->x >= platform->lx + 38.1874) && (ball->x < platform->lx + 45.4999))
						ball->angle = -5;

					if ((ball->x >= platform->lx + 45.4999) && (ball->x < platform->lx + 47.5))
						ball->angle = -2;
					if ((ball->x >= platform->lx + 47.5) && (ball->x < platform->lx + 52.7001))
						ball->angle = 2;


					if ((ball->x <= platform->rx - 8) && (ball->x > platform->rx - 12.3125))
						ball->angle = 40;
					if ((ball->x <= platform->rx - 12.3125) && (ball->x > platform->rx - 16.6249))
						ball->angle = 35;
					if ((ball->x <= platform->rx - 16.6249) && (ball->x > platform->rx - 20.9374))
						ball->angle = 30;
					if ((ball->x <= platform->rx - 20.9374) && (ball->x > platform->rx - 25.2499))
						ball->angle = 25;
					if ((ball->x <= platform->rx - 25.2499) && (ball->x > platform->rx - 29.5624))
						ball->angle = 20;
					if ((ball->x <= platform->rx - 29.5624) && (ball->x > platform->rx - 33.8749))
						ball->angle = 15;
					if ((ball->x <= platform->rx - 33.8749) && (ball->x > platform->rx - 38.1874))
						ball->angle = 10;
					if ((ball->x <= platform->rx - 38.1874) && (ball->x > platform->rx - 45.4999))
						ball->angle = 5;

					ball->up = true;
					if (platform->bonus == 5)
						ball->glued = true;
				}
			}
				//проверка на попадание на угл платформы
				if ((ball->y - ball->r <= 40) && (ball->y > 30)) {
					if ((ball->x + ball->r + 1 >= platform->lx - 10.0 / 256) && (ball->x < platform->lx + 8)) {
						ball->angle = -45;
						ball->up = true;
						hit = true;
						if (platform->bonus == 5)
							ball->glued = true;
					}
					if ((ball->x - ball->r - 1 <= platform->rx + 10.0 / 256) && (ball->x > platform->rx - 8)) {
						ball->angle = 45;
						hit = true;
						ball->up = true;
						if (platform->bonus == 5)
							ball->glued = true;
					}
				}
					//проверка на попадание на стенку платфомы
					if (ball->y <= 30) {
						if ((ball->x + ball->r + 0.99 <= platform->lx) && (ball->x + ball->r >= platform->lx))
							ball->angle = -ball->angle;
						else
							if ((ball->x - ball->r - 0.99 <= platform->rx) && (ball->x - ball->r >= platform->rx))
								ball->angle = -ball->angle;
					}
						//столкновение с стенкой
					if ((ball->x + ball->r >= 555) || (ball->x - ball->r <= 15)) {
							ball->angle = -ball->angle;
							if (ball->x + ball->r >= 555) ball->x -= 1;
							else ball->x += 1;
					}
		}
		//проверка на столкновение с блоком
		if (ball->glued == false) {
			MAP *temp = block;
			bool collision = false;
			while (temp != NULL) {
				if ((ball->x + ball->r + 2 >= temp->x) && (ball->x - ball->r - 2 <= temp->x + 40)) {
					if ((ball->y + ball->r >= temp->y - 22) && (ball->y + ball->r <= temp->y - 17)) {
						ball->angle = -ball->angle;
						temp->state += 1;
						ball->up = false;
						down = true;
						x = temp->x;
					}
					else
						if ((ball->y - ball->r >= temp->y - 3) && (ball->y - ball->r <= temp->y + 2)) {
							ball->angle = -ball->angle;
							temp->state += 1;
							ball->up = true;
							down = true;
							x = temp->x;
						}
						else
							if ((ball->y <= temp->y) && (ball->y >= temp->y - 20)) {
								ball->angle = -ball->angle;
								temp->state += 1;
								down = true;
								x = temp->x;
							}
				}
				if (temp->state == temp->MAXstate) {
					game->score += temp->point;
					MAP *cur = temp;
					temp = temp->next;
					delBlock(cur->index);
				}
				else
					temp = temp->next;
			}
		}
	}
	//тут всякие проверки вылетил ли мяч
	if (ball->y + ball->r <= 0) {
		platform->destroy = true;
		game->life = game->life - 1;
	}
	//ударился ли
	if (hit == true)
		if (ball->glued == false)
			soundHit->play();
	//если выпал бонус генерируем его
	if (down == true) {
		soundHit->play();
		if (rand() % 100 <= 35) {
			BONUS *cur = new BONUS;
			cur->x = x;
			cur->y = ball->y;
			cur->bonus = 1 + rand() % 5;
			if ((game->life == 4) && (cur->bonus == 3))
				while (cur->bonus == 3)
					cur->bonus = 1 + rand() % 5;
			cur->next = NULL;
			if (bonus == NULL) {
				cur->index = 0;
				cur->back = NULL;
				bonus = cur;
			}
			else {
				BONUS *temp = bonus;
				while (temp->next != NULL)
					temp = temp->next;
				cur->back = temp;
				cur->index = temp->index + 1;
				temp->next = cur;
			}
		}
	}


}
//перемещение выстрела
void MoveShoot() {
	SHOOT *temp = shoot;
	while (temp != NULL) {
		temp->y += 0.5;
		MAP *tempB = block;
		//проверка на столкновения выстрела
		while (tempB != NULL) {
			if (temp->y+16 >= tempB->y - 20) {
				if ((((temp->x +6>= tempB->x) && (temp->x+6 <= tempB->x + 40))||((temp->x <= tempB->x+40) && (temp->x  > tempB->x)))&&(temp->l == true)) {
					temp->l = false;
					tempB->state += 1;
				}
				if ((((temp->x + 68 >= tempB->x) && (temp->x + 68 <= tempB->x + 40)) || ((temp->x+68 <= tempB->x + 40) && (temp->x+68  > tempB->x))) && (temp->r == true)) {
					temp->r = false;
					tempB->state += 1;
				}
			}
			//если блок разрушен  - удалить
			if (tempB->state == tempB->MAXstate) {
				game->score += tempB->point;
				MAP *cur = tempB;
				tempB = tempB->next;
				delBlock(cur->index);
			}
			else
			  tempB = tempB->next;
		}
		//если вылетел за карту помечаем, что выстрела нет
		if (temp->y + 16 >= 599) {
			//левый
			if (temp->l == true) {
				temp->l = false;
			}
			//правый
			if (temp->r == true) {
				temp->r = false;
			}
		}
		//если нет правого и левой пули, то удалить
		if ((temp->l == false) && (temp->r == false)) {
			SHOOT *cur = temp;
			temp = temp->next;
			delShoot(cur->index);
		}
		else temp=temp->next;
	}
}
//перемещение бонуса
void MoveBonus() {
	BONUS *temp = bonus;
	while (temp != NULL) {
		temp->y = temp->y - 0.1;
		if ((temp->y - 15 <= 38) && (temp->x + 40 >= platform->lx) && (temp->x <= platform->rx)) {
			if (platform->bonus == 2)
				speedBALL = 7;
			if (platform->bonus == 4) {
				platform->lx = platform->lx + 10;
				platform->rx = platform->rx - 10;
			}
			platform->bonus = temp->bonus;
			if (platform->bonus == 3) game->life++;
			if (platform->bonus == 2) speedBALL=4;
			if (platform->bonus == 4) {
				platform->lx=platform->lx - 10;
				platform->rx=platform->rx + 10;
			}
			int index = temp->index;
			temp = temp->next;
			delBonus(index);
		}
		else
		if (temp->y - 15 <= 0) { 
			int index = temp->index;
			temp = temp->next;
			delBonus(index); 
		}
		else
		temp = temp->next;
	}
}
// ну это не мое, это надо чтобы работал WinApi с OpenGl
int SetWindowPixelFormat()
{
	int m_GLPixelIndex;
	PIXELFORMATDESCRIPTOR pfd;


	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;

	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cRedShift = 16;
	pfd.cGreenBits = 8;
	pfd.cGreenShift = 8;
	pfd.cBlueBits = 8;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 0;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 64;
	pfd.cAccumRedBits = 16;
	pfd.cAccumGreenBits = 16;
	pfd.cAccumBlueBits = 16;
	pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 32;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.bReserved = 0;
	pfd.dwLayerMask = 0;
	pfd.dwVisibleMask = 0;
	pfd.dwDamageMask = 0;



	m_GLPixelIndex = ChoosePixelFormat(hDC, &pfd);
	if (m_GLPixelIndex == 0) // Let's choose a default index.
	{
		m_GLPixelIndex = 1;
		if (DescribePixelFormat(hDC, m_GLPixelIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
			return 0;
	}


	if (SetPixelFormat(hDC, m_GLPixelIndex, &pfd) == FALSE)
		return 0;


	return 1;
}
//задать новый размер окна (-2000 строк xD)
void resize(int width, int height)
{
	glClearColor(0, 0, 0, 1);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 570, 0, 680, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	LoadTextures();
}
//отрисовка левого уголка
void leftPlatform() {
	glColor3f(1, 1, 1);
	glBegin(GL_POLYGON);
	glTexCoord2f(5.0 / 256, 7.0 / 64); glVertex3f(platform->lx - 475.0 / 256, 24.375, 0);
	glTexCoord2f(5.0 / 256, 57.0 / 64); glVertex3f(platform->lx - 475.0 / 256, 35.625, 0);
	glTexCoord2f(10.0 / 256, 57.0 / 64); glVertex3f(platform->lx, 35.625, 0);
	glTexCoord2f(10.0 / 256, 7.0 / 64); glVertex3f(platform->lx, 24.375, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 15.0 / 64); glVertex3f(platform->lx - 475.0 / 128, 28.75, 0);
	glTexCoord2f(0, 50.0 / 64); glVertex3f(platform->lx - 475.0 / 128, 31.25, 0);
	glTexCoord2f(5.0 / 256, 50.0 / 64); glVertex3f(platform->lx - 475.0 / 256, 31.25, 0);
	glTexCoord2f(5.0 / 256, 15.0 / 64); glVertex3f(platform->lx - 475.0 / 256, 28.75, 0);
	glEnd();
}
//правого
void rightPlatform() {
	glColor3f(1, 1, 1);
	glBegin(GL_POLYGON);

	glTexCoord2f(-5.0 / 256, 7.0 / 64); glVertex3f(platform->rx + 475.0 / 256, 24.375, 0);
	glTexCoord2f(-5.0 / 256, 57.0 / 64); glVertex3f(platform->rx + 475.0 / 256, 35.625, 0);
	glTexCoord2f(-10.0 / 256, 57.0 / 64); glVertex3f(platform->rx, 35.625, 0);
	glTexCoord2f(-10.0 / 256, 7.0 / 64); glVertex3f(platform->rx, 24.375, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glTexCoord2f(1, 15.0 / 64); glVertex3f(platform->rx + 475.0 / 128, 28.75, 0);
	glTexCoord2f(1, 50.0 / 64); glVertex3f(platform->rx + 475.0 / 128, 31.25, 0);
	glTexCoord2f(1 - 5.0 / 256, 50.0 / 64); glVertex3f(platform->rx + 475.0 / 256, 31.25, 0);
	glTexCoord2f(1 - 5.0 / 256, 15.0 / 64); glVertex3f(platform->rx + 475.0 / 256, 28.75, 0);
	glEnd();
}
//платформы
void DrawPlatform(unsigned int texture) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1, 1, 1);
	leftPlatform();
    glBegin(GL_POLYGON);
	glTexCoord2f(10.0 / 256, 0); glVertex3f(platform->lx, 20, 0);
	glTexCoord2f(245.0 / 256, 0); glVertex3f(platform->rx, 20, 0);
	glTexCoord2f(245.0 / 256, 1); glVertex3f(platform->rx, 40, 0);
	glTexCoord2f(10.0 / 256, 1); glVertex3f(platform->lx, 40, 0);
	glEnd();
	rightPlatform();
	glDisable(GL_TEXTURE_2D);
}
//отрисовать все блоки
void DrawBlock() {
	MAP *temp = block;
	glLineWidth(1);
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	while (temp != NULL) {
		int index=0;
		for (int i = 0; i < temp->MAXstate; i++) index=index+i;
		index = index + temp->state;
		glBindTexture(GL_TEXTURE_2D, Block[index]);
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(temp->x , temp->y, 0);
		glTexCoord2f(1,1); glVertex3f(temp->x + 40, temp->y, 0);
		glTexCoord2f(1,0); glVertex3f(temp->x + 40, temp->y - 20, 0);
		glTexCoord2f(0,0); glVertex3f(temp->x, temp->y - 20, 0);
		glEnd();
		temp = temp->next;
	}
	glDisable(GL_TEXTURE_2D);
}
//отрисовть поле
void DrawPole() {
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, BorderL);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 1); glVertex3f(0, 620, 0);
	glTexCoord2f(1, 1); glVertex3f(15, 620, 0);
	glTexCoord2f(1, 0); glVertex3f(15, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, BorderR);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 1); glVertex3f(555, 620, 0);
	glTexCoord2f(1, 1); glVertex3f(570, 620, 0);
	glTexCoord2f(1, 0); glVertex3f(570, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(555, 0, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, BorderT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 1); glVertex3f(15, 620, 0);
	glTexCoord2f(1, 1); glVertex3f(555, 620, 0);
	glTexCoord2f(1, 0); glVertex3f(555, 599, 0);
	glTexCoord2f(0, 0); glVertex3f(15, 599, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, Fon);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0); glVertex3f(15, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(15, 599, 0);
	glTexCoord2f(1, 1); glVertex3f(555, 599, 0);
	glTexCoord2f(1, 0); glVertex3f(555, 0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
//нарисовать мяч
void DrawBall() {
	glLineWidth(2);
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360; i++) {
		glVertex3f(ball->x + (ball->r)*sin(i * 3.14 / 180), ball->y + (ball->r)*cos(i * 3.14 / 180), 0);
	}
	glEnd();
	glColor3f(1, 0, 0);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++) {
		glVertex3f(ball->x + (ball->r - 1)*sin(i * 3.14 / 180), ball->y + (ball->r - 1)*cos(i * 3.14 / 180), 0);
	}
	glEnd();
}
//отрисовать выстрелы
void DrawShoot() {
	glBindTexture(GL_TEXTURE_2D, Shoot);
	glColor3f(1, 1, 1);
	SHOOT *temp = shoot;
	while (temp != NULL) {
		if (temp->l == true) {
			glColor3f(1, 1, 1);
			glBegin(GL_POLYGON);
			glVertex3f(temp->x + 2, temp->y + 16, 0);
			glVertex3f(temp->x + 4, temp->y + 16, 0);
			glVertex3f(temp->x + 4, temp->y + 14, 0);
			glVertex3f(temp->x + 2, temp->y + 14, 0);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_POLYGON);
			glTexCoord2f(0,1); glVertex3f(temp->x, temp->y + 14, 0);
			glTexCoord2f(1,1); glVertex3f(temp->x + 6, temp->y + 14, 0);
			glTexCoord2f(1,0); glVertex3f(temp->x + 6, temp->y + 2, 0);
			glTexCoord2f(0,0); glVertex3f(temp->x, temp->y + 2, 0);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glColor3f(1, 1, 0);
			glBegin(GL_POLYGON);
			glVertex3f(temp->x + 2, temp->y , 0);
			glVertex3f(temp->x + 2, temp->y + 2, 0);
			glVertex3f(temp->x + 4, temp->y + 2, 0);
			glVertex3f(temp->x + 4, temp->y , 0);
			glEnd();
		}
		if (temp->r == true) {
			glColor3f(1, 1, 1);
			glBegin(GL_POLYGON);
			glVertex3f(temp->x + 64, temp->y + 16, 0);
			glVertex3f(temp->x + 66, temp->y + 16, 0);
			glVertex3f(temp->x + 66, temp->y + 14, 0);
			glVertex3f(temp->x + 64, temp->y + 14, 0);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(temp->x+62, temp->y + 14, 0);
			glTexCoord2f(1, 1); glVertex3f(temp->x + 68, temp->y + 14, 0);
			glTexCoord2f(1, 0); glVertex3f(temp->x + 68, temp->y + 2, 0);
			glTexCoord2f(0, 0); glVertex3f(temp->x+62, temp->y + 2, 0);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glColor3f(1, 1, 0);
			glBegin(GL_POLYGON);
			glVertex3f(temp->x + 64, temp->y, 0);
			glVertex3f(temp->x + 64, temp->y + 2, 0);
			glVertex3f(temp->x + 66, temp->y + 2, 0);
			glVertex3f(temp->x + 66, temp->y, 0);
			glEnd();
		}
		temp = temp->next;
	}
	glDisable(GL_TEXTURE_2D);
}
//рисуем цифры
void DrawDigits(int x, int y,int xp,int yp,int xd) {
	char buf[12], buf2[12];
	itoa(game->score, buf, 10);
	for (int i = strlen(buf) - 1, j = 0; i >= 0; i--) {
		buf2[j] = buf[i];
		j++;
	}
	for (int i = strlen(buf); i < 6; i++) {
		buf2[i] = 0;
	}
	for (int i = 5; i >= 0; i--) {
		switch (buf2[i])
		{
		case 48:
			null(x, y, x + xp, y - yp);
			break;
		case 49:
			one(x, y, x + xp, y - yp);
			break;
		case 50:
			two(x, y, x + xp, y - yp);
			break;
		case 51:
			three(x, y, x + xp, y - yp);
			break;
		case 52:
			four(x, y, x + xp, y - yp);
			break;
		case 53:
			five(x, y, x + xp, y - yp);
			break;
		case 54:
			six(x, y, x + xp, y - yp);
			break;
		case 55:
			seven(x, y, x + xp, y - yp);
			break;
		case 56:
			eight(x, y, x + xp, y - yp);
			break;
		case 57:
			nine(x, y, x + xp, y - yp);
			break;
		default:
			null(x, y, x + xp, y - yp);
			break;
		}
		x = x + xp + xd;
	}
}
//отрисовываем счет
void DrawScore() {
	glColor3d(1, 0, 0);
	s(237.5, 675, 252.5, 650);
	c(257.5, 675, 272.5, 650);
	o(277.5, 675, 292.5, 650);
	r(297.5, 675, 312.5, 650);
	e(317.5, 675, 332.5, 650);
	glColor3d(1, 1, 1);
	DrawDigits(225,645,15,25,5);
	
}
//отрисовка мультика
bool DrawStory() {
	static int start = GetTickCount();
	static int frame = 0;
	if (frame != 85) {
		glColor3f(1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (frame<76)
			glBindTexture(GL_TEXTURE_2D, Story[frame]);
		else {
			glBindTexture(GL_TEXTURE_2D, Story[75]);
		}
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_POLYGON);
		glTexCoord2f(0,0); glVertex3f(0, 0, 0);
		glTexCoord2f(0, 1); glVertex3f(0,478,0);
		glTexCoord2f(1, 1); glVertex3f(679,478,0);
		glTexCoord2f(1, 0); glVertex3f(679,0,0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		if (GetTickCount() - start >= 100)
		{
			frame++;
			start = GetTickCount();
		}
		SwapBuffers(wglGetCurrentDC());
	}
	if (frame == 85) {
		frame = 0;
		return true;
	}
	return false;
}
//рисуем бонусы
void DrawBonus() {
	BONUS *temp = bonus;
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	while (temp != NULL) {
		switch (temp->bonus) {
		case 1: 	
			glBindTexture(GL_TEXTURE_2D, TableR);
			break;
		case 2:
			glBindTexture(GL_TEXTURE_2D, TableO);
			break;
		case 3:
			glBindTexture(GL_TEXTURE_2D, TableGr);
			break;
		case 4:
			glBindTexture(GL_TEXTURE_2D, TableB);
			break;
		case 5:
			glBindTexture(GL_TEXTURE_2D, TableG);
			break;
		}
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(temp->x, temp->y, 0);
		glTexCoord2f(1,1); glVertex3f(temp->x+40,temp->y,0);
		glTexCoord2f(1,0); glVertex3f(temp->x +40, temp->y - 15,0);
		glTexCoord2f(0,0); glVertex3f(temp->x,temp->y-15,0);
		glEnd();
		temp = temp->next;
	}
	glDisable(GL_TEXTURE_2D);
}
//думаю дальше все по названиям понятно, если что есть ПЗ
void DrawLife() {
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, arkanoid);
	glEnable(GL_TEXTURE_2D);
	if (game->life >= 1) {
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(10, 670, 0);
		glTexCoord2f(1,1); glVertex3f(40, 670, 0);
		glTexCoord2f(1,0); glVertex3f(40, 660, 0);
		glTexCoord2f(0,0); glVertex3f(10, 660, 0);
		glEnd();
	}
	if (game->life >= 2) {
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(45, 670, 0);
		glTexCoord2f(1,1); glVertex3f(75, 670, 0);
		glTexCoord2f(1,0); glVertex3f(75, 660, 0);
		glTexCoord2f(0,0); glVertex3f(45, 660, 0);
		glEnd();
	}
	if (game->life >= 3) {
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(80, 670, 0);
		glTexCoord2f(1,1); glVertex3f(110, 670, 0);
		glTexCoord2f(1,0); glVertex3f(110, 660, 0);
		glTexCoord2f(0,0); glVertex3f(80, 660, 0);
		glEnd();
	}
	if (game->life >= 4) {
		glBegin(GL_POLYGON);
		glTexCoord2f(0,1); glVertex3f(115, 670, 0);
		glTexCoord2f(1,1); glVertex3f(145, 670, 0);
		glTexCoord2f(1,0);	glVertex3f(145, 660, 0);
		glTexCoord2f(0,0); glVertex3f(115, 660, 0);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}
bool DrawGameOver(bool refresh) {
	static float x = 0;
	if (refresh == false){
		glColor3f(1, 1, 1);
		if (x < 570) {
			g(-540 + x, 550, -487.5 + x, 450);
			a(-477.5 + x, 550, -425 + x, 450);
			m(-415 + x, 550, -362.5 + x, 450);
			e(-352.5 + x, 550, -300 + x, 450);
			o(-270 + x, 550, -217.5 + x, 450);
			v(-207.5 + x, 550, -155 + x, 450);
			e(-145 + x, 550, -92.5 + x, 450);
			r(-82.5 + x, 550, -30 + x, 450);
			x += 0.2;
			return false;
		}
		else {
			g(-540 + x, 550, -487.5 + x, 450);
			a(-477.5 + x, 550, -425 + x, 450);
			m(-415 + x, 550, -362.5 + x, 450);
			e(-352.5 + x, 550, -300 + x, 450);
			o(-270 + x, 550, -217.5 + x, 450);
			v(-207.5 + x, 550, -155 + x, 450);
			e(-145 + x, 550, -92.5 + x, 450);
			r(-82.5 + x, 550, -30 + x, 450);
			glColor3d(1, 0, 0);
			DrawDigits(120, 400,45,75,10);

			return true;
		}
	}
	else x = 0;
}
bool DrawYouWin(bool refresh) {
	static float x = 0;
	if (refresh == false) {
		glColor3f(1, 0.5, 0);
		if (x < 630) {
			y(-540 + x, 550, -487.5 + x, 450);
			o(-477.5 + x, 550, -425 + x, 450);
			u(-415 + x, 550, -362.5 + x, 450);

			w(-332.5 + x, 550, -280 + x, 450);
			i(-270 + x, 550, -217.5 + x, 450);
			n(-207.5 + x, 550, -155 + x, 450);
			x += 0.2;
			return false;
		}
		else {
			y(-540 + x, 550, -487.5 + x, 450);
			o(-477.5 + x, 550, -425 + x, 450);
			u(-415 + x, 550, -362.5 + x, 450);

			w(-332.5 + x, 550, -280 + x, 450);
			i(-270 + x, 550, -217.5 + x, 450);
			n(-207.5 + x, 550, -155 + x, 450);
			glColor3d(1, 0, 0);
			DrawDigits(120, 400, 45, 75, 10);

			return true;
		}
	}
	else x = 0;
}
bool DrawPlatformDestroid() {
	static int start = GetTickCount();
	static float x = 0.0, y = 0.0;
	static int frame = 0;
	if (frame != 30) {
		if (GetTickCount() - start >= 10){
				frame++;
				start = GetTickCount();
			}
		DrawPlatform(Destroy[frame]);
	}
	if (frame == 30){
		x = 0;
		y = 0;
		frame = 0;
		return true;
	}
	return false;
}
//отрисовка игры
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3d(1, 1, 1);
	DrawPole();
	DrawLife();
	DrawBlock();
	//если начата игры
	if (game->game == true) {
		if (block == NULL) {
			//если нет блоков или закончены уровни то закончить игру
			if ((game->lvl == -1) || (game->lvl == 5)) {
				game->game = false;
				game->win = true;
			}
			else {
				//если нет перейти на следующий лвл
				game->lvl++;
				LoadMap("");
				DrawLoading();
				Sleep(1000);
				platform->destroy = false;
				platform->lx = 242.5;
				platform->rx = 327.5;
				ball->x = 285.5;
				ball->y = 50.5;
				ball->glued = true;
				while (shoot != NULL) delShoot(0);
				while (bonus != NULL) delBonus(0);
				RefreshKeyboard();
			}
		}
		//отрисовка платформы если не умер
		if ((platform->destroy == false) && (game->life != 0)) {
			keyboard();
			if (platform->bonus == 1) {
				DrawPlatform(ArkanoidS);
			}
			else
				DrawPlatform(arkanoid);
			MoveBonus();
			DrawBonus();
			MoveShoot();
			DrawShoot();
			if (GetTickCount() - STARTtime > 1) {
				STARTtime = GetTickCount();
				MoveBall();
			}
			DrawBall();
		}
		else
			//если она разрушается
			if (platform->destroy == true) {
				if (DrawPlatformDestroid() == true) {
					if (game->life == 0) {
						game->game = false;
						game->win = false;
					}
						else {
							speedBALL = 7;
							platform->destroy = false;
							platform->lx = 242.5;
							platform->rx = 327.5;
							ball->x = 285.5;
							ball->y = 50.5;
							ball->glued = true;
							platform->bonus = 0;
							while (bonus != NULL) delBonus(0);
							while (shoot != NULL) delShoot(0);
							RefreshKeyboard();
						}
				}
			}
		DrawScore();
	}
	else {
		//отрисовка окна победы
		if (game->win==true) {
			if (DrawYouWin(false) == true)
				if ((keys[VK_SPACE] == true) || (keys[VK_RETURN] == true)) {
					DrawYouWin(true);
					RefreshKeyboard();
					game->menu = true;
					while (block != NULL) delBlock(0);
					while (bonus != NULL) delBonus(0);
					while (shoot != NULL) delShoot(0);
					if (game->lvl != -1)
						SaveRecords();
				}
		}
		else
		//проигрыша
		if (DrawGameOver(false) == true)
			if ((keys[VK_SPACE] == true)||(keys[VK_RETURN] == true)) {
				soundGame->stop();
				DrawGameOver(true);
				RefreshKeyboard();
				game->menu = true;
				while (block != NULL) delBlock(0);
				while (bonus != NULL) delBonus(0);
				while (shoot != NULL) delShoot(0);
				if (game->lvl!=-1)
				SaveRecords();
			}
	}
	SwapBuffers(wglGetCurrentDC());
}
//отрисовка меню
void menu() {
	if (!soundMenu->isPlaying()) {
		soundMenu->reset();
		soundMenu->play();
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, Menu[indexMenu]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(0,680,0);
	glTexCoord2f(1, 1); glVertex3f(570,680,0);
	glTexCoord2f(1, 0); glVertex3f(570,0,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	if (keys[VK_DOWN] == true) {
		if (indexMenu == 4) indexMenu = 0;
		else indexMenu++;
		keys[VK_DOWN] = false;
	}
	if (keys[VK_UP] == true) {
		if (indexMenu == 0) indexMenu = 4;
		else indexMenu--;
		keys[VK_UP] = false;
	}
	if (keys[VK_RETURN] == true) {
		char szfile[256] = "\0";
		switch (indexMenu) {
		case 0:
			soundMenu->stop();
			soundHistory->reset();
			soundHistory->play();
			game->menu = false;
			MoveWindow(hWnd, (GetSystemMetrics(SM_CXSCREEN) - 679) / 2, (GetSystemMetrics(SM_CYSCREEN) - 478) / 2, 679, 478, true);
			while (DrawStory() == false) {}
			MoveWindow(hWnd, (GetSystemMetrics(SM_CXSCREEN) - 570) / 2, (GetSystemMetrics(SM_CYSCREEN) - 720) / 2, 570, 680, true);
			soundHistory->stop();
			soundGame->reset();
			soundGame->play();
			soundGame->setRepeat(true);
			InitializeGame("");
			break;
		case 1: 
			OPENFILENAME lpofn;
			{
				ZeroMemory(&lpofn, sizeof(OPENFILENAME));
				lpofn.lStructSize = sizeof(OPENFILENAME);
				lpofn.hwndOwner = hWnd;
				lpofn.lpstrFile = szfile;
				lpofn.nMaxFile = 256;
				lpofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
				lpofn.nFilterIndex = 0;
				lpofn.nMaxFileTitle = 0;
				lpofn.lpstrInitialDir = NULL;
				lpofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			}
			GetOpenFileName(&lpofn);
			if (lpofn.lpstrFile[0] != 0) {
				game->menu = false;
				DrawLoading();
				Sleep(500);
				InitializeGame(lpofn.lpstrFile);
			}
			break;
		case 2:
			DialogBox(_hInst, MAKEINTRESOURCE(IDD_DIALOG), 0, DlgProc);
			break;
		case 3:
			DialogBox(_hInst, MAKEINTRESOURCE(IDD_DIALOG2), 0, DlgProcRecords);
			break;
		case 4:
			PostQuitMessage(0);
			break;
		}
		keys[VK_RETURN] = false;
	}
	SwapBuffers(wglGetCurrentDC());
}
//Инициализация игры
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nCmdShow)
{

	GetModuleFileName(NULL, path, MAX_PATH);
	while (path[strlen(path)-1] != '\\') path[strlen(path)-1] = 0;
	srand(time(0));
	indexMenu = 0;
	_hInst = hInstance;
	platform = new PLATFORM;
	ball = new BALL;
	game = new GAME;
	game->menu = true;
	MSG msg;
	WNDCLASS wcl;
	{
		wcl.hInstance = hInstance;
		wcl.lpszClassName = "Arkanoid";
		wcl.lpfnWndProc = WndProc;
		wcl.style = CS_OWNDC;
		wcl.hIcon = LoadIcon(0, IDI_WINLOGO);
		wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcl.lpszMenuName = NULL;
		wcl.cbClsExtra = 0;
		wcl.cbWndExtra = 0;
		wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	RegisterClass(&wcl);
	hWnd = CreateWindow("Arkanoid", "ARKANOID", WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, (GetSystemMetrics(SM_CXSCREEN) - 570) / 2, (GetSystemMetrics(SM_CYSCREEN) - 720) / 2, 570, 680, HWND_DESKTOP, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
				return 0;
		if (game->menu == true) menu();
		else display();
	}
	return 0;
}
//обработчик игры
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	switch (msg)
	{

	case WM_CREATE: {

		hDC = GetDC(hWnd);
		SetWindowPixelFormat();
		hGLRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hGLRC);
		break;
	}

	case WM_DESTROY: {
		if (hGLRC)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRC);
		}
		ReleaseDC(hWnd, hDC);
		PostQuitMessage(0);
		break;
	}

	case WM_KEYDOWN: {
		keys[wParam] = true;
		break;
	}
	case WM_KEYUP: {
		keys[wParam] = false;
		break;
	}
	case WM_PAINT: {
		BeginPaint(hWnd, &ps);
		if (game->menu == true) menu();
		else display();
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZE: {
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
//обработчик создания карты
BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static int used = 0;
    static HMENU hMainMenu, hPOPUPMenuFile;
	static char szfile[256] = "\0";
    OPENFILENAME lpofn, spofn;
	{
		ZeroMemory(&lpofn, sizeof(OPENFILENAME));
		lpofn.lStructSize = sizeof(OPENFILENAME);
		lpofn.hwndOwner = hDlg;
		lpofn.lpstrFile = szfile;
		lpofn.nMaxFile = 256;
		lpofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
		lpofn.nFilterIndex = 0;
		lpofn.nMaxFileTitle = 0;
		lpofn.lpstrInitialDir = NULL;
		lpofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	}
	{
		ZeroMemory(&spofn, sizeof(OPENFILENAME));
		spofn.lStructSize = sizeof(OPENFILENAME);
		spofn.hwndOwner = hDlg;
		spofn.lpstrFile = szfile;
		spofn.nMaxFile = 256;
		spofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
		spofn.nFilterIndex = 0;
		spofn.nMaxFileTitle = 0;
		spofn.lpstrInitialDir = NULL;
		spofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	}
   struct MARKYP {
		int x, y;
		bool active;
		int NomberBlock;
	};
    static MARKYP *head = NULL;

	static bool ExcAdd[8];
	switch (msg) {
	//инициализация окна
	case WM_INITDIALOG: {
		head = (MARKYP*)calloc(25 * 13, sizeof(MARKYP));
		for (int i = 0; i < 25; i++) {
			for (int j = 0; j < 13; j++) {
				head[j + 13 * i].x = 25 + j * 40;
				head[j + 13 * i].y = 81 + i * 20;
			}
		}
		for (int i = 0; i < 8; i++) ExcAdd[i] = false;
		hMainMenu = CreateMenu(),
			hPOPUPMenuFile = CreatePopupMenu();
		AppendMenu(hMainMenu, MF_STRING | MF_POPUP, (UINT)hPOPUPMenuFile, "Файл");
		{
			AppendMenu(hPOPUPMenuFile, MF_STRING, CM_OPEN, "Открыть");
			AppendMenu(hPOPUPMenuFile, MF_STRING, CM_SAVEAS, "Сохранить как...");
			AppendMenu(hPOPUPMenuFile, MF_SEPARATOR, 0, "");
			AppendMenu(hPOPUPMenuFile, MF_STRING, CM_EXIT, "Выход");
		}
		SetMenu(hDlg, hMainMenu);
		SetMenu(hDlg, hPOPUPMenuFile);
		used = 0;
		strcpy(szfile ,"\0");
		return TRUE;
	}
	case WM_CLOSE: {	
		free(head);
		used = 0;
		EndDialog(hDlg, 0);
		return TRUE;
	}
	//нажатие лефой кнопи мыши
	case WM_LBUTTONDOWN: {
		int x = LOWORD(lParam), y = HIWORD(lParam);
		//выбираем блок которым будем рисовать
		if ((x >= 575) && (x <= 615) && (y >= 20) && (y <= 40)) {
			for (int i = 0; i < 8; i++)
			ExcAdd[i] = false; //снимаем остальные кубики
			ExcAdd[0] = true; //помечаем выбранный
			InvalidateRect(hDlg, 0, TRUE);
		}
		if ((x >= 635) && (x <= 675) && (y >= 20) && (y <= 40)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[1] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}
		if ((x >= 695) && (x <= 735) && (y >= 20) && (y <= 40)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[2] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}

		if ((x >= 575) && (x <= 615) && (y >= 55) && (y <= 75)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[3] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}
		if ((x >= 635) && (x <= 675) && (y >= 55) && (y <= 75)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[4] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}
		if ((x >= 695) && (x <= 735) && (y >= 55) && (y <= 75)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[5] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}

		if ((x >= 575) && (x <= 615) && (y >= 90) && (y <= 110)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[6] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}
		if ((x >= 635) && (x <= 675) && (y >= 90) && (y <= 110)) {
			for (int i = 0; i < 8; i++)
				ExcAdd[i] = false;
			ExcAdd[7] = true;
			InvalidateRect(hDlg, 0, TRUE);
		}
		//____________________________________
		//нажатие на карты, если выбран блок то зарисовать его выбранным
		for (int i = 0; i < 25 * 13; i++) {
			if ((y < head[i].y + 20) && (y > head[i].y) && (x > head[i].x) && (x < head[i].x + 40)) {
				if (head[i].active == false) {
					head[i].active = true;
					for (int j=0;j<8;j++)
						if (ExcAdd[j] == true) {
							head[i].NomberBlock = j + 1;
							used++;
							break;
						}
				}
				else { head[i].active = false; 
				used--;
				head[i].NomberBlock = 0;
				}

				InvalidateRect(hDlg, 0, TRUE);
				break;
			}
		}
		return TRUE;
	}
	case WM_COMMAND: {
		//нажатие на открыть, происходит загрузка карты
		if (LOWORD(wParam) == CM_OPEN) {
			used = 0;
			GetOpenFileName(&lpofn);
			if (lpofn.lpstrFile[0] != 0) {
				char pass[17];
				pass[16] = 0;
				int i = 0;
				ifstream in(lpofn.lpstrFile, ios::binary | ios::in);
				MAP_SAVE *tempMAP = (MAP_SAVE*)malloc(0);
				in.read(pass, 16);
				if (strcmp(pass, "6Q91Ne3MQ2foqnD4") == 0) {
					for (int m = 0; m < 25 * 13; m++) {
						head[m].active = false;
						head[m].NomberBlock = 0;
					}
					while (true) {
						MAP_SAVE *cur = new MAP_SAVE;
						in.read((char*)&*cur, sizeof(MAP_SAVE));
						if (in.eof()) { delete cur; break; }
						i++;
						tempMAP = (MAP_SAVE*)realloc(tempMAP, i * sizeof(MAP_SAVE));
						tempMAP[i - 1].index = cur->index;
						tempMAP[i - 1].MAXstate = cur->MAXstate;
						tempMAP[i - 1].point = cur->point;
						tempMAP[i - 1].state = cur->state;
						tempMAP[i - 1].x = cur->x;
						tempMAP[i - 1].y = cur->y;
					}
					printf("%d ", i);
					for (int j = 0; j < i; j++) {
						for (int m = 0; m < 25 * 13; m++) {
							if ((head[m].x == tempMAP[j].x) && (head[m].y == 680 - tempMAP[j].y)) {
								printf("L");
								head[m].active = true;
								head[m].NomberBlock = tempMAP[j].MAXstate;
								used++;
							}
						}
					}
					free(tempMAP);
					in.close();
					InvalidateRect(hDlg, NULL, TRUE);
				}
				else {
					MessageBox(0, "Файл не поддерживается", "Ошибка", MB_OK);
					break;
				}
			}
		}
		//сохранение карты
		if (LOWORD(wParam) == CM_SAVEAS) {
			MAP_SAVE *tempMAP = (MAP_SAVE*)malloc(used*sizeof(MAP_SAVE));
			int j = 0;
			for (int i = 0; i < 25 * 13; i++) {
				if (head[i].active == true) {
					tempMAP[j].x = head[i].x;
					tempMAP[j].y = 680-head[i].y;
					switch (head[i].NomberBlock)
					{
					case 1:
						tempMAP[j].MAXstate = 1;
						tempMAP[j].point = 50;
						break;
					case 2:
						tempMAP[j].MAXstate = 2;
						tempMAP[j].point = 60;
						break;
					case 3:
						tempMAP[j].MAXstate = 3;
						tempMAP[j].point = 70;
						break;
					case 4:
						tempMAP[j].MAXstate = 4;
						tempMAP[j].point = 80;
						break;
					case 5:
						tempMAP[j].MAXstate = 5;
						tempMAP[j].point = 90;
						break;
					case 6:
						tempMAP[j].MAXstate = 6;
						tempMAP[j].point = 100;
						break;
					case 7:
						tempMAP[j].MAXstate = 7;
						tempMAP[j].point = 110;
						break;
					case 8:
						tempMAP[j].MAXstate = 8;
						tempMAP[j].point = 120;
						break;
					}
					tempMAP[j].state = 0;
					tempMAP->index = j;
					j++;
				}
			}
			char pas[] = "6Q91Ne3MQ2foqnD4";
			GetSaveFileName(&spofn);
			ofstream f(spofn.lpstrFile, ios::binary | ios::out);
			f.write(pas, sizeof(char)*strlen(pas));
			for(int i=0;i<used;i++){
				f.write((char*)&tempMAP[i], sizeof(MAP_SAVE));
			}
			EnableMenuItem(hPOPUPMenuFile, CM_SAVE, MF_ENABLED);
			f.close();
		}
		if (LOWORD(wParam) == DLG_DELETE) {
			for (int m = 0; m < 25 * 13; m++) {
				head[m].active = false;
				head[m].NomberBlock = 0;
			}
			InvalidateRect(hDlg, NULL, TRUE);
		}
		return TRUE;
	}
	case WM_SIZE: {
		
		return TRUE;
	}
				  //отрисовка содержимого окна
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);
		RECT rect;
		//рисование поля и блоков  выбора
		{
		rect.bottom = 680;
		rect.top = 0;
		rect.left = 0;
		rect.right = 800;
		FillRect(hdc, &rect, fon_brush);

		rect.bottom = 620;
		rect.top = 20;
		rect.left = 15;
		rect.right = 555;
		FrameRect(hdc, &rect, black_brush);
		rect.bottom = 40;
		rect.top = 20;
		rect.left = 575;
		rect.right = 615;

		FillRect(hdc, &rect, grey_brush);
		rect.bottom = 40;
		rect.top = 20;
		rect.left = 635;
		rect.right = 675;
		FillRect(hdc, &rect, orange_brush);
		rect.bottom = 40;
		rect.top = 20;
		rect.left = 695;
		rect.right = 735;
		FillRect(hdc, &rect, blue_brush);

		rect.bottom = 75;
		rect.top = 55;
		rect.left = 575;
		rect.right = 615;
		FillRect(hdc, &rect, green_brush);
		rect.bottom = 75;
		rect.top = 55;
		rect.left = 635;
		rect.right = 675;
		FillRect(hdc, &rect, red_brush);
		rect.bottom = 75;
		rect.top = 55;
		rect.left = 695;
		rect.right = 735;
		FillRect(hdc, &rect, dark_blue_brush);

		rect.bottom = 110;
		rect.top = 90;
		rect.left = 575;
		rect.right = 615;
		FillRect(hdc, &rect, purple_brush);
		rect.bottom = 110;
		rect.top = 90;
		rect.left = 635;
		rect.right = 675;
		FillRect(hdc, &rect, yellow_brush);
		
		}
		//рисование подсвечивания 
		{
			for (int i = 0; i < 3; i++)
				if (ExcAdd[i] == true) {
					rect.bottom = 41;
					rect.top = 19;
					rect.left = 574+i*60;
					rect.right = 616+i*60;
					FrameRect(hdc, &rect, while_brush);
				}
			for (int i = 3; i < 6; i++)
				if (ExcAdd[i] == true) {
					rect.bottom = 76;
					rect.top = 54;
					rect.left = 574 + (i-3) * 60;
					rect.right = 616 + (i - 3) * 60;
					FrameRect(hdc, &rect, while_brush);
				}
			for (int i = 6; i < 8; i++)
				if (ExcAdd[i] == true) {
					rect.bottom = 111;
					rect.top = 89;
					rect.left = 574 + (i-6) * 60;
					rect.right = 616 + (i - 6) * 60;
					FrameRect(hdc, &rect, while_brush);
				}

		}
		//рисование блоков
		{
			for (int i = 0; i < 25 * 13; i++) {
				rect.bottom = head[i].y + 20;
				rect.top = head[i].y;
				rect.right = head[i].x + 40;
				rect.left = head[i].x;
				FrameRect(hdc, &rect, black_brush);
				rect.bottom--;
				rect.top++;
				rect.right--;
				rect.left++;
					switch (head[i].NomberBlock)
					{
					case 1:
						FillRect(hdc, &rect, grey_brush);
						break;
					case 2:
						FillRect(hdc, &rect, orange_brush);
						break;
					case 3:
						FillRect(hdc, &rect, blue_brush);
						break;
					case 4:
						FillRect(hdc, &rect, green_brush);
						break;
					case 5:
						FillRect(hdc, &rect, red_brush);
						break;
					case 6:
						FillRect(hdc, &rect, dark_blue_brush);
						break;
					case 7:
						FillRect(hdc, &rect, purple_brush);
						break;
					case 8:
						FillRect(hdc, &rect, yellow_brush);
						break;
					}
				}
		}
		EndPaint(hDlg, &ps);
		return TRUE;
	}
	}
	return FALSE;
}
//обработчик добавить рекорд
BOOL CALLBACK DlgProcADD(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT1);
	switch (msg) {
	case WM_INITDIALOG: {
		SendMessage(hEdit, EM_LIMITTEXT, 19, 0);
		return TRUE;
	}
	case WM_LBUTTONDOWN: {
		return TRUE;
	}
	case WM_CLOSE: {
		if (NickName[0] == 0 || NickName[0] == ' ') {
			MessageBox(0, "Заполните поле", "Ошибка", MB_OK);
		}
		else EndDialog(hDlg, 0);
		return TRUE;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDOK) {
			GetWindowText(hEdit, NickName, 19);
			if (NickName[0] == 0 || NickName[0] == ' ') {
				MessageBox(0, "Заполните поле", "Ошибка", MB_OK);
			}
			else EndDialog(hDlg, 0);
		}
		return TRUE;
	}
	case WM_SIZE: {

		return TRUE;
	}
	}
	return FALSE;
}
//отображение рекордов
BOOL CALLBACK DlgProcRecords(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hNickName[10], hScore[10]; 
	hNickName[0]=GetDlgItem(hDlg, IDC_NICKNAME1);
	hNickName[1] = GetDlgItem(hDlg, IDC_NICKNAME2);
	hNickName[2] = GetDlgItem(hDlg, IDC_NICKNAME3);
	hNickName[3] = GetDlgItem(hDlg, IDC_NICKNAME4);
	hNickName[4] = GetDlgItem(hDlg, IDC_NICKNAME5);
	hNickName[5] = GetDlgItem(hDlg, IDC_NICKNAME6);
	hNickName[6] = GetDlgItem(hDlg, IDC_NICKNAME7);
	hNickName[7] = GetDlgItem(hDlg, IDC_NICKNAME8);
	hNickName[8] = GetDlgItem(hDlg, IDC_NICKNAME9);
	hNickName[9] = GetDlgItem(hDlg, IDC_NICKNAME10);
	hScore[0]= GetDlgItem(hDlg, IDC_SCORE1);
	hScore[1] = GetDlgItem(hDlg, IDC_SCORE2);
	hScore[2] = GetDlgItem(hDlg, IDC_SCORE3);
	hScore[3] = GetDlgItem(hDlg, IDC_SCORE4);
	hScore[4] = GetDlgItem(hDlg, IDC_SCORE5);
	hScore[5] = GetDlgItem(hDlg, IDC_SCORE6);
	hScore[6] = GetDlgItem(hDlg, IDC_SCORE7);
	hScore[7] = GetDlgItem(hDlg, IDC_SCORE8);
	hScore[8] = GetDlgItem(hDlg, IDC_SCORE9);
	hScore[9] = GetDlgItem(hDlg, IDC_SCORE10);
	switch (msg) {
	case WM_INITDIALOG: {
		LoadRecords();
		for (int i = 0; i < 10; i++) {
			if (records[i].active == true) {
				char buf[10];
				itoa(records[i].point, buf, 10);
				SetWindowText(hNickName[i], records[i].NickName);
				SetWindowText(hScore[i], buf);
			}
			else {
				SetWindowText(hNickName[i], "");
				SetWindowText(hScore[i], "");
			}
		}
		return TRUE;
	}
	case WM_CLOSE: {
		EndDialog(hDlg, 0);
		return TRUE;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, 0);
		}
		return TRUE;
	}
	case WM_SIZE: {

		return TRUE;
	}
	}
	return FALSE;
}
void delBlock(int index) {
	if (index != -1) {
		MAP* temp = block;
		if (index == 0) {
			block = block->next;
			if (block != NULL) block->back = NULL;
			MAP* tmp = block;
			while (tmp != NULL) {
				tmp->index = tmp->index - 1;
				tmp = tmp->next;
			}
			delete(temp);
		}
		else {
			while (temp->index != index) temp = temp->next;
			temp->back->next = temp->next;
			if (temp->next != NULL) temp->next->back = temp->back;
			MAP* tmp = temp;
			temp = temp->next;
			delete(tmp);
			while (temp != NULL) {
				temp->index = temp->index - 1;
				temp = temp->next;
			}
		}
	}
}
void delShoot(int index) {
	if (index != -1) {
		SHOOT* temp = shoot;
		if (index == 0) {
			shoot = shoot->next;
			if (shoot != NULL) shoot->back = NULL;
			SHOOT* tmp = shoot;
			while (tmp != NULL) {
				tmp->index = tmp->index - 1;
				tmp = tmp->next;
			}
			delete(temp);
		}
		else {
			while (temp->index != index) temp = temp->next;
			temp->back->next = temp->next;
			if (temp->next != NULL) temp->next->back = temp->back;
			SHOOT* tmp = temp;
			temp = temp->next;
			delete(tmp);
			while (temp != NULL) {
				temp->index = temp->index - 1;
				temp = temp->next;
			}
		}
	}
}
void delBonus(int index) {
	if (index != -1) {
		BONUS* temp = bonus;
		if (index == 0) {
			bonus = bonus->next;
			if (bonus != NULL) bonus->back = NULL;
			BONUS* tmp = bonus;
			while (tmp != NULL) {
				tmp->index = tmp->index - 1;
				tmp = tmp->next;
			}
			delete(temp);
		}
		else {
			while (temp->index != index) temp = temp->next;
			temp->back->next = temp->next;
			if (temp->next != NULL) temp->next->back = temp->back;
			BONUS* tmp = temp;
			temp = temp->next;
			delete(tmp);
			while (temp != NULL) {
				temp->index = temp->index - 1;
				temp = temp->next;
			}
		}
	}
}