
#include "stdlib.h"
#include "usart.h"
#include "../include/font.h" 
#include "../include/alientek_lcd.h"
#include "../include/alientek_delay.h" 


//LCD�Ļ�����ɫ�ͱ���ɫ
u16 POINT_COLOR=0x0000;     //������ɫ
u16 BACK_COLOR=0xFFFF;      //����ɫ 

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;

//д�Ĵ�������
//data:�Ĵ���ֵ
void LCD_WR_REG(u16 data)
{ 
    LCD_RS_CLR;//д��ַ
    LCD_CS_CLR;
    DATAOUT(data);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET; 
}

//д���ݺ���
//�������LCD_WR_DATAX��,��ʱ�任�ռ�.
//data:�Ĵ���ֵ
void LCD_WR_DATAX(u16 data)
{
    LCD_RS_SET;
    LCD_CS_CLR;
    DATAOUT(data);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}

//��mdk -O1ʱ���Ż�ʱ��Ҫ����
//��ʱi
void opt_delay(u8 i)
{
    while (i--);
}

//��LCD����
//����ֵ:������ֵ
u16 LCD_RD_DATA(void)
{
    u16 t;
    GPIOB->CRL=0X88888888; //PB0-7  ��������
    GPIOB->CRH=0X88888888; //PB8-15 ��������
    GPIOB->ODR=0X0000;     //ȫ�����0

    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;

    opt_delay(2);
    t=DATAIN;  

    LCD_RD_SET;
    LCD_CS_SET; 

    GPIOB->CRL=0X33333333; //PB0-7  �������
    GPIOB->CRH=0X33333333; //PB8-15 �������
    GPIOB->ODR=0XFFFF;     //ȫ�������
    return t;  
}

//д�Ĵ���
//LCD_Reg:�Ĵ������
//LCD_RegValue:Ҫд���ֵ
void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue)
{
    LCD_WR_REG(LCD_Reg);  
    LCD_WR_DATA(LCD_RegValue);
}

//���Ĵ���
//LCD_Reg:�Ĵ������
//����ֵ:������ֵ
u16 LCD_ReadReg(u16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);        //д��Ҫ���ļĴ�����
    return LCD_RD_DATA();
}

//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
} 
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(u16 RGB_Code)
{
    LCD_WR_DATA(RGB_Code);      //дʮ��λGRAM
}

//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
u16 LCD_BGR2RGB(u16 c)
{
    u16  r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

//��ȡ��ĳ�����ɫֵ
//x,y:����
//����ֵ:�˵����ɫ
u16 LCD_ReadPoint(u16 x,u16 y)
{
    u16 r, g, b;

    if (x >= lcddev.width || y >= lcddev.height)return 0;   //�����˷�Χ,ֱ�ӷ���

    LCD_SetCursor(x, y);

	LCD_WR_REG(0X2E);

    r = LCD_RD_DATA();          //Dummy Read
    r = LCD_RD_DATA();          //ʵ��������ɫ

    //9341/5310/5510/7789 Ҫ��2�ζ���
    b = LCD_RD_DATA();
    g = r & 0XFF;               //���� 9341/5310/5510/7789, ��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));  // 9341/5310/5510/7789 ��Ҫ��ʽת��һ��
}

//LCD������ʾ
void LCD_DisplayOn(void)
{
	LCD_WR_REG(0X29);       //������ʾ
}

//LCD�ر���ʾ
void LCD_DisplayOff(void)
{
	LCD_WR_REG(0X28);       //�ر���ʾ

}

//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(Xpos >> 8);
	LCD_WR_DATA(Xpos & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(Ypos >> 8);
	LCD_WR_DATA(Ypos & 0XFF);
}

//����LCD���Զ�ɨ�跽��
//dir:0~7,����8������(���嶨���lcd.h)
//9341/5310/5510/1963/7789��IC�Ѿ�ʵ�ʲ���
//ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341),
//����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
void LCD_Scan_Dir(u8 dir)
{
    u16 regval = 0;
    u16 dirreg = 0;
    u16 temp;

    //����ʱ����1963���ı�ɨ�跽��, ����IC�ı�ɨ�跽������ʱ1963�ı䷽��, ����IC���ı�ɨ�跽��
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
        case L2R_U2D://������,���ϵ���
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;
        case L2R_D2U://������,���µ���
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;
        case R2L_U2D://���ҵ���,���ϵ���
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;
        case R2L_D2U://���ҵ���,���µ���
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;
        case U2D_L2R://���ϵ���,������
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;
        case U2D_R2L://���ϵ���,���ҵ���
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;
        case D2U_L2R://���µ���,������
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;
        case D2U_R2L://���µ���,���ҵ���
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

	dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789)   //9341 & 7789 Ҫ����BGRλ
    {
        regval |= 0X08;
    }

    LCD_WriteReg(dirreg, regval);

    if (lcddev.id != 0X1963)   //1963�������괦��
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //����X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //����X,Y
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

//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x,u16 y)
{
    LCD_SetCursor(x, y);        //���ù��λ��
    LCD_WriteRAM_Prepare();     //��ʼд��GRAM
    LCD_WR_DATA(POINT_COLOR);
}

//���ٻ���
//x,y:����
//color:��ɫ
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
    DATAOUT(lcddev.wramcmd);    //дָ��
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
    LCD_WR_DATA(color);         //д����
}

