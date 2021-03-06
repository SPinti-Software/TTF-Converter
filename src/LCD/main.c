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
 * Created : 10-SEPT-2021 (Re-wrote)
 * Updated : 12-OCT-2021
 *
*/


/** Base include **/
#include <stdio.h>
#include <stdlib.h>
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
#define WIDTH   	5000
#define HEIGHT  	640
#define ANGLE 		0.0
#define SizeStep 	4
#define SizeMax 	50

int DPI 		= 100;
int FontSize 	= 8;

#define DISPLAY_RENDER 0

#define CHANNEL_NUM 4

/** Main var **/
#define MaxFontName 	16 // Numbers of char MAX
#define MaxFontFiles 	24 	// Numbers of fonts
#define MaxNumberSizes 	32

#define MaxTextLength	128 // Maximum CHAAAAAAR array elements
#define MaxFilePath		64 	// PNG, INI, TTF file length

char* Temp_dir 				= "TEMP\\fonts";
char* config_file 			= "KRNL\\config\\fonts.cfg";

int IndexFontFilesArray 	= 0; 				// Incrementable index file font
int Size_List[MaxFontFiles][MaxNumberSizes]; 	// Sizes list by font
int Size_List_nb[MaxFontFiles]; 				// Number of sizes by fonts
char Font_List_name[MaxFontFiles][MaxFontName];


