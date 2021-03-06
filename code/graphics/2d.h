/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _GRAPHICS_H
#define _GRAPHICS_H



/* ========================= pixel plotters =========================
In the 2d/texture mapper, bitmaps to be drawn will be passed by number.
The 2d function will call a bmpman function to get the bitmap into whatever
format it needs.  Then it will render.   The only pixels that will ever 
get drawn go thru the 2d/texture mapper libraries only.   This will make
supporting accelerators and psx easier.   Colors will always be set with
the color set functions.

gr_surface_flip()	switch onscreen, offscreen

gr_set_clip(x,y,w,h)	// sets the clipping region
gr_reset_clip(x,y,w,h)	// sets the clipping region
gr_set_color --? 8bpp, 15bpp?
gr_set_font(int fontnum)
// see GR_ALPHABLEND defines for values for alphablend_mode
// see GR_BITBLT_MODE defines for bitblt_mode.
// Alpha = scaler for intensity
gr_set_bitmap( int bitmap_num, int alphblend_mode, int bitblt_mode, float alpha )	
gr_set_shader( int value )  0=normal -256=darken, 256=brighten
gr_set_palette( ubyte * palette ) 

gr_clear()	// clears entire clipping region
gr_bitmap(x,y)
gr_bitmap_ex(x,y,w,h,sx,sy)
gr_rect(x,y,w,h)
gr_shade(x,y,w,h)
gr_string(x,y,char * text)
gr_line(x1,y1,x2,y2)

 
*/

#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"


//#define MATRIX_TRANSFORM_TYPE_WORLD 0
//#define MATRIX_TRANSFORM_TYPE_VIEW 1

enum Alignment
{
	ALIGNMENT_LEFT = 0,
	ALIGNMENT_CENTER,
	ALIGNMENT_RIGHT,
};

//MAX_POLYGON_NORMS
//#define MAX_POLYGON_TRI_POINTS 15000
extern const float Default_min_draw_distance;
extern const float Default_max_draw_distance;
extern float Min_draw_distance;
extern float Max_draw_distance;
extern int Gr_inited;
extern bool Keeping_aspect_ratio;
extern Alignment Active_alignment;

// z-buffering stuff
extern int gr_zbuffering, gr_zbuffering_mode;
extern int gr_global_zbuffering;

struct KeepAspectRatio
{
	KeepAspectRatio(bool keep);

	~KeepAspectRatio();

private:
	bool previousValue;
};

struct ActivateAlignment
{
	ActivateAlignment(Alignment newAlignment);

	~ActivateAlignment();

private:
	Alignment previousAlignment;
};

// This is a structure used by the shader to keep track
// of the values you want to use in the shade primitive.
typedef struct shader
{
	uint screen_sig;					// current mode this is in
	ubyte r,g,b,c;						// factors and constant
	ubyte lookup[256];
} shader;

#define AC_TYPE_NONE		0		// Not an alphacolor
#define AC_TYPE_HUD		1		// Doesn't change hue depending on background.  Used for HUD stuff.
#define AC_TYPE_BLEND	2		// Changes hue depending on background.  Used for stars, etc.

// NEVER REFERENCE THESE VALUES OUTSIDE OF THE GRAPHICS LIBRARY!!!
// If you need to get the rgb values of a "color" struct call
// gr_get_colors after calling gr_set_colors_fast.
typedef struct color
{
	uint screen_sig;
	ubyte red;
	ubyte green;
	ubyte blue;
	ubyte alpha;
	ubyte ac_type;							// The type of alphacolor.  See AC_TYPE_??? defines
	int is_alphacolor;
	ubyte raw8;
	int alphacolor;
	int magic;
} color;


typedef struct tsb_t
{
	vec3d tangent;
	float scaler;
} tsb_t;

// this should be basicly just like it is in the VB
// a list of triangles and their associated normals
struct poly_list
{
	poly_list()
		: n_prim(0),
		  n_verts(0),
		  vert(NULL),
		  norm(NULL),
		  tsb(NULL),
		  currently_allocated(0) {}
	~poly_list();
	poly_list& operator =(poly_list&);

	void allocate(int size);
	void make_index_buffer();
	void calculate_tangent();
	int n_prim;
	int n_verts;
	vertex* vert;
	vec3d* norm;
	tsb_t* tsb;

private:
	int currently_allocated;
};


