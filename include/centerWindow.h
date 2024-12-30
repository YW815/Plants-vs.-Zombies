#ifndef CENTERWINDOW_H
#define CENTERWINDOW_H

#include <windows.h> // 确保包含必要的头文件

void centerWindow(int windowWidth, int windowHeight)//窗口居中函数
{
    HWND hWnd = initgraph(windowWidth, windowHeight,1); // 初始化窗口
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // 获取屏幕宽度
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // 获取屏幕高度
    // 计算窗口左上角的位置，使窗口居中
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;
    MoveWindow(hWnd, windowX, windowY, 900, 600, TRUE); // 移动窗口到指定位置
}

#endif // CENTERWINDOW_H
