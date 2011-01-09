/* 
 *
 *  Copyright (c) 2010 Arthur Huillet
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
 * Generate a texture atlas from a list of PNG files.
 */
#include "pngfuncs.h"
#include "../src/system.h"

#define OFFSET_FILE_OFFSETX_STRING "OffsetX="
#define OFFSET_FILE_OFFSETY_STRING "OffsetY="

const char **input_files;
int image_count;

struct img { 
   SDL_Surface *s;
   const char *f;
   int xoff;
   int yoff;
};

struct img *images;

const char *image_out_path;
SDL_Surface *atlas_surf;
const char *output_path;
FILE *atlas_file;

int atlas_width = 1024;
int atlas_height = 1024;
int atlas_num = 1;

int total_image_area = 0;

int last_y = 0;

static void init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		exit(1);
	}

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);
}

static void init_output(int atlas_num)
{
	atlas_surf = SDL_CreateRGBSurface(0, atlas_width, atlas_height, 32, 0xff, 0xff00, 0xff000000, 0xff000000);
	if (!atlas_surf) {
		fprintf(stderr, "Unable to create atlas surface: %s.\n", SDL_GetError());
		exit(1);
	}

	SDL_SetAlpha(atlas_surf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	SDL_SetColorKey(atlas_surf, 0, 0);

	if (!atlas_file) {
		atlas_file = fopen(output_path, "w");
	}
	
	char path[2048];
	sprintf(path, "%s%d.png", image_out_path, atlas_num);
	fprintf(atlas_file, "* %s size %d %d\n", path, atlas_width, atlas_height);

	total_image_area = 0;
}

static void finish_output(int atlas_num)
{
	char path[2048];
	sprintf(path, "%s%d.png", image_out_path, atlas_num);
	png_save_surface(path, atlas_surf);
	SDL_FreeSurface(atlas_surf);

	printf("Atlas %d created. Last y position is %d. Atlas size: %dkB, images size: %dkB. Packing efficiency is %f.\n", atlas_num, last_y, (atlas_width * atlas_height * 4) / 1024, (total_image_area * 4) / 1024, (float)total_image_area/(float)(atlas_width * atlas_height));
}


static void place_image_at(int x, int y, struct img *i)
{
	fprintf(atlas_file, "%s %d %d %d %d off %d %d\n", i->f, x, y, i->s->w, i->s->h, i->xoff, i->yoff);
	SDL_Rect target = { .x = x, .y = y };
	SDL_BlitSurface(i->s, NULL, atlas_surf, &target);
	total_image_area += i->s->w * i->s->h;
}

static void next_line(int *x, int *y, int *max_h)
{
	*x = 0;
	*y += *max_h + 1;
	*max_h = 0;
}

static int check_position(int x, int y, SDL_Surface *s)
{
	// Check limit on the right
	if (x + s->w >= atlas_width)
		return 1;

	// Check limit at bottom
	if (y + s->h >= atlas_height) {
		fprintf(stderr, "%dx%d atlas is too small, creating another image.\n", atlas_width, atlas_height);
		return 2;
	}

	return 0;
}

static void create_atlas()
{
	int x = 0;
	int y = 0;
	int max_h = 0;
	int i;

	for (i = 0; i < image_count; i++) {
		// Place image
		int check = check_position(x, y, images[i].s);
		  
		if (check == 2) {
			// Create new atlas
			x = 0;
			y = 0;
			max_h = 0;
			finish_output(atlas_num);
			atlas_num++;
			init_output(atlas_num);

			// Loop again
			i--;
			continue;
		} else if (check == 1)	{
			next_line(&x, &y, &max_h);

			// Loop again
			i--;
			continue;
		}

		place_image_at(x, y, &images[i]);

		// Update max height for this line
		if (max_h < images[i].s->h)
			max_h = images[i].s->h;

		// Compute next position for image
		x += images[i].s->w + 1;
		last_y = y + max_h;
	}

}

static void get_offset_for_image(struct img *img)
{
	char offset_file_name[10000];
	FILE *OffsetFile;
	char *dat;
	char *p;
	
	strcpy(offset_file_name, img->f);
	offset_file_name[strlen(offset_file_name) - 4] = 0;
	strcat(offset_file_name, ".offset");

	if ((OffsetFile = fopen(offset_file_name, "rb")) == NULL) {
		img->xoff = 0;
		img->yoff = 0;
		return;
	}

	dat = calloc(1, 4000);

	fread(dat, 3999, 1, OffsetFile);
	fclose(OffsetFile);

	p = strstr(dat, OFFSET_FILE_OFFSETX_STRING);
	p += strlen(OFFSET_FILE_OFFSETX_STRING);

	img->xoff = atoi(p);

	p = strstr(dat, OFFSET_FILE_OFFSETY_STRING);
	p += strlen(OFFSET_FILE_OFFSETY_STRING);

	img->yoff = atoi(p);

	free(dat);
}

void load_images()
{
	int i;

	for (i = 0; i < image_count; i++) {
		images[i].f = input_files[i];
		images[i].s = IMG_Load(input_files[i]);
		if (!images[i].s) {
			fprintf(stderr, "Unable to load image %s: %s\n", input_files[i], IMG_GetError());
			exit(1);
		}
		SDL_SetAlpha(images[i].s, 0, 255);
		SDL_SetColorKey(images[i].s, 0, 0);
		
		get_offset_for_image(&images[i]);
	}
}

static int compare_images_height(const void *i1, const void *i2)
{
	SDL_Surface *img1 = ((struct img *)i1)->s;
	SDL_Surface *img2 = ((struct img *)i2)->s;

	if (img1->h > img2->h)
		return -1;
	else if (img1->h == img2->h)
		return 0;
	else return 1;
}

void sort_images()
{
	qsort(&images[0], image_count, sizeof(images[0]), compare_images_height); 
}

static int check_output_name(const char *name)
{
	if (strstr(name, image_out_path) == name)
		return 1;

	return 0;
}

int main(int argc, char **argv)
{
	// Read commandline
	if (argc < 6) {
		fprintf(stderr, "Usage: %s <image_output_basename> <atlas_width> <atlas_height> <output_file> <image1.png> <image2.png>...\n", argv[0]);
		exit(1);
	}

	image_out_path = argv[1];
	atlas_width = atoi(argv[2]);
	atlas_height = atoi(argv[3]);
	output_path = argv[4];

	int input_files_nb = argc - 5;
	input_files = (const char **)calloc(input_files_nb, sizeof(const char *));
	images = (struct img *)calloc(input_files_nb, sizeof(struct img));
	int i;
	for (i = 0; i < input_files_nb; i++) {
		if (check_output_name(argv[i + 5])) {
			fprintf(stderr, "Warning: not putting %s in the atlas as it seems to be an output file.\n", argv[i + 5]);
			continue;
		}
		input_files[image_count++] = argv[i + 5];
	}

	init_sdl();
	init_output(1);

	printf("Creating atlas in %s from %d images.\n", output_path, image_count);

	load_images();
	sort_images();
	create_atlas();

	finish_output(atlas_num);
	return 0;
}