/** Main image render **/
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

	printf(" - Loading config file (%s)... \n", config_file);
	FILE *open_cfg;
	int bufferLength = 4096;
	char buffer[bufferLength];
	
	

	open_cfg = fopen(config_file, "r");
	if (open_cfg == NULL)
        exit(EXIT_FAILURE);

	bool Line_SRC = true;
	char* SRC_Str = calloc(24, 1);

	char* Size_List_str = (char*) calloc(MaxNumberSizes, bufferLength);

	while(fgets(buffer, bufferLength, open_cfg))
	{


		for(int index = 0; index < bufferLength; index++)
		{
			// Path source
			if((buffer[index] == 'S') && 
				(buffer[index+1] == 'R') && 
				(buffer[index+2] == 'C') && 
				(buffer[index+3] == '='))
			{
				Line_SRC = true;
			}
			// Check "=" char
			if(buffer[index] == '=')
			{
				// Alloc memory
				//Font_List_name[IndexFontFilesArray] = (char*) calloc(index + 1, MaxFontName);
				
				

				// Check chars before and after "="
				for(int pos = 0; pos < bufferLength; pos++)
				{
					if((pos >= bufferLength) || (buffer[pos] == '\0')) break;

					// Getting font name before "="
					if(pos < index)
					{
						if(Line_SRC == false)
							// Recuperer le nom du font
							Font_List_name[IndexFontFilesArray][pos] = (char) buffer[pos];
						
					}
					// Getting font sizes after "="
					else if((pos > index) && (pos < bufferLength))
					{
						if(Line_SRC == false)
							// Recuperer les parametres des SIZES
							Size_List_str[pos - (index+1)] = buffer[pos];
						else
							// Recuperer le path source (sans le CRLF)
							if((buffer[pos] != '\n') && (buffer[pos] != '\r'))
								SRC_Str[pos - (index+1)] = buffer[pos];
						
					}
				}
				break;
			}


		}
		
		if(Line_SRC == true)
		{
			printf(" --> Source font path : '%s'\n", SRC_Str);
			Line_SRC = false;
		}
		else
		
		{
			
			// Reformat to int array
			int NumberElements = 0;
			char *currnum;
			int numbers[MaxNumberSizes], i = 0;

			while ((currnum = strtok(i ? NULL : Size_List_str, ",")) != NULL)
			{
				int number_to_put = atoi(currnum);

				if(number_to_put > 0)
				{
					Size_List[IndexFontFilesArray][i++] = number_to_put;
					Size_List_nb[IndexFontFilesArray]++;
				}
			}
			

			printf("======= '%s' =======\n", Font_List_name[IndexFontFilesArray]);
			printf(" --> Index : %d Size_List[%d] : ", IndexFontFilesArray, Size_List_nb[IndexFontFilesArray]);
			for(int b = 0; b < Size_List_nb[IndexFontFilesArray]; b++)
				printf(" %d ", Size_List[IndexFontFilesArray][b]);
			
			IndexFontFilesArray++;
		}
		printf(".\n");

		
    }

	printf("Config file loaded !\n");

	

	int Loaded_Font = 0;
	int Next_Size = 0;

	while(Loaded_Font < IndexFontFilesArray)
	{

		printf("----------------------------------\n");
		

		double        	angle;
		int           	target_height;
		int           	n, num_chars;

		char filename[MaxFilePath];
		char filepng[MaxFilePath];
		char fileini[MaxFilePath];

		char text[MaxTextLength];

		sprintf(filename, "%s\\%s.ttf", SRC_Str, Font_List_name[Loaded_Font]); // MEDIA path
		sprintf(filepng, "%s\\%s.png", Temp_dir, Font_List_name[Loaded_Font]); // TEMP path
		sprintf(fileini, "%s\\%s.ini", Temp_dir, Font_List_name[Loaded_Font]); // TEMP path
		
		

		FontSize = Size_List[Loaded_Font][Next_Size]; // put first size
		
		printf(" --> Loaded_Font:%d - Generation of '%s' and '%s' from '%s'\n", Loaded_Font, filepng, fileini, filename);
		printf(" --> With %d differents sizes [", Size_List_nb[Loaded_Font]);
		for(int b = 0; b < Size_List_nb[Loaded_Font]; b++)
			printf("%d", Size_List[Loaded_Font][b]);

		printf("]\n\n");

		printf("     Creating char array ... ");
		
		// Cleanning
		for(int index=0; index < MaxTextLength; index++)
			text[index] = '\0';

		// Writing !
		for(int index=32; index < MaxTextLength; index++)
			sprintf(text, "%s%c", text, index);

		printf("[OK]\n");
		

		num_chars     = strlen( text );
		angle         = ( ANGLE / 360 ) * 3.14159 * 2;      // use 25 degrees
		target_height = HEIGHT;

		{
			FT_Library    	library;
			FT_Face       	face;

			FT_GlyphSlot  	slot;
			FT_Matrix     	matrix;
			FT_Vector     	pen;
			FT_Error      	error;

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

			printf(" - Opening INI file (%s) for writing ... ", fileini);
			FILE *write_ini;
			write_ini = fopen(fileini, "w");
			printf("[OK]\n");

			int char_sizes_list[128];
			memset(char_sizes_list, -1, 128);
			
			for(int boucle = 1; boucle <= Size_List_nb[Loaded_Font]; boucle++)
			{
				int NbSautsLigne = 1;
				int Pos_X = 1;
				int Pos_Y = 0;
				int Size_X = WIDTH;
				int Size_Y = 0;


				
				// use 50pt at 100dpi
				printf(" - FT_Set_Char_Size(Size:%d DPI:%d) ... ", FontSize, DPI);
				error = FT_Set_Char_Size( face, FontSize * 64, 0, DPI, 0 );                //set character size
				printf("[OK] return:%d\n", (int) error);
				
				
				// error handling omitted
				slot = face->glyph;

				Pos_Y = (pen.y) - FontSize;

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
					// pen.x += FontSize*DPI; //slot->advance.x;
					pen.x += slot->advance.x;
					pen.y += slot->advance.y;

					// Stocker la taille de chaque char dans un tableau
					char_sizes_list[n] = slot->advance.x / 64;

					if(Size_Y < slot->bitmap.rows)
						Size_Y = slot->bitmap.rows;
					
					if(pen.x > (WIDTH * 64) - FontSize*DPI)
					{
						pen.x = 1;
						pen.y += FontSize*2;
						NbSautsLigne++;
					}
				}


				
				
				
				if(Pos_X <= 0)
					Pos_X = 1;
				if(Pos_Y <= 0)
					Pos_Y = 1;
					
				printf("[OK]\n");

				fprintf(write_ini, "[FONT_SIZE_%d]\n", FontSize);
				fprintf(write_ini, "width=%d\n", (FontSize*DPI) / 64);
				fprintf(write_ini, "height=%d\n", (FontSize*2) - (FontSize/2));
				fprintf(write_ini, "org_x=%d\n", Pos_X);
				fprintf(write_ini, "org_y=%d\n", Pos_Y);
				fprintf(write_ini, "size_x=%d\n", Size_X);
				fprintf(write_ini, "size_y=%d\n", Size_Y);
				fprintf(write_ini, "char_size=");
				for(int b = 0; b < 128; b++)
					if(char_sizes_list[b] >= 0)
						fprintf(write_ini, "%d,", char_sizes_list[b]);

				fprintf(write_ini, "\n\n");
				
				FontSize = Size_List[Loaded_Font][boucle]; 
				
				pen.x = 1;
				pen.y += FontSize*2;
			}


			printf(" - Closing INI file ... ");
			fclose(write_ini);
			printf("[OK]\n");
			
			printf(" - Convert 32 bits to 8 bits ... ");
			
			
			int index = 0;
			for ( int y = 0; y < HEIGHT; y++ )
			{
				for ( int x = 0; x < WIDTH; x++ )
				{
					//int r = (int) (pixels[y][x] & 0xFF0000);
					//int g = (int) (pixels[y][x] & 0x00FF00);
					int b = (int) (pixels[y][x] & 0x0000FF);
					
					if(b > 0)
					{
						pixels_8[index+0] = 255;
						pixels_8[index+1] = 255;
						pixels_8[index+2] = 255;
					}
					pixels_8[index+3] = b; // alpha
					
					index += 4;

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
		

			printf(" - Cleaning buffers.");

			int indexclean = 0;
			for ( int y = 0; y < HEIGHT; y++ )
			{
				for ( int x = 0; x < WIDTH; x++ )
				{
					
					pixels[y][x] = 0;
					
				}
			}

			printf(".. ");

			for(int indexclean = 0; indexclean < HEIGHT * WIDTH * CHANNEL_NUM; indexclean++)
				pixels_8[index] = 0;

		} // Scope LIB
		printf("[OK]\n");

		Loaded_Font++;
	} // while

	printf(" Bye! \n\r");
	return 0;
}

