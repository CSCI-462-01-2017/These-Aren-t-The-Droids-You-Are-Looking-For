/*
 *
 *   Copyright (c) 2010 Arthur Huillet
 *   Copyright (c) 2003 Johannes Prix
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains function relevant for OpenGL based graphics output.
 */

#define _open_gl_c 1

#include <math.h>
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

// 28 degress is the magic angle for our iso view
#define COS_28 0.88294759
#define SIN_28 0.46947156

int gl_max_texture_size;

static unsigned int pbo = 0;

/**
 * This is a wrapper for the SDL_Flip function, that will use either the
 * OpenGL buffer-swapping or the classic SDL flipper, depending on the
 * current output method, like OpenGL or not.
 */
int our_SDL_flip_wrapper()
{
#ifdef DEBUG_QUAD_BORDER
	debug_quad_border_seed = 0;
#endif

	if (use_open_gl)
		SDL_GL_SwapBuffers();
	else
		return (SDL_Flip(Screen));

	return (0);
};				// int our_SDL_flip_wrapper ( SDL_Surface *screen )

/**
 *
 *
 */
void our_SDL_update_rect_wrapper(SDL_Surface * screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if (use_open_gl) {
		our_SDL_flip_wrapper();
	} else {
		SDL_UpdateRect(screen, x, y, w, h);
	}
};				// void our_SDL_update_rect_wrapper ( SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h ) 

/**
 * Simon N.:  ISO functions.  these draw quads in the 3D planes
 */
void drawISOXYQuad(int x, int y, int z, int w, int h)
{
#ifdef HAVE_LIBGL
	glVertex3f(x, y - h, z);
	glVertex3f(x, y, z);
	glVertex3f(x + w * COS_28, y + w * SIN_28, z);
	glVertex3f(x + w * COS_28, y - h + w * SIN_28, z);
#endif
}

void drawISOXZQuad(int x, int y, int z, int w, int d)
{
#ifdef HAVE_LIBGL
	glVertex3f(x + d * COS_28, y - d * SIN_28, z);
	glVertex3f(x, y, z);
	glVertex3f(x + w * COS_28, y + w * SIN_28, z);
	glVertex3f(x + w * COS_28 + d * COS_28, y + w * SIN_28 - d * SIN_28, z);
#endif
}

void drawISOYZQuad(int x, int y, int z, int h, int d)
{
#ifdef HAVE_LIBGL
	glVertex3f(x, y - h, z);
	glVertex3f(x, y, z);
	glVertex3f(x + d * COS_28, y - d * SIN_28, z);
	glVertex3f(x + d * COS_28, y - h - d * SIN_28, z);
#endif
}

/**
 * Simon N.: Draws an isometric energy bar.
 * x,y,z : the position of the lower left hand corner
 * h : the height of the energy bar, as if viewed in the X direction
 * d : the depth of the energy bar, as if viewed in the X direction
 * fill : the percentage the energy bar is filled
 * c1 : the fill color
 * c1 : the background color
 */
void drawIsoEnergyBar(int x, int y, int z, int h, int d, int length, float fill, myColor * c1, myColor * c2)
{
#ifdef HAVE_LIBGL
	int l = (int)(fill * length);
	int l2 = (int)length * (1.0 - fill);
	int lcos, lsin, l2cos, l2sin;

	end_image_batch(__FUNCTION__);

	glColor4ub(c1->r, c1->g, c1->b, c1->a);
	use_shader(NO_SHADER);
	glBegin(GL_QUADS);

	lcos = (int)rint(l * COS_28);
	lsin = (int)rint(l * SIN_28);
	// think of this a dcos, same reason above
	l2cos = (int)rint(d * COS_28);
	l2sin = (int)rint(d * SIN_28);
	drawISOXYQuad(x, y, z, d, h);
	drawISOYZQuad(x + l2cos, y + l2sin, z, h, l);
	drawISOXZQuad(x, y - h, z, d, l);

	glColor4ub(c2->r, c2->g, c2->b, c2->a);
	drawISOYZQuad(x + l2cos + lcos, y + l2sin - lsin, z, h, l2);
	drawISOXZQuad(x + lcos, y - lsin - h, z, d, l2);

	glEnd();
	glColor4ub(255, 255, 255, 255);
#endif
};				// void drawIsoEnergyBar(int dir, int x, int y, int z, int h, int d, int length, float fill, myColor *c1, myColor *c2  ) 

