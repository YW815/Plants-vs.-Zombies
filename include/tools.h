#ifndef TOOLS_H
#define TOOLS_H

#include <graphics.h>

// 载入PNG图并去透明部分
void _putimagePNG(int picture_x, int picture_y, IMAGE* picture) // x为载入图片的X坐标，y为Y坐标
{
    DWORD* dst = GetImageBuffer();    // 获取绘图设备的显存指针，EASYX自带
    DWORD* src = GetImageBuffer(picture); // 获取picture的显存指针
    int picture_width = picture->getwidth(); // 获取picture的宽度，EASYX自带
    int picture_height = picture->getheight(); // 获取picture的高度，EASYX自带
    int graphWidth = getwidth();       // 获取绘图区的宽度，EASYX自带
    int graphHeight = getheight();     // 获取绘图区的高度，EASYX自带

    // 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
    for (int iy = 0; iy < picture_height; iy++)
    {
        for (int ix = 0; ix < picture_width; ix++)
        {
            int srcX = ix + iy * picture_width; // 在显存里像素的角标
            int sa = ((src[srcX] & 0xff000000) >> 24); // 0xAArrggbb; AA是透明度
            int sr = ((src[srcX] & 0xff0000) >> 16); // 获取RGB里的R
            int sg = ((src[srcX] & 0xff00) >> 8);   // G
            int sb = src[srcX] & 0xff;              // B

            int dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; // 在显存里像素的角标
            if (dstX >= 0 && dstX < graphWidth * graphHeight)
            {
                int dr = ((dst[dstX] & 0xff0000) >> 16);
                int dg = ((dst[dstX] & 0xff00) >> 8);
                int db = dst[dstX] & 0xff;
                dst[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
                    | ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
                    | (sb * sa / 255 + db * (255 - sa) / 255);
            }
        }
    }
}

// 适用于 y < 0 以及 x < 0 的任何情况
void putimagePNG(int x, int y, IMAGE* picture) {
    IMAGE* imgTmp = new IMAGE;
    IMAGE* imgTmp2 = new IMAGE;
    IMAGE* imgTmp3 = new IMAGE;
    int winWidth = getwidth();
    int winHeight = getheight();

    if (y < 0) {
        SetWorkingImage(picture);
        int actualHeight = picture->getheight() + y;
        getimage(imgTmp, 0, -y, picture->getwidth(), actualHeight);
        SetWorkingImage();
        y = 0;
        picture = imgTmp;
    }
    else if (y >= winHeight || x >= winWidth) {
        delete imgTmp;
        delete imgTmp2;
        delete imgTmp3;
        return;
    }
    else if (y + picture->getheight() > winHeight) {
        SetWorkingImage(picture);
        int actualHeight = winHeight - y;
        getimage(imgTmp, x, y, picture->getwidth(), actualHeight);
        SetWorkingImage();
        picture = imgTmp;
    }

    if (x < 0) {
        SetWorkingImage(picture);
        int actualWidth = picture->getwidth() + x;
        getimage(imgTmp2, -x, 0, actualWidth, picture->getheight());
        SetWorkingImage();
        x = 0;
        picture = imgTmp2;
    }

    if (x > winWidth - picture->getwidth()) {
        SetWorkingImage(picture);
        int actualWidth = winWidth - x;
        getimage(imgTmp3, 0, 0, actualWidth, picture->getheight());
        SetWorkingImage();
        picture = imgTmp3;
    }

    // 确保图像被正确加载
    if (picture == nullptr) {
        delete imgTmp;
        delete imgTmp2;
        delete imgTmp3;
        return;
    }

    _putimagePNG(x, y, picture);

    // 释放不再使用的图像内存
    if (picture != imgTmp) delete imgTmp;
    if (picture != imgTmp2) delete imgTmp2;
    if (picture != imgTmp3) delete imgTmp3;
}

int getDelay() {
    static unsigned long long lastTime = 0;
    unsigned long long currentTime = GetTickCount();
    if (lastTime == 0) {
        lastTime = currentTime;
        return 0;
    }
    else {
        int ret = currentTime - lastTime;
        lastTime = currentTime;
        return ret;
    }
}

#endif // TOOLS_H
