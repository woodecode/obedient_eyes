
#include "stdlib.h"
#include "usart.h"
#include "../include/font.h" 
#include "../include/alientek_lcd.h"
#include "../include/alientek_delay.h" 


//LCD的画笔颜色和背景色
u16 POINT_COLOR=0x0000;     //画笔颜色
u16 BACK_COLOR=0xFFFF;      //背景色 

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//写寄存器函数
//data:寄存器值
void LCD_WR_REG(u16 data)
{ 
    LCD_RS_CLR;//写地址
    LCD_CS_CLR;
    DATAOUT(data);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET; 
}

//写数据函数
//可以替代LCD_WR_DATAX宏,拿时间换空间.
//data:寄存器值
void LCD_WR_DATAX(u16 data)
{
    LCD_RS_SET;
    LCD_CS_CLR;
    DATAOUT(data);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}

//当mdk -O1时间优化时需要设置
//延时i
void opt_delay(u8 i)
{
    while (i--);
}

//读LCD数据
//返回值:读到的值
u16 LCD_RD_DATA(void)
{
    u16 t;
    GPIOB->CRL=0X88888888; //PB0-7  上拉输入
    GPIOB->CRH=0X88888888; //PB8-15 上拉输入
    GPIOB->ODR=0X0000;     //全部输出0

    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;

    opt_delay(2);
    t=DATAIN;  

    LCD_RD_SET;
    LCD_CS_SET; 

    GPIOB->CRL=0X33333333; //PB0-7  上拉输出
    GPIOB->CRH=0X33333333; //PB8-15 上拉输出
    GPIOB->ODR=0XFFFF;     //全部输出高
    return t;  
}

//写寄存器
//LCD_Reg:寄存器编号
//LCD_RegValue:要写入的值
void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue)
{
    LCD_WR_REG(LCD_Reg);  
    LCD_WR_DATA(LCD_RegValue);
}

//读寄存器
//LCD_Reg:寄存器编号
//返回值:读到的值
u16 LCD_ReadReg(u16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);        //写入要读的寄存器号
    return LCD_RD_DATA();
}

//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
} 
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(u16 RGB_Code)
{
    LCD_WR_DATA(RGB_Code);      //写十六位GRAM
}

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD_BGR2RGB(u16 c)
{
    u16  r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
u16 LCD_ReadPoint(u16 x,u16 y)
{
    u16 r, g, b;

    if (x >= lcddev.width || y >= lcddev.height)return 0;   //超过了范围,直接返回

    LCD_SetCursor(x, y);

	LCD_WR_REG(0X2E);

    r = LCD_RD_DATA();          //Dummy Read
    r = LCD_RD_DATA();          //实际坐标颜色

    //9341/5310/5510/7789 要分2次读出
    b = LCD_RD_DATA();
    g = r & 0XFF;               //对于 9341/5310/5510/7789, 第一次读取的是RG的值,R在前,G在后,各占8位
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));  // 9341/5310/5510/7789 需要公式转换一下
}

//LCD开启显示
void LCD_DisplayOn(void)
{
	LCD_WR_REG(0X29);       //开启显示
}

//LCD关闭显示
void LCD_DisplayOff(void)
{
	LCD_WR_REG(0X28);       //关闭显示

}

//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(Xpos >> 8);
	LCD_WR_DATA(Xpos & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(Ypos >> 8);
	LCD_WR_DATA(Ypos & 0XFF);
}