/**
 * This function flips a given SDL_Surface.
 * 
 * This is particularly necessary, since OpenGL has a different native
 * coordinate system than SDL and therefore images often appear flipped
 * around if one doesn't counter this effect with OpenGL by flipping the
 * images just once more in the same fashion.  That is what this function
 * does.
 */
void flip_image_vertically(SDL_Surface * tmp1)
{
	SDL_LockSurface(tmp1);

	int nHH = tmp1->h >> 1;
	int nPitch = tmp1->pitch;

	unsigned char pBuf[nPitch + 1];
	unsigned char *pSrc = (unsigned char *)tmp1->pixels;
	unsigned char *pDst = (unsigned char *)tmp1->pixels + nPitch * (tmp1->h - 1);

	while (nHH--) {
		memcpy(pBuf, pSrc, nPitch);
		memcpy(pSrc, pDst, nPitch);
		memcpy(pDst, pBuf, nPitch);

		pSrc += nPitch;
		pDst -= nPitch;
	}

	SDL_UnlockSurface(tmp1);

};				// void flip_image_vertically ( SDL_Surface* tmp1 ) 

/**
 *
 *
 */
SDL_Surface *our_IMG_load_wrapper(const char *file)
{
	SDL_Surface *surf;

	surf = IMG_Load(file);

	if (surf == NULL) {
		error_message(__FUNCTION__, "IMG_Load returned NULL. IMG_GetError() : %s.", PLEASE_INFORM, IMG_GetError());
		return (NULL);
	}

	if (use_open_gl) {
//		flip_image_vertically(surf);
	}

	return surf;
}

/**
 * Compute the dimensions of the smaller power-of-two-sized texture
 * that will contain the given image. Used on older GPUs that do not
 * support non-power-of-two-textures with the regular GL_TEXTURE_2D target.
 * (ARB_texture_non_power_of_two).
 */
static void compute_POT_texture_dimensions(struct image *img)
{
	int txw = 1;
	int txh = 1;

	while (txw < img->w)
		txw <<= 1;
	while (txh < img->h)
		txh <<= 1;

	img->tex_w = txw;
	img->tex_h = txh;
}


/**
 * If OpenGL is in use, we need to make textured quads out of our normal
 * SDL surfaces.
 */
