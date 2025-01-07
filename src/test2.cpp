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

IMAGE imgBg;                      // 表示背景图片
IMAGE imgBar;                     // 表示进度条图片
IMAGE imgCards[ZHI_WU_NUM];       // 表示卡牌图片
IMAGE *imgPlants[ZHI_WU_NUM][20]; // 表示植物图片

int curX, curY; // 当前鼠标位置（选中的植物，在拖动过程中的位置）
int curZhiwu;   // 0：未选中植物，1-8：当前选择的植物编号

struct zhiwu
{
    int type;       // 0:未选中植物，1-8：当前选择的植物编号
    int frameIndex; // 序列帧的序号
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

int sunshine;

bool fileExists(const char *filename) // 判断文件是否存在
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        return false;
    }
    fclose(fp);
    return true;
}

void gameInit() // 窗口初始化函数
{
    // 加载背景图片和进度条图片
    loadimage(&imgBg, "res/Map/bg.jpg");
    loadimage(&imgBar, "res/Sidebar/bar5.png");

    // 初始化图像植物数组为0
    memset(imgPlants, 0, sizeof(imgPlants));
    // 初始化地图数组为0
    memset(map, 0, sizeof(map));

    // 加载卡牌图片
    for (int i = 0; i < ZHI_WU_NUM; i++)
    {
        // 生成卡片图片路径
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

    // 加载阳光弹射图片
    memset(balls, 0, sizeof(balls));
    for (int i = 0; i < 29; i++)
    {
        char path[100];
        sprintf(path, "res/Sunshine/%d.png", i + 1);
        loadimage(&imgSunshineBall[i], path);
    }
    srand(time(NULL)); // 初始化随机数种子

    centerWindow(WIN_WIDTH, WIN_HEIGHT); // 居中窗口

    LOGFONT f;
    printf("创建字体\n");
    gettextstyle(&f);

    f.lfHeight = 30;
    f.lfWidth = 15;
    strcpy(f.lfFaceName, "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY; // 抗锯齿效果
    settextstyle(&f);
    setbkmode(TRANSPARENT); // 设置背景透明
    setcolor(BLACK);
}

void updateWindow() // 图像加载函数
{
    BeginBatchDraw(); // 开始缓冲

    // 绘制背景图片
    putimage(0, 0, &imgBg);
    putimagePNG(250, 0, &imgBar);

    // 绘制卡牌图片
    for (int i = 0; i < ZHI_WU_NUM; i++)
    {
        putimage(338 + i * 65, 6, &imgCards[i]);
    }

    // 遍历地图的每一行
    for (int i = 0; i < 3; i++)
    {
        // 遍历地图的每一列
        for (int j = 0; j < 9; j++)
        {
            // 如果当前位置有植物
            if (map[i][j].type > 0)
            {
                // 获取植物的图像
                IMAGE *img = imgPlants[map[i][j].type - 1][map[i][j].frameIndex];
                // 在地图上绘制植物图像
                putimagePNG(256 + j * 81, 179 + i * 102, img);
            }
        }
    }

    // 渲染移动过程中的植物图片
    if (curZhiwu > 0)
    {
        IMAGE *img = imgPlants[curZhiwu - 1][0];
        putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
    }

    // 绘制阳光弹射
    for (int i = 0; i < 10; i++)
    {
        if (balls[i].used)
        {
            IMAGE *img = &imgSunshineBall[balls[i].frameIndex];
            putimagePNG(balls[i].x, balls[i].y, img);
        }
    }

    // 绘制阳光值
    char str[8];
    sprintf(str, "%d", sunshine);
    outtextxy(276, 67, str); // 输出阳光值

    EndBatchDraw(); // 结束缓冲
}

void collectSunshine(ExMessage *msg) // 收集阳光弹射函数
{
    int w = imgSunshineBall[0].getwidth(); // 移到循环外部
    int h = imgSunshineBall[0].getheight(); // 移到循环外部

    for (int i = 0; i < 10; i++)
    {
        if (balls[i].used && msg->x >= balls[i].x && msg->x <= balls[i].x + w && msg->y >= balls[i].y && msg->y <= balls[i].y + h)
        {
            balls[i].used = false;
            sunshine += 25;
            // 异步播放音乐以避免阻塞主线程
            mciSendString("play res/Sounds/sunshine.mp3 alias sunshine", NULL, 0, NULL);
            break;
        }
    }
}

void userClick() // 用户点击处理函数
{
    ExMessage msg;
    static int status = 0;
    if (peekmessage(&msg))
    {
        if (msg.message == WM_LBUTTONDOWN) // 鼠标左键按下
        {
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_NUM && msg.y > 6 && msg.y < 6 + 65)
            {
                // 计算植物索引值
                int zhiwuIndex = (msg.x - 338) / 65;
                // 根据索引值设置当前植物
                curZhiwu = zhiwuIndex + 1;
                status = 1;
            }
            else
            {
                collectSunshine(&msg);
            }
        }
        else if (msg.message == WM_MOUSEMOVE) // 鼠标移动
        {
            curX = msg.x;
            curY = msg.y;
        }
        else if (msg.message == WM_LBUTTONUP) // 鼠标左键松开
        {
            if (msg.x > 256 && msg.y > 179 && msg.y < 489)
            {
                // 计算地图坐标
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

void createSunshine() // 创建随机阳光函数
{
    static int count = 0;
    static int lastTime = 400;
    static int ballMax = sizeof(balls) / sizeof(balls[0]); // 计算阳光弹射数组的最大长度

    count++;
    if (count >= lastTime)
    {
        lastTime = rand() % 200 + 200;
        count = 0;

        for (int i = 0; i < ballMax; i++)
        {
            if (!balls[i].used){
                balls[i].used = true;
                balls[i].frameIndex = 0;
                balls[i].x = rand() % (WIN_WIDTH - 360) + 260;
                balls[i].y = 60;
                balls[i].destY = (rand() % 4) * 90 + 200;
                balls[i].timer = 0;
                // printf("创建阳光弹射成功\n");
                break;
            }
        }
    }
}

void updateSunshine() // 更新阳光弹射函数
{
    for (int i = 0; i < 10; i++)
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

void updateGame() // 游戏逻辑函数
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

void startMainMenu() // 启动界面函数
{
    IMAGE imgStartBg, imgMenu1, imgMenu2;
    loadimage(&imgStartBg, "res/Screen/MainMenu.png");
    loadimage(&imgMenu1, "res/Screen/Menu1.png");
    loadimage(&imgMenu2, "res/Screen/Menu2.png");
    int flag = 0;
    while (1)
    {
        BeginBatchDraw();
        // 绘制开始背景图像
        putimage(0, 0, &imgStartBg);
        // 根据标志位绘制不同的菜单图像
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
    while (1)
    {
        userClick();
        timer += getDelay();
        if (timer > 20)
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
        Sleep(10);
    }

    // 停止所有 MIDI 播放
    mciSendString("stop sunshine", NULL, 0, NULL);

    printf("press any key to exit\n");
    getch();      // 等待用户按键
    closegraph(); // 关闭图形窗口
    return 0;
}
