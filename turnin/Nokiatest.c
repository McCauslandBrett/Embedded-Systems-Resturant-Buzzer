/* Nokia 5110 LCD AVR Library example
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by SkewPL, http://skew.tk
 */

#include <avr/io.h>
#include <util/delay.h>

#include "nokia5110.h"

int main(void)
{
    // DDRB = 0xFF;PORTB =0xFF;
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("Dans Pizzia",1);
    nokia_lcd_set_cursor(0, 10);
    nokia_lcd_write_string("Order #1", 2);
    nokia_lcd_render();

    while(1) {
        _delay_ms(1000);
    }
}