#ifdef HAVE_LIBGL
void make_texture_out_of_surface(struct image *img)
{
	void *data_ptr = img->surface->pixels;
	void *data_ptr_pbo = (pbo) ? NULL : data_ptr;

	// Stop any image batch being constructed, if relevant
	end_image_batch(__FUNCTION__);

	if (!img->texture) {
		glGenTextures(1, &img->texture);
	}

	img->texture_type = TEXTURE_CREATED;
	DebugPrintf(1, "Using texture %d\n", img->texture);

	glBindTexture(GL_TEXTURE_2D, img->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (pbo) {
		glBufferData(GL_PIXEL_UNPACK_BUFFER, img->w*img->h*4, data_ptr, GL_STREAM_DRAW);
	}

	if (!GLEW_ARB_texture_non_power_of_two) {
		// If the GPU does not support NPOT textures, create a larger texture
		compute_POT_texture_dimensions(img);

		if (pbo) {
			// Unbind the PBO to create the texture with undefined storage, this way the NULL
			// argument means "create undefined storage", not "read data from PBO"
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->tex_w, img->tex_h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		if (pbo) {
			// Re-bind PBO for actual data transfer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		}

		// Transfer the data to the texture, leaving the padding undefined
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, img->tex_h - img->h, img->w, img->h, GL_BGRA, GL_UNSIGNED_BYTE, data_ptr_pbo);
	} else {
		// Generate the texture - XXX could use TexStorage but assess performance&availability
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data_ptr_pbo);
		img->tex_w = img->w;
		img->tex_h = img->h;
	}

	img->tex_x0 = 0.0;
	img->tex_y0 = 1.0 - (float)img->h / (float)img->tex_h;
	img->tex_x1 = (float)img->w / (float)img->tex_w;
	img->tex_y1 = 1.0;

	SDL_FreeSurface(img->surface);
	img->surface = NULL;
	open_gl_check_error_status(__FUNCTION__);
}
#endif

static void safely_set_open_gl_viewport_and_matrix_mode(void)
{
#ifdef HAVE_LIBGL

	glViewport(0, 0, GameConfig.screen_width, GameConfig.screen_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GameConfig.screen_width, GameConfig.screen_height, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);

	open_gl_check_error_status(__FUNCTION__);

#endif
}

/**
 * This function does the second part of the OpenGL parameter 
 * initialization.  We've made this chunk of code into a separate function
 * such that the frequent issues with OpenGL drivers can be attributed to
 * a particular spot in the code more easily.
 */
void safely_set_some_open_gl_flags_and_shade_model(void)
{
#ifdef HAVE_LIBGL

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glDisable(GL_TEXTURE_2D);

	glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Create a pixel buffer object if supported
	if (GLEW_VERSION_2_1 || GLEW_ARB_pixel_buffer_object || GLEW_EXT_pixel_buffer_object) {
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	open_gl_check_error_status(__FUNCTION__);

#endif
}

/**
 * Initialize the OpenGL interface.
 */
int init_open_gl(void)
{
#ifdef HAVE_LIBGL
	GLenum err = glewInit();
	if (err	!= GLEW_OK) {
		error_message(__FUNCTION__, "glewInit failed: %s\n", IS_FATAL, glewGetErrorString(err));
	}
	init_opengl_debug();
	safely_set_open_gl_viewport_and_matrix_mode();
	safely_set_some_open_gl_flags_and_shade_model();
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
	open_gl_check_error_status(__FUNCTION__);

	init_shaders();
#endif

	return TRUE;
}

/**
 * This function restores the menu background, that must have been stored
 * before using the function of similar name.
 */
void RestoreMenuBackground(int backup_slot)
{
	if (use_open_gl) {
#ifdef HAVE_LIBGL
		int h = GameConfig.screen_height;
		int w = GameConfig.screen_width;

		// Stop any image batch being constructed, 
		// so that struct image does not get confused.
		end_image_batch(__FUNCTION__);

		use_shader(NO_SHADER);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, (StoredMenuBackgroundTex[backup_slot]));

		glBegin(GL_QUADS);
		glTexCoord2i(0, h);
		glVertex2i(0, 0);
		glTexCoord2i(0, 0);
		glVertex2i(0, h);
		glTexCoord2i(w, 0);
		glVertex2i(w, h);
		glTexCoord2i(w, h);
		glVertex2i(w, 0);
		glEnd();

		glDisable(GL_TEXTURE_RECTANGLE_ARB);

#endif
	} else {
		SDL_BlitSurface(StoredMenuBackground[backup_slot], NULL, Screen, NULL);
	}
}

/**
 * This function stores the current background as the background for a
 * menu, so that it can be refreshed much faster than by reassembling it
 * every frame.
 */
void StoreMenuBackground(int backup_slot)
{
	static int first_call = TRUE;

	if (first_call) {
		StoredMenuBackground[0] = NULL;
		StoredMenuBackground[1] = NULL;
		first_call = FALSE;
	}

	if (use_open_gl) {
#ifdef HAVE_LIBGL
		// Stop any image batch being constructed, 
		// so that struct image does not get confused.
		end_image_batch(__FUNCTION__);

		glFinish();

		if (StoredMenuBackgroundTex[backup_slot] == 0) {
			glGenTextures(1, &StoredMenuBackgroundTex[backup_slot]);
		}

		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, StoredMenuBackgroundTex[backup_slot]);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glCopyTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 0, 0, GameConfig.screen_width, GameConfig.screen_height, 0);
		open_gl_check_error_status(__FUNCTION__);

		glDisable(GL_TEXTURE_RECTANGLE_ARB);

#endif
	} else {
		// If the memory was not yet allocated, we need to do that now...
		//
		// otherwise we free the old surface and create a new copy of the
		// current screen content...
		//
		if (StoredMenuBackground[backup_slot] == NULL) {
			StoredMenuBackground[backup_slot] = SDL_DisplayFormat(Screen);
		} else {
			SDL_FreeSurface(StoredMenuBackground[backup_slot]);
			StoredMenuBackground[backup_slot] = SDL_DisplayFormat(Screen);
		}
	}

};				// void StoreMenuBackground ( int backup_slot )

