/*
    Homeassistant-Eink-Display: Uses a Raspberry Pi Pico W (rp2040) and 
    a Waveshare Pico-ePaper-2.66 to display Home-Assisant sensors 
    (e.g.: temperature)
    Copyright (C) 2022  Guillermo López

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "EPD_2in66b.h"
#include "GUI_Paint.h"
#include "pico/stdlib.h"
#include <malloc.h>


void init_display();
void exit_display();
void clear_screen();
void clear_buffer();
void prepareBuffer();

void add_text(const char *pString, uint position, int color, bool small);
void add_value(const char *pString, uint position, int color);
void add_footer(const char *pString, uint position, int color);
void add_counter(const char *pString);
void add_grid();
void display_buffer();