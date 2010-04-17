#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"
#include "savestruct.h"

#include "mapgen/mapgen.h"

/* Minimum surface of a room */
static const int Smin = 40;

/* Dimensions of the dungeon */
static int dim_x_init;
static int dim_y_init;

/* Worst y/x and x/y ratios accepted. */
#define WORST_ROOM_RATIO 2.0

enum cut_axis {
	CUT_HORIZONTALLY = 0,
	CUT_VERTICALLY   = 1,
	DO_NOT_CUT       = 2
};

/** Check if the two proposed room sizes satisfy the minimal surface criteria or not.
 */
static int check_surf(int w_creator, int h_creator, int w_newroom, int h_newroom)
{
	if ((w_creator * h_creator > Smin) && (h_newroom * w_newroom > Smin))
		return 1;

	return 0;
}

/**
 * Try to cut a room of dimensions dim_x and dim_y, horizontally or vertically
 * depending on the value of vert, with a ratio r.
 *
 * This function picks a ratio for the cut and sets r accordingly.
 */
static int trycut(int dim_x, int dim_y, int *r, enum cut_axis vert)
{
	/*
	 * Example computation for a vertical cut : 
	 *
	 * x1 + x2 = dim_x
	 * x1 = r * x
	 * x2 = (1-r) * x
	 * log(x1/y) <= b, b constant
	 * log(x1/y) >= -b
	 *
	 * =>  y/(a * x) <= r <= a * y/x 
	 * and 1-(ya)/x <= r <= 1 - y/(ax) 
	 *
	 * with a = exp(b)
	 */
	float rmin = 1, rmin2 = 1, rmax = 0, rmax2 = 0;

	switch (vert) {
	case CUT_HORIZONTALLY:
		rmax = WORST_ROOM_RATIO * (float)dim_x / dim_y;
		rmin2 = (1.0 / WORST_ROOM_RATIO) * dim_x / dim_y;
		break;
	case CUT_VERTICALLY:
		rmax = WORST_ROOM_RATIO * (float)dim_y / dim_x;
		rmin2 = (1.0 / WORST_ROOM_RATIO) * dim_y / dim_x;
		break;
	default:
		;
	}

	rmin = 1 - rmax;
	rmax2 = 1 - rmin2;

	rmax = rmax > rmax2 ? rmax2 : rmax;
	rmin = rmin < rmin2 ? rmin2 : rmin;

	if (rmax < rmin) {
		// No cut possible this way
		return 0;
	}
	// Pick a cut ratio
	*r = 100.0 * (rmin + (rmax - rmin) * ((float)rand() / (float)RAND_MAX));

	return 1;
}

/**
 * Make a decision about a cut in a room: whether to cut at all,
 *  what way to cut, and at what position
 */
static enum cut_axis cut(int dim_x, int dim_y, int *r)
{
	enum cut_axis ret = DO_NOT_CUT;
	// linear probability of cut
	float chancetocut = (1 / (dim_x_init * dim_y_init)) * dim_x * dim_y - (Smin / (dim_x_init * dim_y_init - Smin));
	float p = rand() % 10000 + 1;

	// Test whether to cut at all
	if (p / 10000 < chancetocut) {
		return DO_NOT_CUT;
	}
	// Pick a random direction to cut along
	ret = rand() % 2;

	// Try to cut along this direction
	if (!trycut(dim_x, dim_y, r, ret)) {
		// If we cannot, try the other direction
		if (!trycut(dim_x, dim_y, r, !ret))
			return DO_NOT_CUT;
		else
			ret = !ret;
	}
	// Rooms need to be 4 tiles wide at least
	switch (ret) {
	case CUT_HORIZONTALLY:
		if ((*r / 100.0 * dim_y) < 4 || ((100.0 - *r) / 100.0 * dim_y) < 4)
			return DO_NOT_CUT;
		break;
	case CUT_VERTICALLY:
		if ((*r / 100.0 * dim_x) < 4 || ((100.0 - *r) / 100.0 * dim_x) < 4)
			return DO_NOT_CUT;
		break;
	default:
		;
	}

	return ret;
}