//设置LCD的自动扫描方向
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9341/5310/5510/1963/7789等IC已经实际测试
//注意:其他函数可能会受到此函数设置的影响(尤其是9341),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
void LCD_Scan_Dir(u8 dir)
{
    u16 regval = 0;
    u16 dirreg = 0;
    u16 temp;

    //横屏时，对1963不改变扫描方向, 其他IC改变扫描方向！竖屏时1963改变方向, 其他IC不改变扫描方向
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) || (lcddev.dir == 0 && lcddev.id == 0X1963))
    {
        switch (dir){
            case 0:dir = 6;break;
            case 1:dir = 7;break;
            case 2:dir = 4;break;
            case 3:dir = 5;break;
            case 4:dir = 1;break;
            case 5:dir = 0;break;
            case 6:dir = 3;break;
            case 7:dir = 2;break;
        }
    }

    switch (dir){
        case L2R_U2D://从左到右,从上到下
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;
        case L2R_D2U://从左到右,从下到上
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;
        case R2L_U2D://从右到左,从上到下
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;
        case R2L_D2U://从右到左,从下到上
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;
        case U2D_L2R://从上到下,从左到右
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;
        case U2D_R2L://从上到下,从右到左
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;
        case D2U_L2R://从下到上,从左到右
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;
        case D2U_R2L://从下到上,从右到左
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

	dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789)   //9341 & 7789 要设置BGR位
    {
        regval |= 0X08;
    }

    LCD_WriteReg(dirreg, regval);

    if (lcddev.id != 0X1963)   //1963不做坐标处理
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    } 
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.width - 1) >> 8);
	LCD_WR_DATA((lcddev.width - 1) & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.height - 1) >> 8);
	LCD_WR_DATA((lcddev.height - 1) & 0XFF);
}