struct colored_vector
{
	colored_vector()
		: pad(1.0f) {}
	vec3d vec;
	float pad;	//needed so I can just memcpy it in d3d
	ubyte color[4];
};

bool same_vert(vertex* v1, vertex* v2, vec3d* n1, vec3d* n2);

//finds the first occorence of a vertex within a poly list
int find_first_index(poly_list* plist, int idx);

//given a list (plist) and an indexed list (v) find the index within the indexed list that the vert at position idx within list is at 
int find_first_index_vb(poly_list* plist, int idx, poly_list* v);


struct line_list
{
	int n_line;
	vertex* vert;
};

struct light;


#define GR_ALPHABLEND_NONE			0		// no blending
#define GR_ALPHABLEND_FILTER		1		// 50/50 mix of foreground, background, using intensity as alpha

#define GR_BITBLT_MODE_NORMAL		0		// Normal bitblting
#define GR_BITBLT_MODE_RLE			1		// RLE would be faster

// fog modes
#define GR_FOGMODE_NONE				0		// set this to turn off fog
#define GR_FOGMODE_FOG				1		// linear fog

typedef struct screen
{
	uint signature;			// changes when mode or palette or width or height changes
	int max_w, max_h;		// Width and height
	int max_w_unscaled, max_h_unscaled;		// Width and height, should be 1024x768 or 640x480 in non-standard resolutions
	int save_max_w, save_max_h;		// Width and height
	int res;					// GR_640 or GR_1024
	int mode;					// What mode gr_init was called with.
	float aspect, clip_aspect;				// Aspect ratio, aspect of clip_width/clip_height
	int rowsize;				// What you need to add to go to next row (includes bytes_per_pixel)
	int bits_per_pixel;	// How many bits per pixel it is. (7,8,15,16,24,32)
	int bytes_per_pixel;	// How many bytes per pixel (1,2,3,4)
	int offset_x, offset_y;		// The offsets into the screen
	int offset_x_unscaled, offset_y_unscaled;	// Offsets into the screen, in 1024x768 or 640x480 dimensions
	int clip_width, clip_height;
	int clip_width_unscaled, clip_height_unscaled;	// Height and width of clip aread, in 1024x768 or 640x480 dimensions
	// center of clip area
	float clip_center_x, clip_center_y;

	float fog_near, fog_far;

	// the clip_l,r,t,b are used internally.  left and top are
	// actually always 0, but it's nice to have the code work with
	// arbitrary clipping regions.
	int clip_left, clip_right, clip_top, clip_bottom;
	// same as above except in 1024x768 or 640x480 dimensions
	int clip_left_unscaled, clip_right_unscaled, clip_top_unscaled, clip_bottom_unscaled;

	int current_alphablend_mode;		// See GR_ALPHABLEND defines above
	int current_bitblt_mode;				// See GR_BITBLT_MODE defines above
	int current_fog_mode;					// See GR_FOGMODE_* defines above
	int current_bitmap;
	color current_color;
	color current_fog_color;				// current fog color
	color current_clear_color;				// current clear color
	shader current_shader;
	float current_alpha;
	//	void		*offscreen_buffer;				// NEVER ACCESS!  This+rowsize*y = screen offset
	//	void		*offscreen_buffer_base;			// Pointer to lowest address of offscreen buffer

	bool custom_size;
	int rendering_to_texture;		//wich texture we are rendering to, -1 if the back buffer
	int rendering_to_face;			//wich face of the texture we are rendering to, -1 if the back buffer

	int envmap_render_target;

	bool recording_state_block;
	int current_state_block;

	void (*gf_start_state_block)();
	int (*gf_end_state_block)();
	void (*gf_set_state_block)(int);

	//switch onscreen, offscreen
	void (*gf_flip)();

	// Sets the current palette
	void (*gf_set_palette)(ubyte* new_pal, int restrict_alphacolor);

	// Fade the screen in/out
	void (*gf_fade_in)(int instantaneous);
	void (*gf_fade_out)(int instantaneous);

	// Flash the screen
	void (*gf_flash)(int r, int g, int b);
	void (*gf_flash_alpha)(int r, int g, int b, int a);

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h, bool resize, bool change_fov);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	//void (*gf_set_color)( int r, int g, int b );
	//void (*gf_get_color)( int * r, int * g, int * b );
	//void (*gf_init_color)( color * dst, int r, int g, int b );

	//void (*gf_init_alphacolor)( color * dst, int r, int g, int b, int alpha, int type );
	//void (*gf_set_color_fast)( color * dst );

	//void (*gf_set_font)(int fontnum);

	// Call this to create a shader.   
	// This function takes a while, so don't call it once a frame!
	// r,g,b, and c should be between -1.0 and 1.0f

	// The matrix is used as follows:
	// Dest(r) = Src(r)*r + Src(g)*r + Src(b)*r + c;
	// Dest(g) = Src(r)*g + Src(g)*g + Src(b)*g + c;
	// Dest(b) = Src(r)*b + Src(g)*b + Src(b)*b + c;
	// For instance, to convert to greyscale, use
	// .3 .3 .3  0
	// To turn everything green, use:
	//  0 .3  0  0
	//void (*gf_create_shader)(shader * shade, float r, float g, float b, float c );

	// Initialize the "shader" by calling gr_create_shader()
	// Passing a NULL makes a shader that turns everything black.
	//void (*gf_set_shader)( shader * shade );

	// clears entire clipping region to current color
	void (*gf_clear)();

	// void (*gf_bitmap)(int x, int y, bool resize);
	void (*gf_bitmap_ex)(int x, int y, int w, int h, int sx, int sy, bool resize);

	void (*gf_aabitmap)(int x, int y, bool resize, bool mirror);
	void (*gf_aabitmap_ex)(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror);

	//	void (*gf_rect)(int x, int y, int w, int h,bool resize);
	//	void (*gf_shade)(int x, int y, int w, int h);
	void (*gf_string)(int x, int y, char* text, bool resize);

	// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
	void (*gf_gradient)(int x1, int y1, int x2, int y2, bool resize);

	void (*gf_circle)(int x, int y, int r, bool resize);
	void (*gf_curve)(int x, int y, int r, int direction);

	// Integer line. Used to draw a fast but pixely line.  
	void (*gf_line)(int x1, int y1, int x2, int y2, bool resize);

	// Draws an antialiased line is the current color is an 
	// alphacolor, otherwise just draws a fast line.  This
	// gets called internally by g3_draw_line.   This assumes
	// the vertex's are already clipped, so call g3_draw_line
	// not this if you have two 3d points.
	void (*gf_aaline)(vertex* v1, vertex* v2);

	void (*gf_pixel)(int x, int y, bool resize);

	// Scales current bitmap between va and vb with clipping
	void (*gf_scaler)(vertex* va, vertex* vb, bool bw_bitmap);

	// Scales current bitmap between va and vb with clipping, draws an aabitmap
	void (*gf_aascaler)(vertex* va, vertex* vb);

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_tmapper)(int nv, vertex* verts[], uint flags);

	// dumps the current screen to a file
	void (*gf_print_screen)(char* filename);

	// Call once before rendering anything.
	void (*gf_start_frame)();

	// Call after rendering is over.
	void (*gf_stop_frame)();

	// Retrieves the zbuffer mode.
	int (*gf_zbuffer_get)();

	// Sets mode.  Returns previous mode.
	int (*gf_zbuffer_set)(int mode);

	// Clears the zbuffer.  If use_zbuffer is FALSE, then zbuffering mode is ignored and zbuffer is always off.
	void (*gf_zbuffer_clear)(int use_zbuffer);

	// Saves screen. Returns an id you pass to restore and free.
	int (*gf_save_screen)();

	// Resets clip region and copies entire saved screen to the screen.
	void (*gf_restore_screen)(int id);

	// Frees up a saved screen.
	void (*gf_free_screen)(int id);

	// CODE FOR DUMPING FRAMES TO A FILE
	// Begin frame dumping
	void (*gf_dump_frame_start)(int first_frame_number, int nframes_between_dumps);

	// Dump the current frame to file
	void (*gf_dump_frame)();

	// Dump the current frame to file
	void (*gf_dump_frame_stop)();

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// Lock/unlock the screen
	// Returns non-zero if sucessful (memory pointer)
	uint (*gf_lock)();
	void (*gf_unlock)();

	// grab a region of the screen. assumes data is large enough
	void (*gf_get_region)(int front, int w, int h, ubyte* data);

	// set fog attributes
	void (*gf_fog_set)(int fog_mode, int r, int g, int b, float fog_near, float fog_far);

	// poly culling
	int (*gf_set_cull)(int cull);

	// cross fade
	void (*gf_cross_fade)(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);

	// set a texture into cache. for sectioned bitmaps, pass in sx and sy to set that particular section of the bitmap
	int (*gf_tcache_set)(int bitmap_id, int bitmap_type, float* u_scale, float* v_scale, int stage);

	// preload a bitmap into texture memory
	int (*gf_preload)(int bitmap_num, int is_aabitmap);

	// set the color to be used when clearing the background
	void (*gf_set_clear_color)(int r, int g, int b);

	// Here be the bitmap functions
	void (*gf_bm_free_data)(int n, bool release);
	void (*gf_bm_create)(int n);
	int (*gf_bm_load)(ubyte type, int n, char* filename, CFILE* img_cfp, int* w, int* h, int* bpp, ubyte* c_type,
					  int* mm_lvl, int* size);
	void (*gf_bm_init)(int n);
	void (*gf_bm_page_in_start)();
	int (*gf_bm_lock)(char* filename, int handle, int bitmapnum, ubyte bpp, ubyte flags);

	int (*gf_bm_make_render_target)(int n, int* width, int* height, ubyte* bpp, int* mm_lvl, int flags);
	int (*gf_bm_set_render_target)(int n, int face);

	void (*gf_translate_texture_matrix)(int unit, vec3d* shift);
	void (*gf_push_texture_matrix)(int unit);
	void (*gf_pop_texture_matrix)(int unit);

	void (*gf_set_texture_addressing)(int);

	int (*gf_make_buffer)(poly_list*, uint flags);
	void (*gf_destroy_buffer)(int);
	void (*gf_set_buffer)(int);
	void (*gf_render_buffer)(int, int, ushort*, uint*, int);
	int (*gf_make_flat_buffer)(poly_list*);
	int (*gf_make_line_buffer)(line_list*);


	//the projection matrix; fov, aspect ratio, near, far
	void (*gf_set_proj_matrix)(float, float, float, float);
	void (*gf_end_proj_matrix)();
	//the view matrix
	void (*gf_set_view_matrix)(vec3d*, matrix*);
	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(vec3d*);
	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(vec3d*, matrix*);
	void (*gf_start_angles_instance_matrix)(vec3d*, angles*);
	void (*gf_end_instance_matrix)();

	int (*gf_make_light)(light*, int, int);
	void (*gf_modify_light)(light*, int, int);
	void (*gf_destroy_light)(int);
	void (*gf_set_light)(light*);
	void (*gf_reset_lighting)();
	void (*gf_set_ambient_light)(int, int, int);

	// postprocessing effects
	void (*gf_set_post_effect)(SCP_string&, int);
	void (*gf_set_default_post_process)();

	void (*gf_post_process_init)();
	void (*gf_post_process_before)();
	void (*gf_post_process_after)();
	void (*gf_save_zbuffer)();

	void (*gf_lighting)(bool, bool);
	void (*gf_center_alpha)(int);

	void (*gf_start_clip_plane)();
	void (*gf_end_clip_plane)();

	void (*gf_zbias)(int zbias);
	void (*gf_setup_background_fog)(bool);

	void (*gf_set_fill_mode)(int);
	void (*gf_set_texture_panning)(float u, float v, bool enable);

	void (*gf_draw_line_list)(colored_vector* lines, int num);

	void (*gf_set_line_width)(float width);

	void (*gf_line_htl)(vec3d* start, vec3d* end);
	void (*gf_sphere_htl)(float rad);

	//	void (*gf_set_environment_mapping)(int i);

	/*	void (*gf_begin_sprites)();//does prep work for sprites
		void (*gf_draw_sprite)(vec3d*);//draws a sprite
		void (*gf_display_sprites))();//actualy darws the drawen sprites
		void (*gf_end_sprites)();//clears the lists and stuff
	*/
} screen;

