 /*  -== Freetype ==-
 *
 * This allow to convert TTF to PNG some ASCII char
 *
 *  @autors
 *   - Maeiky
 *	 - Sebouney
 *  
 * Copyright (c) 2021 - V·Liance  /  SPinti Software
 *
 * The contents of this file are subject to the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * If a copy of the Apache License Version 2.0 was not distributed with this file,
 * You can obtain one at https://www.apache.org/licenses/LICENSE-2.0.html
 *
*/


/** Base include **/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <windows.h>


/** Create PNG **/
// #include <png.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_img.h"

HINSTANCE xe_hInstance = 0;

/** Xe-loader wrapper **/
#define ShowPixView
#include "Xternal/Xternal.h"
#include "XE/XEGI.h"

/** FreeType Lib **/
#include <ft2build.h>
#include "freetype/freetype.h"
#include "freetype/ftlcdfil.h"

/** Parameters **/
#define WIDTH   2048
#define HEIGHT  2048
#define ANGLE 0.0
#define SizeStep 4
#define SizeMax 50

int DPI 		= 100;
int FontSize 	= 8;

#define DISPLAY_RENDER 0

#define CHANNEL_NUM 3

/** Main array **/
unsigned char image[HEIGHT][WIDTH];
uint32_t pixels[HEIGHT][WIDTH];
uint8_t pixels_8[HEIGHT * WIDTH * CHANNEL_NUM];

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      _x,
             FT_Int      _y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = _x + bitmap->width;
	FT_Int  y_max = _y + bitmap->rows;

	// printf("\nbitmap->pixel_mode: %d \n", bitmap->pixel_mode  );
	// printf("\n bitmap->pitch  %d \n", bitmap->pitch  );
	// printf("\n bitmap->width  %d \n", bitmap->width  );
	// printf("\n bitmap->rows  %d \n", bitmap->rows  );

	
	for ( int y = 0; y < bitmap->rows; y++ ){
		 for ( int x = 0; x < bitmap->width; x+=3 ){
			if(bitmap->buffer != 0 ){
				uint8_t r = bitmap->buffer[y*bitmap->pitch + x+0];
				uint8_t g = bitmap->buffer[y*bitmap->pitch + x+1];
				uint8_t b = bitmap->buffer[y*bitmap->pitch + x+2];
				
				pixels[_y + y][_x +x/3 ] |=  (r<<16) | (g<<8) | b;

			}
		}
	}
	
	


}

int WINAPI 
	WinMain (HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
	printf(" -=== FreeType ===- \n\r");
	printf("\n\r");
	printf(" TFF to PNG converter for Cpcdos OSx\n\r");
	printf(" By VLiance and SPinti Software\n\r");
	setbuf(stdout, NULL);//Required to see every printf
	setbuf(stderr, NULL);//Required to see every printf


	FT_Library    	library;
	FT_Face       	face;

	FT_GlyphSlot  	slot;
	FT_Matrix     	matrix;
	FT_Vector     	pen;
	FT_Error      	error;


	char*         	filename;
	char*         	filepng;
	char*         	text;

	double        	angle;
	int           	target_height;
	int           	n, num_chars;

	filename = (char*)"arial.ttf";
	filepng = (char*)"arial.png";
	//text = (char*)"LCD Freetype Render!";
	
	text = (char*) calloc(127, 4);
	for(int index=32; index < 127; index++)
		sprintf(text, "%s%c", text, index);
	
	

	num_chars     = strlen( text );
	angle         = ( ANGLE / 360 ) * 3.14159 * 2;      // use 25 degrees
	target_height = HEIGHT;

	printf(" - FT_Init_FreeType() ... ");
	error = FT_Init_FreeType( &library );              // initialize library
	printf("[OK] return:%d\n", (int) error);


	printf(" - FT_New_Face(%s) ... ", filename);
	error = FT_New_Face( library, filename, 0, &face );// create face object
	printf("[OK] return:%d\n", (int) error);


	//set up matrix
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	// the pen position in 26.6 cartesian space coordinates;
	// start at (300,200) relative to the upper left corner
	pen.x = 1;// 30 * 64;
	pen.y = FontSize;// ( target_height - 200 ) * 64 - 12000;

	while(FontSize <= SizeMax)
	{
		
		
		// use 50pt at 100dpi
		printf(" - FT_Set_Char_Size(Size:%d DPI:%d) ... ", FontSize, DPI);
		error = FT_Set_Char_Size( face, FontSize * 64, 0, DPI, 0 );                //set character size
		printf("[OK] return:%d\n", (int) error);
		
		
		// error handling omitted
		slot = face->glyph;

		

		printf(" - FT_Library_SetLcdFilter() ... ");
		FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT);
		
		printf("[OK]\n");

		printf(" - Init font char... ");
		for ( n = 0; n < num_chars; n++ )
		{
			// set transformation
			FT_Set_Transform( face, &matrix, &pen );

			error = FT_Load_Char( face, text[n], FT_LOAD_RENDER | FT_LOAD_TARGET_LCD );
			
			if ( error )
			{
				printf("[error] %d\n", (int) error);
				continue;                 // ignore errors
			}
			  

			//now, draw to our target surface (convert position)
			draw_bitmap( &slot->bitmap,
						 slot->bitmap_left,
						 /*target_height*/ pen.y - slot->bitmap_top );

			// increment pen position
			pen.x += FontSize*DPI; //slot->advance.x;
			pen.y += slot->advance.y;
			
			if(pen.x > (WIDTH * 64) - FontSize*DPI)
			{
				pen.x = 1;
				pen.y += FontSize*2;
			}
		}
		
		printf("[OK]\n");
		
		FontSize += SizeStep;
		
		pen.x = 1;
		pen.y += FontSize*2;
	}
	  
	printf(" - Convert 32 bits to 8 bits ... ");
	
	
	int index = 0;
	for ( int y = 0; y < HEIGHT; y++ ){
		 for ( int x = 0; x < WIDTH; x++ )
		 {
			//int r = (int) (pixels[y][x] & 0xFF0000);
			//int g = (int) (pixels[y][x] & 0x00FF00);
			int b = (int) (pixels[y][x] & 0x0000FF);
			
			pixels_8[index+0] = b;
			pixels_8[index+1] = b;
			pixels_8[index+2] = b;
			
			index += 3;

		}
	}
	
	printf("[OK]\n");
	

	printf(" - Writing to %s file ... ", filepng);
	// stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
	stbi_write_png(filepng, WIDTH, HEIGHT, CHANNEL_NUM, pixels_8, WIDTH * CHANNEL_NUM);
	
	printf("[OK]\n");
	
	if(DISPLAY_RENDER == 1)
	{
		printf(" - Creating window ... ");
		int idx = Create_context((ContextInf){.width=WIDTH, .height=HEIGHT});
		Blit_context(idx, (uint8_t*)pixels, WIDTH);
		
		
		
		
		SetWindowTextA((HWND) _hInstance, "FreeType - Windows TTF file rendering experimentation");
		
		printf("[OK]\n");
		
		while(1)
		{		
			Sleep(1);
		}
	}
	
	printf(" - FT_Done_Face()\n");
	FT_Done_Face( face );
	printf(" - FT_Done_FreeType()\n");
	FT_Done_FreeType( library );

	printf(" Bye! \n\r");
	return 0;
}

