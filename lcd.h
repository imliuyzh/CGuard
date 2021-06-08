/**
 * lcd.h
 * Copyright (C) 2001-2021, Tony Givargis and Yizhen Liu
 */

#ifndef _LCD_H_
#define _LCD_H_

void lcd_init(void);

void lcd_clr(void);

void lcd_pos(unsigned char r, unsigned char c);

void lcd_put(char c);

void lcd_puts(const char *s);

void lcd_print(const char *line1, const char *line2);

#endif /* _LCD_H_ */