// handy macro
#define GR_MAYBE_CLEAR_RES(bmap)		do  { int bmw = -1; int bmh = -1; if(bmap != -1){ bm_get_info( bmap, &bmw, &bmh, NULL, NULL, NULL); if((bmw != gr_screen.max_w) || (bmh != gr_screen.max_h)){gr_clear();} } else {gr_clear();} } while(0);

//Window's interface to set up graphics:
//--------------------------------------
// Call this at application startup

// # Software Re-added by Kazan --- THIS HAS TO STAY -- It is used by standalone!
#define GR_DEFAULT				(-1)		// set to use default settings
#define GR_STUB					(100)
#define GR_OPENGL				(104)		// Use OpenGl hardware renderer

// resolution constants   - always keep resolutions in ascending order and starting from 0  
#define GR_NUM_RESOLUTIONS			2
#define GR_640							0		// 640 x 480
#define GR_1024						1		// 1024 x 768

extern bool gr_init(int d_mode = GR_DEFAULT, int d_width = GR_DEFAULT, int d_height = GR_DEFAULT,
					int d_depth = GR_DEFAULT);
extern void gr_screen_resize(int width, int height);

// Call this when your app ends.
extern void gr_close();

extern screen gr_screen;

#define GR_FILL_MODE_WIRE 1
#define GR_FILL_MODE_SOLID 2

