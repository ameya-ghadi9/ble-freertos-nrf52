#include "LCD.h"

void LCD_GPIO_Init(void)
{
	NRF_GPIO->PIN_CNF[LCD_RS]  = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
	NRF_GPIO->PIN_CNF[LCD_E]   = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
	NRF_GPIO->PIN_CNF[LCD_DB4] = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
	NRF_GPIO->PIN_CNF[LCD_DB5] = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
	NRF_GPIO->PIN_CNF[LCD_DB6] = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
	NRF_GPIO->PIN_CNF[LCD_DB7] = ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 8)| (0 << 16));
}

void LCD_Enable(void)
{
	NRF_GPIO->OUTSET = (1UL<<LCD_E);
	nrf_delay_us (10);
	NRF_GPIO->OUTCLR = (1UL<<LCD_E);
	nrf_delay_us (50);
}

void LCD_Set_Data(uint8_t data)
{
	data = (data&0x0F);
	if (data & 0x01)
	{
		NRF_GPIO->OUTSET = (1UL<<LCD_DB4);
	}
	else
	{
		NRF_GPIO->OUTCLR = (1UL<<LCD_DB4);
	}
	if ((data>>1) & 0x01)
	{
		NRF_GPIO->OUTSET = (1UL<<LCD_DB5);
	}
	else
	{
		NRF_GPIO->OUTCLR = (1UL<<LCD_DB5);
	}
	if ((data>>2) & 0x01)
	{
		NRF_GPIO->OUTSET = (1UL<<LCD_DB6);
	}
	else
	{
		NRF_GPIO->OUTCLR = (1UL<<LCD_DB6);
	}
	if ((data>>3) & 0x01)
	{
		NRF_GPIO->OUTSET = (1UL<<LCD_DB7);
	}
	else
	{
		NRF_GPIO->OUTCLR = (1UL<<LCD_DB7);
	}
}

void LCD_CM_Write(uint8_t cm)
{
      NRF_GPIO->OUTCLR = (1UL<<LCD_RS);
      LCD_Set_Data(cm>>4);
      LCD_Enable();
      LCD_Set_Data(cm & 0x0F);
      LCD_Enable();
      nrf_delay_ms(10);
}

void LCD_Data_Write(uint8_t data)
{
      NRF_GPIO->OUTSET = (1UL<<LCD_RS);
      LCD_Set_Data(data>>4);
      LCD_Enable();
      LCD_Set_Data(data & 0x0F);
      LCD_Enable();
      nrf_delay_ms(10);
}

void LCD_Init(void)
{
      NRF_GPIO->OUTCLR = (1UL<<LCD_DB4);
      NRF_GPIO->OUTCLR = (1UL<<LCD_DB5);
      NRF_GPIO->OUTCLR = (1UL<<LCD_DB6);
      NRF_GPIO->OUTCLR = (1UL<<LCD_DB7);
      
      NRF_GPIO->OUTCLR = (1UL<<LCD_E);
      NRF_GPIO->OUTCLR = (1UL<<LCD_RS);
      
      nrf_delay_ms(15);
      LCD_Set_Data(0x03);
      nrf_delay_ms(5);
      LCD_Set_Data(0x03);
      nrf_delay_us(160);
      LCD_Set_Data(0x03);
      nrf_delay_us(160);
      LCD_Set_Data(0x02);
  
      LCD_CM_Write(0x28);    /* function set */
       //LCD_CM_WRITE(0x08);
      LCD_CM_Write(0x0C);    /* display on,cursor off,blink off */
      LCD_CM_Write(0x01);    /* clear display */
      LCD_CM_Write(0x06);    /* entry mode, set increment */
}

void LCD_Build_Char(uint8_t loc, uint8_t *p)
{
     unsigned char i;
	 
     if(loc<8)                                 //If valid address
    {
        LCD_CM_Write(0x40+(loc*8));               //Write to CGRAM
        for(i=0;i<8;i++)
        LCD_Data_Write(p[i]);                   //Write the character pattern to CGRAM
     }
      LCD_CM_Write(0x80);                           //shift back to DDRAM location 0
}

void LCD_Go_To_xy(unsigned char x, unsigned char y)
{
    if(y!=0) LCD_CM_Write(0xC0+x);
    else LCD_CM_Write(0x80+x);
    nrf_delay_us(1000);
}
