#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nrf_delay.h"


#define LCD_RS         25  
#define LCD_E          26  
#define LCD_DB4        27  
#define LCD_DB5        28  
#define LCD_DB6        02  
#define LCD_DB7        03  

void LCD_GPIO_Init(void);
void LCD_Init(void);

void LCD_Enable(void);
void LCD_Set_Data(uint8_t data);
void LCD_CM_Write(uint8_t cm);
void LCD_Data_Write(uint8_t data);

void LCD_Build_Char(uint8_t loc, uint8_t *p);
void LCD_Go_To_xy(uint8_t x, uint8_t y);