static void deriv_P(int id)
{
	int p;
	int prop;
	int x = rooms[id].x - 1;
	int y = rooms[id].y - 1;
	int creator = id;
	int dim_x = rooms[creator].w + 2;
	int dim_y = rooms[creator].h + 2;
	int newroom;
	p = cut(dim_x, dim_y, &prop);

	int w_creator = dim_x;
	int h_creator = dim_y;
	int w_newroom = dim_x;
	int h_newroom = dim_y;
	int x_newroom = x;
	int y_newroom = y;

	/*
	   Two rules : 
	   P -> P h P | P -> P v P

	   h horizontal cut
	   v vertical cut
	 */
	switch (p) {
	case CUT_HORIZONTALLY:
		h_creator = rooms[creator].h * prop / 100.0;
		h_newroom = dim_y - h_creator + 1;
		y_newroom = y + h_creator - 1;
		break;
	case CUT_VERTICALLY:
		w_creator = rooms[creator].w * prop / 100.0;
		w_newroom = dim_x - w_creator + 1;
		x_newroom = x + w_creator - 1;
		break;
	default:
		return;
	}

	if (!check_surf(w_creator, h_creator, w_newroom, h_newroom)) {
		return;
	}

	newroom = mapgen_add_room(x_newroom, y_newroom, w_newroom, h_newroom);

	rooms[creator].w = w_creator-2;
	rooms[creator].h = h_creator-2;

	mapgen_draw_room(newroom);
	mapgen_draw_room(creator);
	deriv_P(id);
	deriv_P(newroom);
}

/**
 * Is the given position a room corner?
 */
static int is_room_corner(int x, int y)
{
	int tile = mapgen_get_tile(x, y);
	if (tile == TILE_WALL_SW || tile == TILE_WALL_SE ||
			tile == TILE_WALL_NW || tile == TILE_WALL_NE
	   ) 
	{
		return 1;
	}
	return 0;
}

static void adj(struct cplist_t *cplist, int *nx, int *ny)
{
	switch (cplist->t) {
	case UP:
		*nx = cplist->x;
		*ny = cplist->y - 1;
		break;
	case DOWN:
		*nx = cplist->x;
		*ny = cplist->y + 1;
		break;
	case LEFT:
		*nx = cplist->x - 1;
		*ny = cplist->y;
		break;
	case RIGHT:
		*nx = cplist->x + 1;
		*ny = cplist->y;
		break;
	default:
		*nx = 0;
		*ny = 0;
		break;
	}
}

/* This function checks corner wall tiles that may look ugly after wall fusion
 * and changes them to straight walls if necessary. 
 */
static void fix_corner(int x, int y, enum connection_type t, enum connection_type t2) {
	if (mapgen_get_room(x, y) != -1)
		return;

	int tile = mapgen_get_tile(x, y);
	switch(tile) {
		case TILE_WALL_NW:
			if (mapgen_get_tile(x + 1, y) == TILE_FLOOR)
				tile = TILE_WALL_W;
			if (mapgen_get_tile(x, y + 1) == TILE_FLOOR)
				tile = TILE_WALL_N;
			break;
		case TILE_WALL_NE:
			if (mapgen_get_tile(x - 1, y) == TILE_FLOOR)
				tile = TILE_WALL_E;
			if (mapgen_get_tile(x, y + 1) == TILE_FLOOR)
				tile = TILE_WALL_N;
			break;
		case TILE_WALL_SW:
			if (mapgen_get_tile(x + 1, y) == TILE_FLOOR)
				tile = TILE_WALL_W;
			if (mapgen_get_tile(x, y - 1) == TILE_FLOOR)
				tile = TILE_WALL_S;
			break;
		case TILE_WALL_SE:
			if (mapgen_get_tile(x - 1, y) == TILE_FLOOR)
				tile = TILE_WALL_E;
			if (mapgen_get_tile(x, y - 1) == TILE_FLOOR)
				tile = TILE_WALL_S;
			break;
		/* Sometimes it is possible to see "pseudo" corner composed of
		 * straight walls, that is appear due to tile overlapping. That code
		 * checks all such combinations and make correction if necessary.
		 */
		default:
			switch(tile) {
			case TILE_WALL_N:
				if (
						(t2 == RIGHT && mapgen_get_tile(x, y - 1) == TILE_WALL_E) ||
						(t2 == LEFT && mapgen_get_tile(x, y - 1) == TILE_WALL_W)
				  )
					tile = TILE_FLOOR;
			break;
			case TILE_WALL_S:
				if (
						(t2 == RIGHT && mapgen_get_tile(x, y + 1) == TILE_WALL_E) ||
						(t2 == LEFT && mapgen_get_tile(x, y + 1) == TILE_WALL_W)
				  )
					tile = TILE_FLOOR; 
			break;
			case TILE_WALL_W:
				if (
						(t2 == UP && mapgen_get_tile(x - 1, y) == TILE_WALL_N) ||
						(t2 == DOWN && mapgen_get_tile(x - 1, y) == TILE_WALL_S)
				  )
					tile = TILE_FLOOR;
				break;
			case TILE_WALL_E:
				if (
						(t2 == UP && mapgen_get_tile(x + 1, y) == TILE_WALL_N) ||
						(t2 == DOWN && mapgen_get_tile(x + 1, y) == TILE_WALL_S)
				  )
					tile = TILE_FLOOR;
				break;
			}
	}
	mapgen_put_tile(x, y, tile, -1);
	return;
}

/**
 * Join two rooms by breaking the wall between them
 */