/**
 * Following a suggestion from Simon, we're now implementing one single
 * small texture (to be modified with pixel operations every frame) that
 * can be stretched out over the whole screen via OpenGL.
 * This function is here to set up the texture in the first place.
 */
void set_up_stretched_texture_for_light_radius(void)
{
#ifdef HAVE_LIBGL

	static int texture_is_set_up_already = FALSE;

	// In the non-open-gl case, this function shouldn't be called ever....
	//
	if (!use_open_gl)
		return;

	// Stop any image batch being constructed, 
	// so that struct image does not get confused.
	end_image_batch(__FUNCTION__);

	// Some protection against creating this texture twice...
	//
	if (texture_is_set_up_already)
		return;
	texture_is_set_up_already = TRUE;

	// We create an SDL surface, so that we can make the texture for the
	// stretched-texture method light radius from it...
	//
	light_radius_stretch_surface =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, LightRadiusConfig.texture_w, LightRadiusConfig.texture_h, 32, rmask, gmask, bmask, amask);

	// Having prepared the raw image it's now time to create the real
	// textures.
	//
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &light_radius_stretch_texture);

	glBindTexture(GL_TEXTURE_2D, (light_radius_stretch_texture));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Generate The Texture 
	if (pbo) {
		glBufferData(GL_PIXEL_UNPACK_BUFFER,
				light_radius_stretch_surface->w * light_radius_stretch_surface->h * 4,
				light_radius_stretch_surface->pixels, GL_STREAM_DRAW);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 4,
			light_radius_stretch_surface->w, light_radius_stretch_surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE,
			(pbo) ? NULL : light_radius_stretch_surface->pixels);

#endif

};				// void set_up_stretched_texture_for_light_radius ( void )

/**
 * This function updated the automap texture, such that all info from the
 * current square is on the automap.
 */
void light_radius_update_stretched_texture(void)
{
#ifdef HAVE_LIBGL
	int x, y;
	int red = 0;
	int blue = 0;
	int green = 0;
	int alpha = 0;
	int light_strength;
	static float alpha_factor = 255.0 / (float)NUMBER_OF_SHADOW_IMAGES;
	// Now it's time to edit the automap texture.
	//
	for (y = 0; y < LightRadiusConfig.cells_h; y++) {
		for (x = 0; x < LightRadiusConfig.cells_w; x++) {
			light_strength = get_light_strength_cell(x, y);

			alpha = 255 - (alpha_factor) * ((float)light_strength);

			sdl_put_pixel(light_radius_stretch_surface, x, y, red, green, blue, alpha);

		}
	}

	// Stop any image batch being constructed, 
	// so that struct image does not get confused.
	end_image_batch(__FUNCTION__);

	glBindTexture(GL_TEXTURE_2D, light_radius_stretch_texture);
	if (pbo) {
		glBufferData(GL_PIXEL_UNPACK_BUFFER, LightRadiusConfig.texture_w * LightRadiusConfig.texture_h * 4,
				light_radius_stretch_surface->pixels, GL_STREAM_DRAW);
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			LightRadiusConfig.texture_w, LightRadiusConfig.texture_h, GL_RGBA, GL_UNSIGNED_BYTE,
			(pbo) ? NULL : light_radius_stretch_surface->pixels);

	open_gl_check_error_status(__FUNCTION__);

#endif

};				// void light_radius_update_stretched_texture ( void ) 

