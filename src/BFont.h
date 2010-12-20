/************************************************************/
/*                                                          */
/*   BFONT.h v. 1.0.3 - Billi Font Library by Diego Billi   */
/*                                                          */
/************************************************************/

#include "system.h"

#define MAX_CHARS_IN_FONT 256

typedef struct BFont_Info {
	/* font height */
	int h;
	
	struct image font_image;
	struct image char_image[MAX_CHARS_IN_FONT];
	unsigned int number_of_chars;
} BFont_Info;

/* Load and store le font in the BFont_Info structure */
BFont_Info *LoadFont(char *filename);

/* Returns a pointer to the current font structure */
BFont_Info *GetCurrentFont(void);

/* Set the current font */
void SetCurrentFont(BFont_Info * Font);

/* Returns the font height */
int FontHeight(BFont_Info * Font);

/* Returns the character width of the specified font */
int CharWidth(BFont_Info * Font, unsigned char c);

/* Get letter-spacing for specified font. */
int get_letter_spacing(BFont_Info *font);

/* Handle font switching on special characters. */
int handle_switch_font_char(unsigned char c);

/* Write a single character on the "Surface" with the specified font */
int PutCharFont(SDL_Surface * Surface, BFont_Info * Font, int x, int y, unsigned char c);

/* Returns the width, in pixels, of the text calculated with the current font*/
int TextWidth(char *text);

/* Returns the width, in pixels, of the text calculated with the specified font*/
int TextWidthFont(BFont_Info * Font, char *text);

/* Returns the index of the last character than is inside the width limit, with the current font */
int LimitTextWidth(char *text, int limit);

/* Write a string on the "Surface" with the current font */
void PutString(SDL_Surface *surface, int x, int y, char *text);

/* Write a string on the "Surface" with the specified font */
void PutStringFont(SDL_Surface *surface, BFont_Info *font, int x, int y, char *text);

/* Write a left-aligned string on the "Surface" with the current font */
void LeftPutString(SDL_Surface * Surface, int y, char *text);

/* Write a left-aligned string on the "Surface" with the specified font */
void LeftPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text);

/* Write a center-aligned string on the "Surface" with the current font */
void CenteredPutString(SDL_Surface * Surface, int y, char *text);

/* Write a center-aligned string on the "Surface" with the specified font */
void CenteredPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text);

/* Write a right-aligned string on the "Surface" with the specified font */
void RightPutString(SDL_Surface * Surface, int y, char *text);

/* Write a right-aligned string on the "Surface" with the specified font */
void RightPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text);

/* Write a justify-aligned string on the "Surface" with the specified font */
void JustifiedPutString(SDL_Surface * Surface, int y, char *text);

/* Write a justify-aligned string on the "Surface" with the specified font */
void JustifiedPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text);

/* The following functions do the same task but have the classic "printf" syntax */
void PrintString(SDL_Surface * Surface, int x, int y, const char *fmt, ...);