void fusion(int id, int cible)
{
	struct cplist_t cplist[100];
	int correct_directory[100];

	memset(correct_directory, -1, 100 * sizeof(int));
	memset(cplist, -1, 100 * sizeof(struct cplist_t));

	int nb_max;
	nb_max = find_connection_points(id, cplist, 0);
	int k = 0;
	int l = 0;		//index du tableau correct_directory
	int x, y; 
	int minx = 10000;
	int miny = 10000;	
	int maxx = -1;
	int maxy = -1;
	while (k < nb_max) {
		x = cplist[k].x;
		y = cplist[k].y;
		if (cplist[k].r == cible && !is_room_corner(x, y)) {
			correct_directory[l] = k;
			minx = min(minx, cplist[k].x);
			maxx = max(maxx, cplist[k].x);
			miny = min(miny, cplist[k].y);
			maxy = max(maxy, cplist[k].y);
			l++;
		}
		k++;
	}

	for (k = 0; k < l; k++) {
		x = cplist[correct_directory[k]].x;
		y = cplist[correct_directory[k]].y;
		mapgen_put_tile(x, y, TILE_FLOOR, -1);
	}

	if (l) {
		k = cplist[correct_directory[0]].t;
		if (miny == maxy) {
			fix_corner(minx - 1, miny, k, LEFT);
			fix_corner(maxx + 1, miny, k, RIGHT);
		}
		if (minx == maxx) {
			fix_corner(minx, miny - 1, k, UP);
			fix_corner(minx, maxy + 1, k, DOWN); 
		}
	}
}

static void add_rel(int x, int y, enum connection_type type, int r, int cible)
{
	MakeConnect(x, y, type);
	if ((((rooms[r].x != rooms[cible].x) || (rooms[r].w != rooms[cible].w)) && (type == UP || type == DOWN))
	    || (((rooms[r].y != rooms[cible].y) || (rooms[r].h != rooms[cible].h)) && (type == RIGHT || type == LEFT))) {
		if (!(rand() % 4))
			fusion(r, cible);
	}
}

static void bulldozer(unsigned char *seen, int r)
{
	struct cplist_t cplist[300];
	int max_connections = find_connection_points(r, cplist, 1);

	// Mark the room as seen by a bulldozer
	seen[r] = 1;

	// Pick a random connection to do
	int i = rand() % max_connections;
	int x2 = cplist[i].x;
	int y2 = cplist[i].y;
	adj(&cplist[i], &x2, &y2);

	// if the rooms are already connected we do not create a new connection and stop the bulldozer
	if (mapgen_are_connected(r, cplist[i].r))
		return;

	// else we create a connection
	add_rel(cplist[i].x, cplist[i].y, cplist[i].t, r, cplist[i].r);

	// and move the bulldozer to the next room
	bulldozer(seen, mapgen_get_room(x2, y2));
}

static void launch_buldo()
{
	int r, i, recalculate_components;
	unsigned char seen[total_rooms];
	unsigned char connected_to_room_0[total_rooms];
	memset(seen, 0, total_rooms);

	// Start bulldozers so that every room has been seen
	for (r = 0; r < total_rooms; r++) {
		if (!(seen[r])) {
			bulldozer(seen, r);
		}
	}

	// Check whether the graph is connected.
	// If it is not, ensure connectivity by connecting each vertex from 
	// a component that is not 0's to 0's whenever possible, until we have
	// connectivity.
	while (!mapgen_is_connected(connected_to_room_0)) {
		recalculate_components = 0;
		// Find the first room that is not connected to room 0
		for (i = 1; i < total_rooms && !recalculate_components; i++) {
			if (!connected_to_room_0[i]) {
				// See if we can connect it to a room that belongs to 0's connected component
				int n;
				struct cplist_t neigh[100];
				int nbconn, prevneigh = -1;
				nbconn = find_connection_points(i, neigh, 1);
				for (n = 0; n < nbconn; n++) {
					if (neigh[n].r == prevneigh)
						continue;
					else {
						int next = 0;
						while (next < nbconn && neigh[n + next].r == neigh[n].r)
							next++;

						int pick = n + rand() % next;

						n = pick;
					}

					prevneigh = neigh[n].r;

					if (connected_to_room_0[neigh[n].r]) {
						add_rel(neigh[n].x, neigh[n].y, neigh[n].t, i, neigh[n].r);

						if (!(rand() % 3)) {
							recalculate_components = 1;
						}

						break;
					}
				}
			}
		}
	}
}

int generate_dungeon_gram(int dim_x, int dim_y)
{
	dim_x_init = dim_x;
	dim_y_init = dim_y;
	total_rooms = 0;

	// Create first room
	mapgen_add_room(0, 0, dim_x_init, dim_y_init);
	mapgen_draw_room(0);

	// Recursively cut
	deriv_P(0);

	// Make connections between rooms
	launch_buldo();
	return 0;
}