#define GR_ZBUFF_NONE	0
#define GR_ZBUFF_WRITE	(1<<0)
#define GR_ZBUFF_READ	(1<<1)
#define GR_ZBUFF_FULL	(GR_ZBUFF_WRITE|GR_ZBUFF_READ)

extern std::pair<int, int> gr_get_alignment_offset(bool useForClipping = false);
bool gr_unsize_screen_pos(int* x, int* y);
bool gr_resize_screen_pos(int* x, int* y, bool isDimension = false);
bool gr_unsize_screen_posf(float* x, float* y);
bool gr_resize_screen_posf(float* x, float* y, bool isDimension = false);

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void _cdecl gr_printf(int x, int y, char* format, ...);
// same as above but doesn't resize for non-standard resolutions
extern void _cdecl gr_printf_no_resize(int x, int y, char* format, ...);

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size(int* w, int* h, char* text, int len = 9999);

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(char* name, ubyte* palette, int restrict_to_128 = 0);

// These two functions use a Windows mono font.  Only for use
// in the editor, please.
void gr_get_string_size_win(int* w, int* h, char* text);
void gr_string_win(int x, int y, char* s);

// set the mouse pointer to a specific bitmap, used for animating cursors
#define GR_CURSOR_LOCK		1
#define GR_CURSOR_UNLOCK	2
void gr_set_cursor_bitmap(int n, int lock = 0);
void gr_unset_cursor_bitmap(int n);
int gr_get_cursor_bitmap();
extern int Web_cursor_bitmap;