/**
 * Following a suggestion from Simon, we're now implementing one single
 * small texture (to be modified with pixel operations every frame) that
 * can be stretched out over the whole screen via OpenGL.
 * This function is here to set up the texture in the first place.
 *
 * decay_x and decay_y does translate the textured rectangle to avoid
 * darkness flickering. See flicker-free code's note in set_up_light_strength_buffer()
 */
void blit_open_gl_stretched_texture_light_radius(int decay_x, int decay_y)
{
#ifdef HAVE_LIBGL
	struct image local_iso_image;

	// We make sure, that there is one single texture created before
	// doing any of our texture-blitting or texture-modification stuff
	// with it.
	//
	set_up_stretched_texture_for_light_radius();

	light_radius_update_stretched_texture();

	// Now we blit the current automap texture to the screen.  We use standard
	// texture blitting code for this, so we need to embed the automap texture
	// in a surrounding 'iso_image', but that shouldn't be costly or anything...
	//
	local_iso_image.texture = light_radius_stretch_texture;
	local_iso_image.tex_w = LightRadiusConfig.texture_w;
	local_iso_image.tex_h = LightRadiusConfig.texture_h;
	local_iso_image.w = LightRadiusConfig.texture_w;
	local_iso_image.h = LightRadiusConfig.texture_h;
	local_iso_image.texture_type = TEXTURE_CREATED | IS_SUBTEXTURE;
	local_iso_image.offset_x = 0;
	local_iso_image.offset_y = 0;
	local_iso_image.tex_x0 = 0.0;
	local_iso_image.tex_y0 = 0.0;
	local_iso_image.tex_x1 = 1.0;
	local_iso_image.tex_y1 = 1.0;

	display_image_on_screen(&local_iso_image, decay_x, decay_y, IMAGE_SCALE_TRANSFO(LightRadiusConfig.scale_factor));

#endif

};				// void blit_open_gl_stretched_texture_light_radius ( void )

/**
 * Draw a colored rectangle on screen, given its 4 points.
 */
void gl_draw_quad(const struct point vertices[4], int r, int g, int b, int a)
{
#ifdef HAVE_LIBGL
	use_shader(NO_SHADER);

	glColor4ub(r, g, b, a);

	glBegin(GL_QUADS);
	glVertex2i(vertices[0].x, vertices[0].y);
	glVertex2i(vertices[1].x, vertices[1].y);
	glVertex2i(vertices[2].x, vertices[2].y);
	glVertex2i(vertices[3].x, vertices[3].y);
	glEnd();

	glColor4ub(255, 255, 255, 255);
#endif
}

static struct background backgrounds[] = {
		{"inventory.png", EMPTY_IMAGE, 0, 0, FALSE },
		{"character.png", EMPTY_IMAGE, -320, 0, FALSE },
		{"SkillScreen.png", EMPTY_IMAGE, -320, 0, FALSE },
		{"SkillExplanationScreen.png", EMPTY_IMAGE, 0, 0, FALSE },
		{"title.jpg", EMPTY_IMAGE, 0, 0,  TRUE },
		{"credits.jpg", EMPTY_IMAGE, 0, 0,  TRUE },
		{"shoppe.jpg", EMPTY_IMAGE, 0, 0, TRUE },
		{"takeover_browser.png", EMPTY_IMAGE, 0, 0, TRUE },
		{"item_browser_shop.png", EMPTY_IMAGE, 0, 0,TRUE },
		{"startup1.jpg", EMPTY_IMAGE, 0, 0,  TRUE },
		{"conversation.png", EMPTY_IMAGE, 0, 0,  TRUE },
		{"transfer.jpg", EMPTY_IMAGE, 0, 0,  TRUE },
		{"number_selector.png", EMPTY_IMAGE, 0, 0,  TRUE },
		{"item_upgrade.png", EMPTY_IMAGE, ITEM_UPGRADE_RECT_X, ITEM_UPGRADE_RECT_Y,  FALSE },
		{"item_upgrade_crafting.png", EMPTY_IMAGE, ADDON_CRAFTING_RECT_X, ADDON_CRAFTING_RECT_Y, FALSE },
        {"console_bg1.jpg", EMPTY_IMAGE, 0, 0, TRUE },
};

