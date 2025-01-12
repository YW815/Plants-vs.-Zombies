#include <stdio.h>
#include <graphics.h>
#include <easyx.h>
#include <conio.h>
#include <windows.h>
#include <string>
#include <time.h>
#include "centerWindow.h"
#include "tools.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define WIN_WIDTH 1100
#define WIN_HEIGHT 600

enum
{
    ZHI_WU_1,
    ZHI_WU_2,
    ZHI_WU_3,
    ZHI_WU_4,
    ZHI_WU_5,
    ZHI_WU_6,
    ZHI_WU_7,
    ZHI_WU_8,
    ZHI_WU_NUM
};

IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCards[ZHI_WU_NUM];
IMAGE *imgPlants[ZHI_WU_NUM][20];

int curX, curY;
int curZhiwu;

struct zhiwu
{
    int type;
    int frameIndex;
};

struct zhiwu map[3][9];

struct sunshineBall
{
    int x, y;
    int frameIndex;
    int destY;
    bool used;
    int timer;
};
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int ballMax = sizeof(balls) / sizeof(balls[0]);

int sunshine;

bool fileExists(const char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        return false;
    }
    fclose(fp);
    return true;
}
void gameInit()
{
    loadimage(&imgBg, "res/Map/bg.jpg");
    loadimage(&imgBar, "res/Sidebar/bar5.png");

    memset(imgPlants, 0, sizeof(imgPlants));

    memset(map, 0, sizeof(map));

    for (int i = 0; i < ZHI_WU_NUM; i++)
    {

        char path[100];
        sprintf(path, "res/Cards/card_%d.png", i + 1);
        loadimage(&imgCards[i], path);

        for (int j = 0; j < 20; j++)
        {
            sprintf(path, "res/Plants/%d/%d.png", i, j + 1);
            if (fileExists(path))
            {
                imgPlants[i][j] = new IMAGE;
                loadimage(imgPlants[i][j], path);
            }
            else
                break;
        }
    }

    curZhiwu = 0;
    sunshine = 50;

    memset(balls, 0, sizeof(balls));
    for (int i = 0; i < 29; i++)
    {
        char path[100];
        sprintf(path, "res/Sunshine/%d.png", i + 1);
        loadimage(&imgSunshineBall[i], path);
    }
    srand(time(NULL));

    centerWindow(WIN_WIDTH, WIN_HEIGHT);

    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 30;
    f.lfWidth = 15;
    strcpy(f.lfFaceName, "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    setbkmode(TRANSPARENT);
    setcolor(BLACK);

    // 预加载音乐
    // mciSendString("open res/Sounds/sunshine.mp3", NULL, 0, NULL);
}

void updateWindow()
{
    BeginBatchDraw();

    putimage(0, 0, &imgBg);
    putimagePNG(250, 0, &imgBar);

    for (int i = 0; i < ZHI_WU_NUM; i++)
    {
        putimage(338 + i * 65, 6, &imgCards[i]);
    }

    for (int i = 0; i < 3; i++)
    {

        for (int j = 0; j < 9; j++)
        {
            if (map[i][j].type > 0)
            {

                IMAGE *img = imgPlants[map[i][j].type - 1][map[i][j].frameIndex];

                putimagePNG(256 + j * 81, 179 + i * 102, img);
            }
        }
    }

    if (curZhiwu > 0)
    {
        IMAGE *img = imgPlants[curZhiwu - 1][0];
        putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
    }

    for (int i = 0; i < 10; i++)
    {
        if (balls[i].used)
        {
            IMAGE *img = &imgSunshineBall[balls[i].frameIndex];
            putimagePNG(balls[i].x, balls[i].y, img);
        }
    }

    char str[8];
    sprintf(str, "%d", sunshine);
    outtextxy(276, 67, str);

    EndBatchDraw();
}

void collectSunshine(ExMessage *msg)
{
    int ballMax = sizeof(balls) / sizeof(balls[0]);
    int w = imgSunshineBall[0].getwidth();
    int h = imgSunshineBall[0].getheight();
    int xMin, xMax, yMin, yMax;
    for (int i = 0; i < ballMax; i++)
    {
        if (balls[i].used)
        {
            xMin = balls[i].x;
            xMax = balls[i].x + w;
            yMin = balls[i].y;
            yMax = balls[i].y + h;
            if (msg->x >= xMin && msg->x <= xMax && msg->y >= yMin && msg->y <= yMax)
            {
                balls[i].used = false;
                sunshine += 25;
                mciSendString("play res/Sounds/sunshine.mp3", NULL, 0, NULL);
                break;
            }
        }
    }
}

void userClick()
{
    ExMessage msg;

    static int status = 0;
    if (peekmessage(&msg))
    {
        if (msg.message == WM_LBUTTONDOWN)
        {
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_NUM && msg.y > 6 && msg.y < 6 + 65)
            {

                int zhiwuIndex = (msg.x - 338) / 65;

                curZhiwu = zhiwuIndex + 1;
                status = 1;
            }
            else
            {
                collectSunshine(&msg);
            }
        }
        else if (msg.message == WM_MOUSEMOVE)
        {
            curX = msg.x;
            curY = msg.y;
        }
        else if (msg.message == WM_LBUTTONUP)
        {
            if (msg.x > 256 && msg.y > 179 && msg.y < 489)
            {

                int row = (msg.y - 179) / 102;
                int col = (msg.x - 256) / 81;

                if (map[row][col].type == 0)
                {
                    map[row][col].type = curZhiwu;
                    map[row][col].frameIndex = 0;
                }
            }
            curZhiwu = 0;
            status = 0;
        }
    }
}