// Called by OS when application gets/looses focus
extern void gr_activate(int active);

#define GR_CALL(x)			(*x)

// These macros make the function indirection look like the
// old Descent-style gr_xxx calls.

#define gr_print_screen		GR_CALL(gr_screen.gf_print_screen)

//#define gr_flip				GR_CALL(gr_screen.gf_flip)
void gr_flip();

//#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
__inline void gr_set_clip(int x, int y, int w, int h, bool resize=true, bool change_fov=true)
{
	(*gr_screen.gf_set_clip)(x, y, w, h, resize, change_fov);
}
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)
//#define gr_set_font			GR_CALL(gr_screen.gf_set_font)

//#define gr_init_color		GR_CALL(gr_screen.gf_init_color)
//#define gr_init_alphacolor	GR_CALL(gr_screen.gf_init_alphacolor)
//__inline void gr_init_alphacolor( color * dst, int r, int g, int b, int alpha, int type=AC_TYPE_HUD )
//{
//	(*gr_screen.gf_init_alphacolor)(dst, r, g, b, alpha,type );
//}

//#define gr_set_color			GR_CALL(gr_screen.gf_set_color)
//#define gr_get_color			GR_CALL(gr_screen.gf_get_color)
//#define gr_set_color_fast	GR_CALL(gr_screen.gf_set_color_fast)

