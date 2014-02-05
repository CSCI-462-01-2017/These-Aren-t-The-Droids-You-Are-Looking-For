/***********************************************************/
/*                                                         */
/*   BFONT.c v. 1.0.3 - Billi Font Library by Diego Billi  */
/*   Heavily modified for FreedroidRPG needs over years    */
/*                                                         */
/***********************************************************/

#define _bfont_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/* Current font */
static BFont_Info *CurrentFont;

#define FIRST_FONT_CHAR '!'

/**
 * Initialize a font.
 */
static void find_character_positions(BFont_Info *font, SDL_Rect char_rect[MAX_CHARS_IN_FONT])
{
	unsigned int x = 0, i = 0, y = 0, max_h = 1;
	SDL_Rect *rect;
	SDL_Surface *font_surf = font->font_image.surface;
	
	i = FIRST_FONT_CHAR;
	int sentry_horiz = SDL_MapRGB(font_surf->format, 255, 0, 255);
	int sentry_vert = SDL_MapRGB(font_surf->format, 0, 255, 0);

	if (SDL_MUSTLOCK(font_surf))
		SDL_LockSurface(font_surf);

	while (1) {
		if (i == MAX_CHARS_IN_FONT)
			break;

		// Read this line of characters
		while (x < font_surf->w - 1 && i < MAX_CHARS_IN_FONT) {
			if (sdl_get_pixel(font_surf, x, y) != sentry_horiz) {
				// Found a character
				rect = &char_rect[i];
				rect->x = x;
				rect->y = y;

				// Compute character width
				int x2 = x;
				while (x2 < font_surf->w) {
					if (sdl_get_pixel(font_surf, x2, y) == sentry_horiz)
						break;
					x2++;
				}
				rect->w = x2 - x;

				if (x2 == font_surf->w)
					break;

				// Compute character height
				int y2 = y;
				while (y2 < font_surf->h) {
					if (sdl_get_pixel(font_surf, x, y2) == sentry_horiz ||
							sdl_get_pixel(font_surf, x, y2) == sentry_vert)
						break;
					y2++;
				}
				rect->h = y2 - y;

				// Update maximal h
				if (max_h < y2 - y)
					max_h = y2 - y;

				// Move on to the next character
				i++;
				x = x2;
			} else {
				// On a sentry? Move right.
				x++;
			}
		}

		// Find the next line of characters
		y += max_h + 1;
		max_h = 1;
		x = 0;

		if (y >= font_surf->h)
			break;	

	}

	if (SDL_MUSTLOCK(font_surf))
		SDL_UnlockSurface(font_surf);

	// Set "space" character width
	char_rect[' '].w = char_rect['!'].w;
	char_rect[' '].h = char_rect['!'].h;
	
	// We assume a constant font height
	font->h = char_rect['!'].h;
	font->number_of_chars = i;
}


/**	
 * Prepare font for rendering: create struct image for each character in the font.
 */
static void prepare_font(BFont_Info *font, SDL_Rect char_rect[MAX_CHARS_IN_FONT])
{
	int i;

	if (use_open_gl) {
		make_texture_out_of_surface(&font->font_image);
	}

	// Create each char_image
	for (i = FIRST_FONT_CHAR; i < font->number_of_chars; i++) {
		struct image *img = &font->char_image[i];

		create_subimage(&font->font_image, img, &char_rect[i]);
	}
	
	// Space is a special case	
	create_subimage(&font->font_image, &font->char_image[' '], &char_rect[' ']);
	
	// Delete font_image SDL surface
	free_image_surface(&font->font_image);
}

/**
 * Load the font and stores it in the BFont_Info structure 
 */
BFont_Info *LoadFont(const char *filename)
{
	BFont_Info *font = MyMalloc(sizeof(BFont_Info));

	// Load the font image
	load_image_surface(&font->font_image, filename, FALSE);

	// Find character coordinates in the image
	SDL_Rect char_rect[MAX_CHARS_IN_FONT];
	memset(char_rect, 0, sizeof(char_rect));
	find_character_positions(font, char_rect);

	// Prepare the data structures according to the rendering mode
	prepare_font(font, char_rect);	
	
	return font;
}

/**
 * Set the current font 
 */
void SetCurrentFont(BFont_Info * Font)
{
	CurrentFont = Font;
};				// void SetCurrentFont (BFont_Info * Font)

/**
 * Returns the pointer to the current font strucure in use 
 */
BFont_Info *GetCurrentFont(void)
{
	return CurrentFont;
};				// BFont_Info * GetCurrentFont (void)

/**
 * Return the font height 
 */
int FontHeight(BFont_Info * Font)
{
	return (Font->h);
};				// int FontHeight (BFont_Info * Font)