//SSD1963 ��������
//pwm:����ȼ�,0~100.Խ��Խ��.
void LCD_SSD_BackLightSet(u8 pwm)
{
    LCD_WR_REG(0xBE);           //����PWM���
    LCD_WR_DATA(0x05);          //1����PWMƵ��
    LCD_WR_DATA(pwm * 2.55);    //2����PWMռ�ձ�
    LCD_WR_DATA(0x01);          //3����C
    LCD_WR_DATA(0xFF);          //4����D
    LCD_WR_DATA(0x00);          //5����E
    LCD_WR_DATA(0x00);          //6����F
}

//����LCD��ʾ����
//dir:0,������1,����
void LCD_Display_Dir(u8 dir)
{
    lcddev.dir = dir;       //����/����

    if (dir == 0)           //����
    {
        lcddev.width = 240;
        lcddev.height = 320;

		lcddev.wramcmd = 0X2C;
		lcddev.setxcmd = 0X2A;
		lcddev.setycmd = 0X2B;
    }
    else     //����
    {
        lcddev.width = 320;
        lcddev.height = 240;

		lcddev.wramcmd = 0X2C;
		lcddev.setxcmd = 0X2A;
		lcddev.setycmd = 0X2B;
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);     //Ĭ��ɨ�跽��
}

//���ô���,���Զ����û������굽�������Ͻ�(sx,sy).
//sx,sy:������ʼ����(���Ͻ�)
//width,height:���ڿ�Ⱥ͸߶�,�������0!!
//�����С:width*height. 
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