//#define gr_set_bitmap			GR_CALL(gr_screen.gf_set_bitmap)
void gr_set_bitmap(int bitmap_num, int alphablend = GR_ALPHABLEND_NONE, int bitbltmode = GR_BITBLT_MODE_NORMAL,
				   float alpha = 1.0f);

//#define gr_create_shader	GR_CALL(gr_screen.gf_create_shader)
//#define gr_set_shader		GR_CALL(gr_screen.gf_set_shader)
#define gr_clear				GR_CALL(gr_screen.gf_clear)
//#define gr_aabitmap			GR_CALL(gr_screen.gf_aabitmap)
__inline void gr_aabitmap(int x, int y, bool resize = true, bool mirror = false)
{
	(*gr_screen.gf_aabitmap)(x, y, resize, mirror);
}
//#define gr_aabitmap_ex		GR_CALL(gr_screen.gf_aabitmap_ex)
__inline void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize = true, bool mirror = false)
{
	(*gr_screen.gf_aabitmap_ex)(x, y, w, h, sx, sy, resize, mirror);
}
//#define gr_bitmap_ex		GR_CALL(gr_screen.gf_bitmap_ex)
__inline void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize = true)
{
	(*gr_screen.gf_bitmap_ex)(x, y, w, h, sx, sy, resize);
}

void gr_rect(int x, int y, int w, int h, bool resize = true);
void gr_shade(int x, int y, int w, int h, bool resize = true);

//#define gr_shade				GR_CALL(gr_screen.gf_shade)
//#define gr_string				GR_CALL(gr_screen.gf_string)
__inline void gr_string(int x, int y, char* string, bool resize = true)
{
	(*gr_screen.gf_string)(x, y, string, resize);
}

//#define gr_circle				GR_CALL(gr_screen.gf_circle)
__inline void gr_circle(int xc, int yc, int d, bool resize = true)
{
	(*gr_screen.gf_circle)(xc, yc, d, resize);
}
#define gr_curve				GR_CALL(gr_screen.gf_curve)

//#define gr_line				GR_CALL(gr_screen.gf_line)
__inline void gr_line(int x1, int y1, int x2, int y2, bool resize = true)
{
	(*gr_screen.gf_line)(x1, y1, x2, y2, resize);
}

#define gr_aaline				GR_CALL(gr_screen.gf_aaline)

//#define gr_pixel				GR_CALL(gr_screen.gf_pixel)
__inline void gr_pixel(int x, int y, bool resize = true)
{
	(*gr_screen.gf_pixel)(x, y, resize);
}
#define gr_scaler				GR_CALL(gr_screen.gf_scaler)
#define gr_aascaler			GR_CALL(gr_screen.gf_aascaler)
#define gr_tmapper			GR_CALL(gr_screen.gf_tmapper)

//#define gr_gradient			GR_CALL(gr_screen.gf_gradient)
__inline void gr_gradient(int x1, int y1, int x2, int y2, bool resize = true)
{
	(*gr_screen.gf_gradient)(x1, y1, x2, y2, resize);
}


#define gr_fade_in			GR_CALL(gr_screen.gf_fade_in)
#define gr_fade_out			GR_CALL(gr_screen.gf_fade_out)
#define gr_flash			GR_CALL(gr_screen.gf_flash)
#define gr_flash_alpha		GR_CALL(gr_screen.gf_flash_alpha)

#define gr_zbuffer_get		GR_CALL(gr_screen.gf_zbuffer_get)
#define gr_zbuffer_set		GR_CALL(gr_screen.gf_zbuffer_set)
#define gr_zbuffer_clear	GR_CALL(gr_screen.gf_zbuffer_clear)

#define gr_save_screen		GR_CALL(gr_screen.gf_save_screen)
#define gr_restore_screen	GR_CALL(gr_screen.gf_restore_screen)
#define gr_free_screen		GR_CALL(gr_screen.gf_free_screen)

#define gr_dump_frame_start	GR_CALL(gr_screen.gf_dump_frame_start)
#define gr_dump_frame_stop		GR_CALL(gr_screen.gf_dump_frame_stop)
#define gr_dump_frame			GR_CALL(gr_screen.gf_dump_frame)

#define gr_set_gamma			GR_CALL(gr_screen.gf_set_gamma)

