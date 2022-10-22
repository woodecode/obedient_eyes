#include "main.h"
#include "../include/lcd.h"

/*-----------------LCD IO Define----------------*/
#define CTRL_PORT GPIOC /*PC6-PC9*/
#define DATA_PORT GPIOB 

#define LCD_CS_SET  CTRL_PORT->BSRR = 1 << 9    //CS Port      	PC9
#define LCD_RS_SET  CTRL_PORT->BSRR = 1 << 8    //data/cmd     	PC8
#define LCD_WR_SET  CTRL_PORT->BSRR = 1 << 7    //write data	PC7
#define LCD_RD_SET  CTRL_PORT->BSRR = 1 << 6    //read data  	PC6

#define LCD_CS_CLR  CTRL_PORT->BRR	= 1 << 9     //CS Port      PC9
#define LCD_RS_CLR  CTRL_PORT->BRR	= 1 << 8     //data/cmd     PC8
#define LCD_WR_CLR  CTRL_PORT->BRR	= 1 << 7     //write data	PC7
#define LCD_RD_CLR  CTRL_PORT->BRR	= 1 << 6     //read data  	PC6

#define LCD_BL_SET  CTRL_PORT->BSRR	= 1 << 10     //read data  	PC10
#define LCD_BL_CLR  CTRL_PORT->BRR	= 1 << 10     //read data  	PC10
/*PB0~15 used as Data Wire*/
/*macros WRITE_REG and READ_REG are implemented in the <stm32f1xx.h> file*/
#define LCD_DATA_OUTPUT(VAL) 	    WRITE_REG (DATA_PORT->ODR,VAL) 	/*data output */
#define LCD_DATA_INPUT(VAL)         READ_REG  (DATA_PORT->IDR)		/*data input  */

/**
 * @brief   simple delay function 
 * @param 	None
 * @return  None
 */
void opt_delay(uint8_t i) {
    while (i--);
}
/**
 * @brief   I/O port is initialized by STM32CubeMX
 * @param 	None
 * @return  None
 */
void LCD_IO_Init(void){
	LCD_BL_SET;
}
/**
 * @brief    
 * @param 	pData
 * @param 	Size
 * @return  None
 */
void LCD_IO_WriteMultipleData(uint16_t *pData, uint32_t Size){
	for(uint32_t i = 0; i < Size; i++){
		LCD_RS_CLR;//write addr
		LCD_CS_CLR;
		LCD_DATA_OUTPUT(*(pData + i));
		LCD_WR_CLR;
		LCD_WR_SET;
		LCD_CS_SET; 
	}
}
/**
 * @brief    
 * @param 	Reg
 * @return  None
 */
void LCD_IO_WriteReg(uint8_t Reg){
	LCD_RS_CLR;//write addr
    LCD_CS_CLR;
    LCD_DATA_OUTPUT(Reg);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET; 
}
/**
 * @brief    
 * @param 	RegValue
 * @return  None
 */
void LCD_IO_WriteData(uint16_t RegValue){
	LCD_RS_SET;
    LCD_CS_CLR;
    LCD_DATA_OUTPUT(RegValue);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}
/**
 * @brief    
 * @param 	None
 * @return  None
 */
uint16_t LCD_IO_ReadData(void){
	uint16_t t;
    DATA_PORT->CRL = 0X88888888; //PB0-7  pull-up input
    DATA_PORT->CRH = 0X88888888; //PB8-15 pull-up input
    DATA_PORT->ODR = 0X0000;     //output 0x0000
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_RD_CLR;
    opt_delay(2);
    t = LCD_DATA_INPUT();  
    LCD_RD_SET;
    LCD_CS_SET; 
    DATA_PORT->CRL = 0X33333333; //PB0-7  pull-up output
    DATA_PORT->CRH = 0X33333333; //PB8-15 pull-up output
    DATA_PORT->ODR = 0XFFFF;     //output 0xffff
    return t;  
}
/**
 * @brief   delay ms
 * @param   delay
 * @return  None
 */
void LCD_IO_Delay(uint32_t delay){
	HAL_Delay(delay);
}