//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ������ALIENTEK��Ʒ��LCDҺ����
//������ռ�ýϴ�flash,�û����Ը����Լ���ʵ�����,ɾ��δ�õ���LCD��ʼ������.�Խ�ʡ�ռ�.
void LCD_Init(void)
{
           
    __HAL_AFIO_REMAP_SWJ_DISABLE();				//��ֹJTAG

    delay_ms(50); // delay 50 ms 
    LCD_WriteReg(0x0000,0x0001);
    delay_ms(50); // delay 50 ms 
    
    //����9341 ID�Ķ�ȡ
    LCD_WR_REG(0XD3);
    lcddev.id = LCD_RD_DATA();          //dummy read
    lcddev.id = LCD_RD_DATA();          //����0X00
    lcddev.id = LCD_RD_DATA();          //��ȡ0X93
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA();         //��ȡ0X41

    if (lcddev.id != 0X9341)            //���� 9341 , ���Կ����ǲ��� ST7789
    {
        LCD_WR_REG(0X04);
        lcddev.id = LCD_RD_DATA();      //dummy read
        lcddev.id = LCD_RD_DATA();      //����0X85
        lcddev.id = LCD_RD_DATA();      //��ȡ0X85
        lcddev.id <<= 8;
        lcddev.id |= LCD_RD_DATA();     //��ȡ0X52

        if (lcddev.id == 0X8552)        //��8552��IDת����7789
        {
            lcddev.id = 0x7789;
        }

    }

    // printf(" LCD ID:%x\r\n", lcddev.id); //��ӡLCD ID

    if (lcddev.id == 0x7789)     //7789��ʼ��
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

    LCD_Display_Dir(0);         //Ĭ��Ϊ����
    LCD_LED = 1;                //��������
    LCD_Clear(WHITE);
}
  
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u16 color)
{
    u32 index = 0;
    u32 totalpoint = lcddev.width;
    totalpoint *= lcddev.height;    //�õ��ܵ���

    LCD_SetCursor(0x00, 0x0000);    //���ù��λ��
    LCD_WriteRAM_Prepare();         //��ʼд��GRAM

    for (index = 0; index < totalpoint; index++)
    {
        LCD_WR_DATA(color);
    }
}

//��ָ�����������ָ����ɫ
//�����С:(xend-xsta+1)*(yend-ysta+1)
//xsta
//color:Ҫ������ɫ
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
    u16 i, j;
    u16 xlen = 0;

    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i);       //���ù��λ��
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM

        for (j = 0; j < xlen; j++)
        {
            LCD_WR_DATA(color);     //���ù��λ��
        }
    }
}

//��ָ�����������ָ����ɫ��
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{
    u16 height, width;
    u16 i, j;
    width = ex - sx + 1;            //�õ����Ŀ��
    height = ey - sy + 1;           //�߶�

    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i);  //���ù��λ��
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM

        for (j = 0; j < width; j++)
        {
            LCD_WR_DATA(color[i * width + j]); //д������
        }
    }
}

//��ָ�����������ָ����ɫ��
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
//color:Ҫ������ɫ
#if _USE_LVGL_INTERFACE_
void LCD_LV_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,lv_color_t *color)
{
    u16 height, width;
    u16 i, j;
    width = ex - sx + 1;            //�õ����Ŀ��
    height = ey - sy + 1;           //�߶�

    for (i = 0; i < height; i++) {
        LCD_SetCursor(sx, sy + i);  //���ù��λ��
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM

        for (j = 0; j < width; j++) {
            LCD_WR_DATA(color++->full); //д������
        }
    }
}
#endif
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;              //������������
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)incx = 1;       //���õ�������
    else if (delta_x == 0)incx = 0; //��ֱ��
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; //ˮƽ��
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y)distance = delta_x; //ѡȡ��������������
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++ )    //�������
    {
        LCD_DrawPoint(uRow, uCol); //����
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

//������
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);       //�ж��¸���λ�õı�־

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

        //ʹ��Bresenham�㷨��Բ
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16/24
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{
    u8 temp, t1, t;
    u16 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
    num = num - ' ';    //�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩

    for (t = 0; t < csize; t++)
    {
        if (size == 12)temp = asc2_1206[num][t];        //����1206����
        else if (size == 16)temp = asc2_1608[num][t];   //����1608����
        else if (size == 24)temp = asc2_2412[num][t];   //����2412����
        else return;                                    //û�е��ֿ�

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);

            temp <<= 1;
            y++;

            if (y >= lcddev.height)return;      //��������

            if ((y - y0) == size)
            {
                y = y0;
                x++;

                if (x >= lcddev.width)return;   //��������

                break;
            }
        }
    } 
}

//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;    
    return result;
}

//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);
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

//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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

//��ʾ�ַ���
//x,y:�������
//width,height:�����С
//size:�����С
//*p:�ַ�����ʼ��ַ
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
    u8 x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' '))   //�ж��ǲ��ǷǷ��ַ�!
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)break; //�˳�

        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}






