#define gr_get_region		GR_CALL(gr_screen.gf_get_region)

//#define gr_fog_set			GR_CALL(gr_screen.gf_fog_set)
__inline void gr_fog_set(int fog_mode, int r, int g, int b, float fog_near = -1.0f, float fog_far = -1.0f)
{
	(*gr_screen.gf_fog_set)(fog_mode, r, g, b, fog_near, fog_far);
}

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)

#define gr_cross_fade		GR_CALL(gr_screen.gf_cross_fade)

//#define gr_tcache_set		GR_CALL(gr_screen.gf_tcache_set)
__inline int gr_tcache_set(int bitmap_id, int bitmap_type, float* u_scale, float* v_scale, int stage = 0)
{
	return (*gr_screen.gf_tcache_set)(bitmap_id, bitmap_type, u_scale, v_scale, stage);
}

#define gr_preload			GR_CALL(gr_screen.gf_preload)

#define gr_set_clear_color	GR_CALL(gr_screen.gf_set_clear_color)

#define gr_translate_texture_matrix		GR_CALL(gr_screen.gf_translate_texture_matrix)
#define gr_push_texture_matrix			GR_CALL(gr_screen.gf_push_texture_matrix)
#define gr_pop_texture_matrix			GR_CALL(gr_screen.gf_pop_texture_matrix)


// Here be the bitmap functions
#define gr_bm_free_data				GR_CALL(*gr_screen.gf_bm_free_data)
#define gr_bm_create				GR_CALL(*gr_screen.gf_bm_create)
#define gr_bm_init					GR_CALL(*gr_screen.gf_bm_init)
__inline int gr_bm_load(ubyte type, int n, char* filename, CFILE* img_cfp = NULL, int* w = 0, int* h = 0, int* bpp = 0,
						ubyte* c_type = 0, int* mm_lvl = 0, int* size = 0)
{
	return (*gr_screen.gf_bm_load)(type, n, filename, img_cfp, w, h, bpp, c_type, mm_lvl, size);
}
#define gr_bm_page_in_start			GR_CALL(*gr_screen.gf_bm_page_in_start)
#define gr_bm_lock					GR_CALL(*gr_screen.gf_bm_lock)

#define gr_bm_make_render_target					GR_CALL(*gr_screen.gf_bm_make_render_target)
//#define gr_bm_set_render_target					GR_CALL(*gr_screen.gf_bm_set_render_target)          
__inline int gr_bm_set_render_target(int n, int face = -1)
{
	return (*gr_screen.gf_bm_set_render_target)(n, face);
}

#define gr_set_texture_addressing					 GR_CALL(*gr_screen.gf_set_texture_addressing)

#define gr_make_buffer					GR_CALL(*gr_screen.gf_make_buffer)
#define gr_destroy_buffer				 GR_CALL(*gr_screen.gf_destroy_buffer)
__inline void gr_render_buffer(int start, int n_prim, ushort* sbuffer, uint* ibuffer = NULL, int flags =
							   TMAP_FLAG_TEXTURED)
{
	(*gr_screen.gf_render_buffer)(start, n_prim, sbuffer, ibuffer, flags);
}

#define gr_set_buffer					GR_CALL(*gr_screen.gf_set_buffer)

#define gr_make_flat_buffer				GR_CALL(*gr_screen.gf_make_flat_buffer)
#define gr_make_line_buffer				GR_CALL(*gr_screen.gf_make_line_buffer)

#define gr_set_proj_matrix					GR_CALL(*gr_screen.gf_set_proj_matrix)
#define gr_end_proj_matrix					GR_CALL(*gr_screen.gf_end_proj_matrix)
#define gr_set_view_matrix					GR_CALL(*gr_screen.gf_set_view_matrix)
#define gr_end_view_matrix					GR_CALL(*gr_screen.gf_end_view_matrix)
#define gr_push_scale_matrix				GR_CALL(*gr_screen.gf_push_scale_matrix)
#define gr_pop_scale_matrix					GR_CALL(*gr_screen.gf_pop_scale_matrix)
#define gr_start_instance_matrix			GR_CALL(*gr_screen.gf_start_instance_matrix)
#define gr_start_angles_instance_matrix		GR_CALL(*gr_screen.gf_start_angles_instance_matrix)
#define gr_end_instance_matrix				GR_CALL(*gr_screen.gf_end_instance_matrix)