void createSunshine()
{
    static int count = 0;
    static int lastTime = 400;
    static int ballMax = sizeof(balls) / sizeof(balls[0]);
    count++;
    if (count >= lastTime)
    {
        lastTime = rand() % 200 + 200;
        count = 0;
        for (int i = 0; i < ballMax; i++)
        {
            if (!balls[i].used)
            {
                balls[i].used = true;
                balls[i].frameIndex = 0;
                balls[i].x = rand() % (WIN_WIDTH - 360) + 260;
                balls[i].y = 60;
                balls[i].destY = (rand() % 4) * 90 + 200;
                balls[i].timer = 0;
                break;
            }
        }
    }
}

void updateSunshine()
{
    for (int i = 0; i < ballMax; i++)
    {
        if (balls[i].used)
        {
            balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
            if (balls[i].timer == 0)
            {
                balls[i].y += 2;
            }
            if (balls[i].y >= balls[i].destY)
            {
                balls[i].timer++;
                if (balls[i].timer > 100)
                {
                    balls[i].used = false;
                }
            }
        }
    }
}

void updateGame()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (map[i][j].type > 0)
            {
                map[i][j].frameIndex++;
                int zhiwuIndex = map[i][j].type - 1;
                int FrameNum = map[i][j].frameIndex;
                if (imgPlants[zhiwuIndex][FrameNum] == NULL)
                {
                    map[i][j].frameIndex = 0;
                }
            }
        }
    }

    createSunshine();
    updateSunshine();
}

void startMainMenu()
{
    IMAGE imgStartBg, imgMenu1, imgMenu2;
    loadimage(&imgStartBg, "res/Screen/MainMenu.png");
    loadimage(&imgMenu1, "res/Screen/Menu1.png");
    loadimage(&imgMenu2, "res/Screen/Menu2.png");
    int flag = 0;
    while (1)
    {
        BeginBatchDraw();

        putimage(0, 0, &imgStartBg);

        putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

        ExMessage msg;

        if (peekmessage(&msg))
        {

            if (msg.message == WM_LBUTTONDOWN)
            {

                if (msg.x > 474 && msg.x < 474 + 300 && msg.y > 75 && msg.y < 75 + 140)
                {

                    flag = 1;
                }
            }

            else if (msg.message == WM_LBUTTONUP && flag)
            {

                return;
            }
        }

        EndBatchDraw();
    }
}

int main(void)
{
    gameInit();

    startMainMenu();

    int timer = 0;
    bool flag = true;
    int fps =20;
    while (1)
    {
        userClick();
        timer += getDelay();
        if (timer > fps)
        {
            flag = true;
            timer = 0;
        }

        if (flag)
        {
            flag = false;
            updateWindow();
            updateGame();
        }
    }
    printf("press any key to exit\n");
    getch();
    closegraph();
    return 0;
}