//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x,u16 y)
{
    LCD_SetCursor(x, y);        //设置光标位置
    LCD_WriteRAM_Prepare();     //开始写入GRAM
    LCD_WR_DATA(POINT_COLOR);
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(x >> 8);
	LCD_WR_DATA(x & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(y >> 8);
	LCD_WR_DATA(y & 0XFF);

    LCD_RS_CLR;
    LCD_CS_CLR;
    DATAOUT(lcddev.wramcmd);    //写指令
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
    LCD_WR_DATA(color);         //写数据
}

//SSD1963 背光设置
//pwm:背光等级,0~100.越大越亮.
void LCD_SSD_BackLightSet(u8 pwm)
{
    LCD_WR_REG(0xBE);           //配置PWM输出
    LCD_WR_DATA(0x05);          //1设置PWM频率
    LCD_WR_DATA(pwm * 2.55);    //2设置PWM占空比
    LCD_WR_DATA(0x01);          //3设置C
    LCD_WR_DATA(0xFF);          //4设置D
    LCD_WR_DATA(0x00);          //5设置E
    LCD_WR_DATA(0x00);          //6设置F
}

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
    lcddev.dir = dir;       //竖屏/横屏

    if (dir == 0)           //竖屏
    {
        lcddev.width = 240;
        lcddev.height = 320;

		lcddev.wramcmd = 0X2C;
		lcddev.setxcmd = 0X2A;
		lcddev.setycmd = 0X2B;
    }
    else     //横屏
    {
        lcddev.width = 320;
        lcddev.height = 240;

		lcddev.wramcmd = 0X2C;
		lcddev.setxcmd = 0X2A;
		lcddev.setycmd = 0X2B;
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);     //默认扫描方向
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height. 
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{    
    u16 twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(sx >> 8);
	LCD_WR_DATA(sx & 0XFF);
	LCD_WR_DATA(twidth >> 8);
	LCD_WR_DATA(twidth & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(sy >> 8);
	LCD_WR_DATA(sy & 0XFF);
	LCD_WR_DATA(theight >> 8);
	LCD_WR_DATA(theight & 0XFF);

}

//初始化lcd
//该初始化函数可以初始化各种ALIENTEK出品的LCD液晶屏
//本函数占用较大flash,用户可以根据自己的实际情况,删掉未用到的LCD初始化代码.以节省空间.
void LCD_Init(void)
{
           
    __HAL_AFIO_REMAP_SWJ_DISABLE();				//禁止JTAG

    delay_ms(50); // delay 50 ms 
    LCD_WriteReg(0x0000,0x0001);
    delay_ms(50); // delay 50 ms 
    
    //尝试9341 ID的读取
    LCD_WR_REG(0XD3);
    lcddev.id = LCD_RD_DATA();          //dummy read
    lcddev.id = LCD_RD_DATA();          //读到0X00
    lcddev.id = LCD_RD_DATA();          //读取0X93
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA();         //读取0X41

    if (lcddev.id != 0X9341)            //不是 9341 , 尝试看看是不是 ST7789
    {
        LCD_WR_REG(0X04);
        lcddev.id = LCD_RD_DATA();      //dummy read
        lcddev.id = LCD_RD_DATA();      //读到0X85
        lcddev.id = LCD_RD_DATA();      //读取0X85
        lcddev.id <<= 8;
        lcddev.id |= LCD_RD_DATA();     //读取0X52

        if (lcddev.id == 0X8552)        //将8552的ID转换成7789
        {
            lcddev.id = 0x7789;
        }

    }

    // printf(" LCD ID:%x\r\n", lcddev.id); //打印LCD ID

    if (lcddev.id == 0x7789)     //7789初始化
    {
        LCD_WR_REG(0x11);

        delay_ms(120);

        LCD_WR_REG(0x36);
        LCD_WR_DATAX(0x00);

        LCD_WR_REG(0x3A);
        LCD_WR_DATAX(0X05);

        LCD_WR_REG(0xB2);
        LCD_WR_DATAX(0x0C);
        LCD_WR_DATAX(0x0C);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x33);
        LCD_WR_DATAX(0x33);

        LCD_WR_REG(0xB7);
        LCD_WR_DATAX(0x35);

        LCD_WR_REG(0xBB);       //vcom
        LCD_WR_DATAX(0x32);     //30

        LCD_WR_REG(0xC0);
        LCD_WR_DATAX(0x0C);

        LCD_WR_REG(0xC2);
        LCD_WR_DATAX(0x01);

        LCD_WR_REG(0xC3);       //vrh
        LCD_WR_DATAX(0x10);     //17 0D

        LCD_WR_REG(0xC4);       //vdv
        LCD_WR_DATAX(0x20);     //20

        LCD_WR_REG(0xC6);
        LCD_WR_DATAX(0x0f);

        LCD_WR_REG(0xD0);
        LCD_WR_DATAX(0xA4);
        LCD_WR_DATAX(0xA1);

        LCD_WR_REG(0xE0);       //Set Gamma
        LCD_WR_DATAX(0xd0);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x02);
        LCD_WR_DATAX(0x07);
        LCD_WR_DATAX(0x0a);
        LCD_WR_DATAX(0x28);
        LCD_WR_DATAX(0x32);
        LCD_WR_DATAX(0X44);
        LCD_WR_DATAX(0x42);
        LCD_WR_DATAX(0x06);
        LCD_WR_DATAX(0x0e);
        LCD_WR_DATAX(0x12);
        LCD_WR_DATAX(0x14);
        LCD_WR_DATAX(0x17);

        LCD_WR_REG(0XE1);       //Set Gamma
        LCD_WR_DATAX(0xd0);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x02);
        LCD_WR_DATAX(0x07);
        LCD_WR_DATAX(0x0a);
        LCD_WR_DATAX(0x28);
        LCD_WR_DATAX(0x31);
        LCD_WR_DATAX(0x54);
        LCD_WR_DATAX(0x47);
        LCD_WR_DATAX(0x0e);
        LCD_WR_DATAX(0x1c);
        LCD_WR_DATAX(0x17);
        LCD_WR_DATAX(0x1b);
        LCD_WR_DATAX(0x1e);

        LCD_WR_REG(0x2A);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0xef);

        LCD_WR_REG(0x2B);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x00);
        LCD_WR_DATAX(0x01);
        LCD_WR_DATAX(0x3f);

        LCD_WR_REG(0x29);       //display on
    }

    LCD_Display_Dir(0);         //默认为竖屏
    LCD_LED = 1;                //点亮背光
    LCD_Clear(WHITE);
}
  
//清屏函数
//color:要清屏的填充色
void LCD_Clear(u16 color)
{
    u32 index = 0;
    u32 totalpoint = lcddev.width;
    totalpoint *= lcddev.height;    //得到总点数

    LCD_SetCursor(0x00, 0x0000);    //设置光标位置
    LCD_WriteRAM_Prepare();         //开始写入GRAM

    for (index = 0; index < totalpoint; index++)
    {
        LCD_WR_DATA(color);
    }
}