struct background *get_background(const char *name)
{
	struct background *bg = NULL;

	// Search the specified background in the list
	int i;
	for (i = 0; i < sizeof(backgrounds)/sizeof(backgrounds[0]); i++) {
		if (!strcmp(backgrounds[i].filename, name)) {
			bg = &backgrounds[i];
			break;
		}
	}

	if (!bg)
		return NULL;

	// Load the background
	if (!image_loaded(&bg->img)) {
		char path[2048];
		sprintf(path, "backgrounds/%s", bg->filename);
		load_image(&bg->img, GRAPHICS_DIR, path, USE_WIDE);
	}

	return bg;
}

/**
 * This function displays a background (or UI element) at a given position in the game.
 */
void blit_background(const char *background)
{
	struct background *bg = get_background(background);
	if (!bg) {
		error_message(__FUNCTION__, "Received a request to display background %s which is unknown. Doing nothing.", PLEASE_INFORM, background);
		return;
	}

	// Compute coordinates and display
	int x = bg->x;
	int y = bg->y;
	
	float scalex = 1.0, scaley = 1.0;
	if (bg->must_scale) {
		scalex = (float)GameConfig.screen_width / bg->img.w;
		scaley = (float)GameConfig.screen_height / bg->img.h;
	}

	x *= scalex;
	y *= scaley;

	if (x < 0)
		x += GameConfig.screen_width;

	if (y < 0)
		y += GameConfig.screen_height;


	gl_debug_marker(__FUNCTION__);
	display_image_on_screen(&bg->img, x, y, set_image_transformation(scalex, scaley, 1.0, 1.0, 1.0, 1.0, 0));
}

void set_gl_clip_rect(const SDL_Rect *clip)
{
#ifdef HAVE_LIBGL
	// Flush image batch. Scissor test shouldn't affect it.
	end_image_batch(__FUNCTION__);

	if (use_open_gl) {
		glScissor(clip->x, GameConfig.screen_height - (clip->y + clip->h), clip->w, clip->h);
		glEnable(GL_SCISSOR_TEST);
	}
#endif
}

void unset_gl_clip_rect(void)
{
#ifdef HAVE_LIBGL
	if (use_open_gl) {
		glDisable(GL_SCISSOR_TEST);
	}
#endif
}

/**
 * Retrieve a bit mask of the OpenGL "quirks" that FreedroidRPG will apply. These are driver- and hardware-dependent workarounds we apply in the driver.
 */
int get_opengl_quirks(void)
{
	static int quirks = 0;
	static int done = 0;

	if (!done) {
		// GLSL-related quirks
		const char *str = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		if (!str || strlen(str) < 3) {
			error_message(__FUNCTION__, "Could not retrieve GL_SHADING_LANGUAGE_VERSION\n", PLEASE_INFORM);
			str = "1.0";
		}

		/* GPUs that are limited to GLSL 1.2 (an arbitrary value chosen to
		   match i915 as present in eeePC 701) are going to have limited
		   abilities in their shaders, such as inability to do conditional
		   branching, and drastic limitations on instruction counts and so
		   on.  One such example is the i915, with a Linux driver that
		   falls back to the CPU if the shader has more than 4 indirect
		   texture lookups - but due to a driver bug, any texture lookup
		   written in GLSL is turned into an indirect lookup. The effective
		   result is that i915 will fall back to CPU when running our
		   standard blitter shader. To avoid this, use a quirk to reduce
		   the number of textures used for the blitter shader to 2.  This
		   problem was observed on i915, but similar problems are expected
		   to appear on any GPU sufficiently old to be limited to GLSL 1.2,
		   so apply this quirk to all such GPUs.
		 */
		if (!strncmp(str, "1.0", 3) || !strncmp(str, "1.1", 3) || !strncmp(str, "1.2", 3)) {
			quirks |= DISABLE_SHADERS;
		}

		done = 1;
	}

	return quirks;
}


#undef _open_gl_c