#define	gr_make_light					GR_CALL(*gr_screen.gf_make_light)
#define	gr_modify_light					GR_CALL(*gr_screen.gf_modify_light)
#define	gr_destroy_light				GR_CALL(*gr_screen.gf_destroy_light)
#define	gr_set_light					GR_CALL(*gr_screen.gf_set_light)
#define gr_reset_lighting				GR_CALL(*gr_screen.gf_reset_lighting)
#define gr_set_ambient_light			GR_CALL(*gr_screen.gf_set_ambient_light)

#define gr_set_post_effect				GR_CALL(*gr_screen.gf_set_post_effect)

#define	gr_set_lighting					GR_CALL(*gr_screen.gf_lighting)
#define	gr_center_alpha					GR_CALL(*gr_screen.gf_center_alpha)

#define	gr_start_clip					GR_CALL(*gr_screen.gf_start_clip_plane)
#define	gr_end_clip						GR_CALL(*gr_screen.gf_end_clip_plane)

#define	gr_zbias						GR_CALL(*gr_screen.gf_zbias)
#define	gr_set_fill_mode				GR_CALL(*gr_screen.gf_set_fill_mode)
#define	gr_set_texture_panning			GR_CALL(*gr_screen.gf_set_texture_panning)

#define	gr_start_state_block			GR_CAL(*gr_screen.gf_start_state_block)
#define	gr_end_state_block				GR_CALL(*gr_screen.gf_end_state_block)
#define	gr_set_state_block				GR_CALL(*gr_screen.gf_set_state_block)

//#define	gr_set_environment_mapping	GR_CALL(*gr_screen.gf_set_environment_mapping)

#define gr_setup_background_fog			GR_CALL(*gr_screen.gf_setup_background_fog)

#define gr_draw_line_list				GR_CALL(*gr_screen.gf_draw_line_list)

#define gr_set_line_width				GR_CALL(*gr_screen.gf_set_line_width)

#define gr_line_htl						GR_CALL(*gr_screen.gf_line_htl)
#define gr_sphere_htl					GR_CALL(*gr_screen.gf_sphere_htl)

/*
#define	gr_begin_sprites				GR_CALL(*gr_screen.gf_begin_sprites)
#define	gr_draw_sprites					GR_CALL(*gr_screen.gf_draw_sprites)
#define	gr_end sprites					GR_CALL(*gr_screen.gf_end_sprites)
#define	gr_display_sprites				GR_CALL(*gr_screen.gf_display_sprites)
*/

// color functions
void gr_get_color(int* r, int* g, int b);
void gr_init_color(color* c, int r, int g, int b);
void gr_init_alphacolor(color* clr, int r, int g, int b, int alpha, int type = AC_TYPE_HUD);
void gr_set_color(int r, int g, int b);
void gr_set_color_fast(color* dst);

// shader functions
void gr_create_shader(shader* shade, ubyte r, ubyte g, ubyte b, ubyte c);
void gr_set_shader(shader* shade);

// new bitmap functions
void gr_bitmap(int x, int y, bool resize = true);
void gr_bitmap_list(bitmap_2d_list* list, int n_bm, bool allow_scaling);
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, bool allow_scaling);

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(vec3d** pts, int num_pts, int thickness, bool resize=true);

#define VERTEX_FLAG_POSITION	(1<<0)
#define VERTEX_FLAG_RHW			(1<<1)	//incompatable with the next normal
#define VERTEX_FLAG_NORMAL		(1<<2)
#define VERTEX_FLAG_DIFUSE		(1<<3)
#define VERTEX_FLAG_SPECULAR	(1<<4)
#define VERTEX_FLAG_UV1			(1<<5)	//how many UV coords, only use one of these
#define VERTEX_FLAG_UV2			(1<<6)
#define VERTEX_FLAG_UV3			(1<<7)
#define VERTEX_FLAG_UV4			(1<<8)
#define VERTEX_FLAG_TANGENT		(1<<9)

void gr_clear_shaders_cache();

float gr_zoom_to_fov(float zoom);

float gr_fov_to_zoom(float fov);

#endif