/**
 * Return the width of specified character
 */
int CharWidth(BFont_Info * Font, unsigned char c)
{
	if (c < ' ' || c > Font->number_of_chars - 1)
		c = '.';
	return Font->char_image[c].w;
}

/**
 * Get letter-spacing for specified font.
 *
 * Letter-spacing refers to the overall spacing of a word or block of text
 * affecting its overall density and texture.
 */
int get_letter_spacing(BFont_Info *font) {
	if (font == FPS_Display_BFont || font == Blue_BFont || font == Red_BFont)
		return -2;
	else if (font == Menu_BFont)
		return -4;
	else
		return 0;
}

/**
 * Handle font switching on special characters (\x) or formatting tags (aka bbcodes).
 * Returns 1 if the font was changed and 0 if it was not.
 */
int handle_switch_font_char(char **ptr)
{
	if (**ptr == '[' && *(*ptr+2) != ']')
		return FALSE;

	int index = 0;
	int incr = 1;

	if (**ptr == '[') {
		(*ptr)++;
		index = 2;
		incr = 2;
	}

	if (**ptr == font_switchto_red[index]) {
		SetCurrentFont(Red_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_blue[index]) {
		SetCurrentFont(Blue_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_neon[index]) {
		SetCurrentFont(FPS_Display_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_msgstat[index]) {
		SetCurrentFont(Messagestat_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_msgvar[index]) {
		SetCurrentFont(Messagevar_BFont);
		(*ptr) += incr;
		return TRUE;
	}

	return FALSE;
}

/**
 * Puts a single char on the surface with the specified font 
 */
int put_char(BFont_Info *font, int x, int y, unsigned char c)
{
	SDL_Rect dest;
	SDL_Rect clipping_rect;
	struct image *img;

	dest.w = CharWidth(font, ' ');
	dest.h = FontHeight(font);
	dest.x = x;
	dest.y = y;

	if (c < ' ' || c > font->number_of_chars - 1)
		c = '.';

	img = &font->char_image[c];

	if ((c != ' ') && (c != '\n')) {
		if (use_open_gl) {
			SDL_GetClipRect(Screen, &clipping_rect);

			if ((dest.x < clipping_rect.x + clipping_rect.w) && (dest.x >= clipping_rect.x)) {
				display_image_on_screen(img, dest.x, dest.y, IMAGE_NO_TRANSFO);
			}
		} else {
			display_image_on_screen(img, dest.x, dest.y, IMAGE_NO_TRANSFO);
		}
	}

	return CharWidth(font, c);

}

/**
 * Write a string on a surface using specified font, taking letter-spacing
 * into account.
 */
void put_string(BFont_Info *font, int x, int y, const char *text)
{
	char *ptr = (char *)text;

	SetCurrentFont(font);

	if (use_open_gl) {
		// Set up clipping
		SDL_Rect clip_rect;
		SDL_GetClipRect(Screen, &clip_rect);

		set_gl_clip_rect(&clip_rect);
	}

	start_image_batch();

	int letter_spacing = get_letter_spacing(GetCurrentFont());

	while (*ptr != '\0') {
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(GetCurrentFont());
			continue;
		}
		x += put_char(GetCurrentFont(), x, y, *ptr) + letter_spacing;
		ptr++;
	}

	end_image_batch();

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		unset_gl_clip_rect();
	}
#endif
}

/**
 * Calculate the width of a string using a certain font, taking letter-spacing
 * into account.
 */
int text_width(BFont_Info *font, const char *text)
{
	char *ptr = (char *)text;
	int width = 0;
	int letter_spacing = get_letter_spacing(font);

	while (*ptr != '\0') {
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(font);
			continue;
		}
		width += CharWidth(font, *ptr) + letter_spacing;
		ptr++;
	}
	return width;
}

/**
 *
 *
 */
int limit_text_width(BFont_Info *font, const char *text, int limit)
{
	char *ptr = (char *)text;
	int width = 0;
	int letter_spacing = get_letter_spacing(font);

	while (*ptr != '\0') {
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(font);
			continue;
		}
		width += CharWidth(font, *ptr) + letter_spacing;
		ptr++;
		if (width >= limit)
			return (ptr - text);
	}
	return -1;
}

void put_string_centered(BFont_Info *font, int y, const char *text)
{
	put_string(font, Screen->w / 2 - text_width(font, text) / 2, y, text);
}

void put_string_right(BFont_Info *font, int y, const char *text)
{
	put_string(font, Screen->w - text_width(font, text) - 13, y, text);
}

void put_string_left(BFont_Info *font, int y, const char *text)
{
	put_string(font, 13, y, text);
}

#undef _bfont_c
