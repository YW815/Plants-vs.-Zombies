#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <string>
#include <time.h>
#include "centerWindow.h"
#include "tools.h"

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
        // memset(imgPlants[i], 0, sizeof(imgPlants[i]));
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

    memset(balls, 0, sizeof(balls)); // 加载阳光弹射图片
    for (int i = 0; i < 29; i++)
    {
        char path[100];
        sprintf(path, "res/Sunshine/%d.png", i + 1);
        loadimage(&imgSunshineBall[i], path);
    }
    srand(time(NULL)); // 初始化随机数种子
    // HWND hWnd = initgraph(WIN_WIDTH, WIN_HEIGHT,1);// 创建游戏的窗口
    centerWindow(WIN_WIDTH, WIN_HEIGHT); // 居中窗口
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
    

    EndBatchDraw(); // 结束缓冲
}

void userClick()// 用户点击处理函数
{
    ExMessage msg;
    // peekmessage(msg);// 读取消息队列，防止消息丢失
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
                // printf("row:%d,col:%d\n", row, col); // 测试植物安放位置
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
    count++;
    if (count >= lastTime)
    {
        lastTime = rand() % 200 + 200;
        count = 0;
        int ballMax = sizeof(balls) / sizeof(balls[0]); // 计算阳光弹射数组的最大长度
        int i;
        for (i = 0; i < ballMax && balls[i].used == 0; i++);
        if (i >= ballMax)
            return;
        balls[i].used = true;
        balls[i].frameIndex = 0;
        balls[i].x = rand() % (WIN_WIDTH - 260) + 260;
        balls[i].y = 60;
        balls[i].destY = (rand() % 4) * 90 + 200;
        balls[i].timer = 0;
    }
}

void updateSunshine() // 更新阳光弹射函数
{
    int ballMax = sizeof(balls) / sizeof(balls[0]);
    for (int i = 0; i < ballMax; i++)
    {
        if (balls[i].used)
        {
            balls[i].frameIndex =(balls[i].frameIndex + 1) % 29;
            if(balls[i].timer == 0) 
            {
                balls[i].y += 2;
            }
            if (balls[i].y >= balls[i].destY)
            {
                balls[i].timer ++;
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
        // Sleep(10);
        ExMessage msg;
        // 检查是否有消息待处理
        if (peekmessage(&msg))
        {
            // 如果消息是鼠标左键按下
            if (msg.message == WM_LBUTTONDOWN)
            {
                // 检查鼠标位置是否在菜单按钮范围内
                if (msg.x > 474 && msg.x < 474 + 300 && msg.y > 75 && msg.y < 75 + 140)
                {
                    // 设置标志位为1
                    flag = 1;
                    // EndBatchDraw();
                }
            }
            // 如果消息是鼠标左键释放且标志位为1
            else if (msg.message == WM_LBUTTONUP && flag)
            {
                // 返回结束处理
                return;
            }
        }
        // 结束批量绘制
        EndBatchDraw();
    }
}
// 游戏主函数
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
        // 如果计时器超过80，设置标志为真并将计时器重置为0
        // if (timer > 80)
        if (timer > 20)
        {
            flag = true;
            timer = 0;
        }
        // 如果标志为真，重置标志，更新窗口和游戏状态
        if (flag)
        {
            flag = false;
            updateWindow();
            updateGame();
        }
        Sleep(10);
    }

    // system("pause");// 等待用户输入,但是这个函数在非windows系统下可能不会起作用，所以建议使用getch()函数代替
    printf("press any key to exit\n");
    getch();      // 等待用户按键
    closegraph(); // 关闭图形窗口
    return 0;
}