//在指定区域内填充指定颜色
//区域大小:(xend-xsta+1)*(yend-ysta+1)
//xsta
//color:要填充的颜色
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
    u16 i, j;
    u16 xlen = 0;

    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i);       //设置光标位置
        LCD_WriteRAM_Prepare();     //开始写入GRAM

        for (j = 0; j < xlen; j++)
        {
            LCD_WR_DATA(color);     //设置光标位置
        }
    }
}

//在指定区域内填充指定颜色块
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{
    u16 height, width;
    u16 i, j;
    width = ex - sx + 1;            //得到填充的宽度
    height = ey - sy + 1;           //高度

    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i);  //设置光标位置
        LCD_WriteRAM_Prepare();     //开始写入GRAM

        for (j = 0; j < width; j++)
        {
            LCD_WR_DATA(color[i * width + j]); //写入数据
        }
    }
}

//在指定区域内填充指定颜色块
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
#if _USE_LVGL_INTERFACE_
void LCD_LV_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,lv_color_t *color)
{
    u16 height, width;
    u16 i, j;
    width = ex - sx + 1;            //得到填充的宽度
    height = ey - sy + 1;           //高度

    for (i = 0; i < height; i++) {
        LCD_SetCursor(sx, sy + i);  //设置光标位置
        LCD_WriteRAM_Prepare();     //开始写入GRAM

        for (j = 0; j < width; j++) {
            LCD_WR_DATA(color++->full); //写入数据
        }
    }
}
#endif
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;              //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)incx = 1;       //设置单步方向
    else if (delta_x == 0)incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y)distance = delta_x; //选取基本增量坐标轴
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++ )    //画线输出
    {
        LCD_DrawPoint(uRow, uCol); //画点
        xerr += delta_x ;
        yerr += delta_y ;

        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

//画矩形
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);       //判断下个点位置的标志

    while (a <= b)
    {
        LCD_DrawPoint(x0 + a, y0 - b);        //5
        LCD_DrawPoint(x0 + b, y0 - a);        //0
        LCD_DrawPoint(x0 + b, y0 + a);        //4
        LCD_DrawPoint(x0 + a, y0 + b);        //6
        LCD_DrawPoint(x0 - a, y0 + b);        //1
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - a, y0 - b);        //2
        LCD_DrawPoint(x0 - b, y0 - a);        //7
        a++;

        //使用Bresenham算法画圆
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{
    u8 temp, t1, t;
    u16 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);  //得到字体一个字符对应点阵集所占的字节数
    num = num - ' ';    //得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）

    for (t = 0; t < csize; t++)
    {
        if (size == 12)temp = asc2_1206[num][t];        //调用1206字体
        else if (size == 16)temp = asc2_1608[num][t];   //调用1608字体
        else if (size == 24)temp = asc2_2412[num][t];   //调用2412字体
        else return;                                    //没有的字库

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);

            temp <<= 1;
            y++;

            if (y >= lcddev.height)return;      //超区域了

            if ((y - y0) == size)
            {
                y = y0;
                x++;

                if (x >= lcddev.width)return;   //超区域了

                break;
            }
        }
    } 
}

//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;    
    return result;
}

//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{
    u8 t, temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + (size / 2)*t, y, ' ', size, 0);
                continue;
            }
            else enshow = 1;

        }

        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, 0);
    }
}

//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{  
    u8 t, temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                if (mode & 0X80)LCD_ShowChar(x + (size / 2)*t, y, '0', size, mode & 0X01);
                else LCD_ShowChar(x + (size / 2)*t, y, ' ', size, mode & 0X01);

                continue;
            }
            else enshow = 1;

        }

        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, mode & 0X01);
    }
}

//显示字符串
//x,y:起点坐标
//width,height:区域大小
//size:字体大小
//*p:字符串起始地址
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
    u8 x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' '))   //判断是不是非法字符!
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)break; //退出

        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}






























