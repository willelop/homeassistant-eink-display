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
#include "eink_display_266.h"

#define PADDING 4

#define OFFSETY EPD_2IN66B_HEIGHT/2
#define OFFSETX EPD_2IN66B_WIDTH/2
#define OFFSET_COUNTER EPD_2IN66B_HEIGHT-PADDING-6*7


#define TEXT_SIZE 20
#define ROTATION ROTATE_90

unsigned int offset_x[4];
unsigned int offset_y[4];

UBYTE *BlackImage;
UBYTE *RedImage;

void clear_screen()
{
    EPD_2IN66B_Clear();
}

void init_display()
{
    DEV_Module_Init();
    EPD_2IN66B_Init(); // init 1 Gray mode
    offset_x[0] = 0;
    offset_x[1] = 0;
    offset_x[2] = OFFSETX;
    offset_x[3] = OFFSETX;
    offset_y[0] = 0;
    offset_y[1] = OFFSETY;
    offset_y[2] = 0;
    offset_y[3] = OFFSETY;
    //clear_screen();
}

void exit_display()
{
        DEV_Module_Exit();
}

uint16_t select_color(int color)
{
    uint16_t color_text;

    if (color == 1)
    {
        Paint_SelectImage(RedImage);
        color_text = RED;
    }
    else
    {
        Paint_SelectImage(BlackImage);
        color_text = BLACK;
    }
    return color_text;
}
void add_text(const char *pString, uint position, int color, bool small)
{    
    uint16_t color_text = select_color(color);
    sFONT font = Font20;
    if(small)   {font = Font12;}
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_DrawString_EN(offset_y[position]+PADDING,offset_x[position]+PADDING, pString, &font, WHITE,color_text);
    return;
}

void add_counter(const char *pString)
{
    uint16_t color_text = select_color(0);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_DrawRectangle(OFFSET_COUNTER,PADDING,EPD_2IN66B_HEIGHT-PADDING,PADDING+12,BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawString_EN(OFFSET_COUNTER,PADDING, pString, &Font12, WHITE, color_text);
    return;
}

void add_value(const char *pString, uint position, int color)
{
    uint16_t color_text = select_color(color);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(4);
    Paint_DrawString_EN((offset_y[position]+PADDING)/2,((offset_x[position]+TEXT_SIZE+PADDING))/2, pString, &Font24, WHITE, color_text);
    return;
}
void add_footer(const char *pString, uint position, int color)
{
    uint16_t color_text = select_color(color);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_DrawString_EN(offset_y[position]+PADDING,((offset_x[position]+OFFSETX-12)), pString, &Font12, WHITE, color_text);
    return;
}

void display_buffer()
{
    EPD_2IN66B_Init();
    EPD_2IN66B_Display(BlackImage,RedImage);
    EPD_2IN66B_Sleep();
}

void add_grid()
{
    Paint_SelectImage(RedImage);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_DrawLine(OFFSETY,0,OFFSETY,EPD_2IN66B_WIDTH,RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(0,OFFSETX+4,EPD_2IN66B_HEIGHT,OFFSETX+4,RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
}

void prepareBuffer()
{
    UWORD Imagesize = ((EPD_2IN66B_WIDTH % 8 == 0)? (EPD_2IN66B_WIDTH / 8 ): (EPD_2IN66B_WIDTH / 8 + 1)) * EPD_2IN66B_HEIGHT;

    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for BLACK memory...\r\n");
        return;
    }

    if ((RedImage = (UBYTE *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for RED memory...\r\n");
        return;
    }
    Paint_NewImage(BlackImage, EPD_2IN66B_WIDTH, EPD_2IN66B_HEIGHT, ROTATION, BLACK);
    Paint_NewImage(RedImage, EPD_2IN66B_WIDTH, EPD_2IN66B_HEIGHT, ROTATION, RED);

    Paint_SelectImage(BlackImage);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
    Paint_SelectImage(RedImage);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
}

void clear_buffer()
{
    Paint_SelectImage(BlackImage);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_Clear(WHITE);
    Paint_SelectImage(RedImage);
    Paint_SetRotate(ROTATION);
    Paint_SetScale(2);
    Paint_Clear(WHITE); 
}
