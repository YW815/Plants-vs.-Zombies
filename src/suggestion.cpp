struct Prompt
{
    int index;
}
IMAGE Prompt[10];

//想要制作一个提示信息，使其可以让玩家将植物放在已经有植物的地方就会报错提示

//插入gameinit()函数中
//加载报错提示信息
char path[100];
sprintf(path, "res/Prompt/%d.png", Prompt);
loadimage(&Prompt, path);

//在updatewindow()函数中
//如果草坪上已有植物，提示用户
if(Prompt == 1){
    putimagePNG(250, 200, &Prompt);
}


//在userclick()函数中
//如果用户点击了草坪上的植物，提示用户不能再放置植物
else
    [
        Prompt = 1;
    ]