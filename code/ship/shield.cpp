/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



//	Detail level effects (Detail.shield_effects)
//		0		Nothing rendered
//		1		An animating bitmap rendered per hit, not shrink-wrapped.  Lasts half time.  One per ship.
//		2		Animating bitmap per hit, not shrink-wrapped.  Lasts full time.  Unlimited.
//		3		Shrink-wrapped texture.  Lasts half-time.
//		4		Shrink-wrapped texture.  Lasts full-time.

//#include <math.h>


#include "render/3d.h"
#include "model/model.h"
#include "freespace2/freespace.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "object/objectshield.h"
#include "species_defs/species_defs.h"
#include "ship/ship.h"
#include "particle/particle.h"

int Show_shield_mesh = 0;

#ifndef DEMO // not for FS2_DEMO

//	One unit in 3d means this in the shield hit texture map.
//#define	SHIELD_HIT_SCALE	0.075f			//	Scale decreased by MK on 12/18/97, made about 1/4x as large. Note, larger constant means smaller effect
#define	SHIELD_HIT_SCALE	0.15f			//	Doubled on 12/23/97 by MK.  Was overflowing.  See todo item #924.
//#define	MAX_SHIELD_HITS	20
#define	MAX_TRIS_PER_HIT	40					//	Number of triangles per shield hit, maximum.
#define	MAX_SHIELD_HITS	20					//	Maximum number of active shield hits.
#define	MAX_SHIELD_TRI_BUFFER	(MAX_SHIELD_HITS*MAX_TRIS_PER_HIT) //(MAX_SHIELD_HITS*20) //	Persistent buffer of triangle comprising all active shield hits.
#define	SHIELD_HIT_DURATION	(3*F1_0/4)	//	Duration, in milliseconds, of shield hit effect

#define	SH_UNUSED			-1					//	Indicates an unused record in Shield_hits
#define	SH_TYPE_1			1					//	Indicates Shield_hits record is of type 1.

#define	UV_MAX				(63.95f/64.0f)	//	max allowed value until tmapper bugs fixed, 1/24/97

float Shield_scale = SHIELD_HIT_SCALE;

//	Structure which mimics the shield_tri structure in model.h.  Since the global shield triangle
// array needs the vertex information, we will acutally store the information in this
// structure instead of the indices into the vertex list
typedef struct gshield_tri
{
	int used;							//	Set if this triangle is currently in use.
	int trinum;						//	a debug parameter
	fix creation_time;				//	time at which created.
	shield_vertex verts[4];					//	Triangles, but at lower detail level, a square.
} gshield_tri;

typedef struct shield_hit
{
	int start_time;								//	start time of this object
	int type;										//	type, probably the weapon type, to indicate the bitmap to use
	int objnum;									//	Object index, needed to get current orientation, position.
	int num_tris;								//	Number of Shield_tris comprising this shield.
	int tri_list[MAX_TRIS_PER_HIT];		//	Indices into Shield_tris, triangles for this shield hit.
	ubyte rgb[3];									// rgb colors
} shield_hit;

//	Stores point at which shield was hit.
//	Gets processed in frame interval.
typedef struct shield_point
{
	int objnum;								//	Object that was hit.
	int shield_tri;							//	Triangle in shield mesh that took hit.
	vec3d hit_point;							//	Point in global 3-space of hit.
} shield_point;

#define	MAX_SHIELD_POINTS	256
shield_point Shield_points[MAX_SHIELD_POINTS];
int Num_shield_points;
int Num_multi_shield_points;					// used by multiplayer clients

gshield_tri Global_tris[MAX_SHIELD_TRI_BUFFER];	//	The persistent triangles, part of shield hits.
int Num_tris;								//	Number of triangles in current shield.  Would be a local, but needed in numerous routines.

shield_hit Shield_hits[MAX_SHIELD_HITS];

int Shield_bitmaps_loaded = 0;

//	This is a recursive function, so prototype it.
extern void create_shield_from_triangle(int trinum, matrix* orient, shield_info* shieldp, vec3d* tcp, vec3d* centerp,
										float radius, vec3d* rvec, vec3d* uvec);

void do_shield_effect(object* ship_objp, mc_info* mc, bool shield_collision)
{
	Assert(ship_objp->type == OBJ_SHIP);
	Assert(ship_objp->instance >= 0);

	ship* shipp = &Ships[ship_objp->instance];
	ship_info* sip = &Ship_info[shipp->ship_info_index];
	polymodel* pm = model_get(sip->model_num);

	int anim_id = Species_info[sip->species].shield_anim.first_frame;
	float radius = MIN(15.0f, MAX(1.5f, ship_objp->radius * 0.04f));
	vec3d impact_pos;
	vec3d impact_normal;

	// do the hit effect
	if (shield_collision)
	{
		//add_shield_point(OBJ_INDEX(ship_objp), mc_shield.shield_hit_tri, &mc_shield.hit_point);

		// KeldorKatarn interpolate the shield vertex normals.
		/*impact_pos = mc->hit_point;
		impact_normal = pm->shield.tris[mc->shield_hit_tri].norm;
		
		vec3d A = pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[0]].pos;
		vec3d B = pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[1]].pos;
		vec3d C = pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[2]].pos;
		
		vec3d E0, E1, impact_to_A;
		
		vm_vec_sub(&E0, &B, &A);
		vm_vec_sub(&E1, &C, &A);
		vm_vec_sub(&impact_to_A, &A, &impact_pos);
		vm_vec_normalize_safe(&E0);
		vm_vec_normalize_safe(&E1);
		vm_vec_normalize_safe(&impact_to_A);
		
		float a00 = vm_vec_dotprod(&E0, &E0);
		float a01 = vm_vec_dotprod(&E0, &E1);
		float a11 = vm_vec_dotprod(&E1, &E1);
		float b0 = vm_vec_dotprod(&E0, &impact_to_A);
		float b1 = vm_vec_dotprod(&E1, &impact_to_A);
		
		float s = (a01 * b1 - a11 * b0) / (a00 * a11 - a01 * a01);
		float t = (a01 * b0 - a00 * b1) / (a00 * a11 - a01 * a01);
		
		impact_normal = vmd_zero_vector;
		vm_vec_scale_add2(&impact_normal, &pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[0]].normal, (1.0f - s - t));
		vm_vec_scale_add2(&impact_normal, &pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[1]].normal, s);
		vm_vec_scale_add2(&impact_normal, &pm->shield.verts[pm->shield.tris[mc->shield_hit_tri].verts[2]].normal, t);
		
		vm_vec_normalize_safe(&impact_normal);*/

		// KeldorKatarn: Interpolation doesn't seem to work for some reason
		// if the collision tree isn't used but the octants instead. Not sure why.
		impact_pos = mc->hit_point;
		impact_normal = mc->hit_normal;
		vm_vec_scale_add2(&impact_pos, &impact_normal, 1.0f);
	}
	else
	{
		model_rot_sub_into_obj(&impact_pos, &mc->hit_point, pm, mc->hit_submodel);
		impact_normal = mc->hit_normal;
		vm_vec_scale_add2(&impact_pos, &impact_normal, 1.0f);
	}

	particle_create(&impact_pos, &impact_normal, &vmd_zero_vector, 0.0f, radius, PARTICLE_BITMAP_3D, anim_id,
		sip->shield_color[0], sip->shield_color[1], sip->shield_color[2], -1.0f, ship_objp);
}

void load_shield_hit_bitmap()
{
#ifndef DEMO // not for FS2_DEMO

	uint i;
	// Check if we've already allocated the shield effect bitmaps
	if (Shield_bitmaps_loaded)
		return;

	Shield_bitmaps_loaded = 1;

	for (i = 0; i < Species_info.size(); i++)
	{
		Species_info[i].shield_anim.first_frame = bm_load_animation(Species_info[i].shield_anim.
			filename, &Species_info[i].shield_anim.num_frames, NULL, NULL, 1);

		// *This is disabled for TBP    -Et1
		// Changed to an assert by kazan

		/*
		if ( Shield_ani[i].first_frame < 0 )
		Int3();
		*/
		Assert(Species_info[i].shield_anim.first_frame >= 0);
	}

#endif
}

void shield_hit_page_in()
{
	uint i;

	if (!Shield_bitmaps_loaded)
	{
		load_shield_hit_bitmap();
	}

	for (i = 0; i < Species_info.size(); i++)
	{
		generic_anim* sa = &Species_info[i].shield_anim;
		if (sa->first_frame >= 0)
		{
			bm_page_in_xparent_texture(sa->first_frame, sa->num_frames);
		}
	}
}


//	Initialize shield hit system.  Called from game_level_init()
void shield_hit_init()
{
	int i;

	for (i = 0; i < MAX_SHIELD_HITS; i++)
	{
		Shield_hits[i].type = SH_UNUSED;
		Shield_hits[i].objnum = -1;
	}

	for (i = 0; i < MAX_SHIELD_TRI_BUFFER; i++)
	{
		Global_tris[i].used = 0;
		Global_tris[i].creation_time = Missiontime;
	}

	Num_multi_shield_points = 0;

	load_shield_hit_bitmap();
}

// ---------------------------------------------------------------------
// release_shield_hit_bitmap()
//
// Release the storage allocated to store the shield effect.
//
void release_shield_hit_bitmap()
{
	if (!Shield_bitmaps_loaded)
		return;

	// This doesn't need to do anything; the bitmap manager will
	// release everything.
}

int Poly_count = 0;

// ---------------------------------------------------------------------
// shield_hit_close()
//
// De-initialize the shield hit system.  Called from game_level_close().
//
// TODO: We should probably not bother releasing the shield hit bitmaps every level.
//
void shield_hit_close()
{
	release_shield_hit_bitmap();
}

void shield_frame_init()
{
	//nprintf(("AI", "Frame %i: Number of shield hits: %i, polycount = %i\n", Framecount, Num_shield_points, Poly_count));

	Poly_count = 0;

	Num_shield_points = 0;
}

void create_low_detail_poly(int global_index, vec3d* tcp, vec3d* rightv, vec3d* upv)
{
	float scale;
	gshield_tri* trip;

	trip = &Global_tris[global_index];

	scale = vm_vec_mag(tcp) * 2.0f;

	vm_vec_scale_add(&trip->verts[0].pos, tcp, rightv, -scale / 2.0f);
	vm_vec_scale_add2(&trip->verts[0].pos, upv, scale / 2.0f);

	vm_vec_scale_add(&trip->verts[1].pos, &trip->verts[0].pos, rightv, scale);

	vm_vec_scale_add(&trip->verts[2].pos, &trip->verts[1].pos, upv, -scale);

	vm_vec_scale_add(&trip->verts[3].pos, &trip->verts[2].pos, rightv, -scale);

	//	Set u, v coordinates.
	//	Note, this need only be done once, as it's common for all explosions.
	trip->verts[0].u = 0.0f;
	trip->verts[0].v = 0.0f;

	trip->verts[1].u = 1.0f;
	trip->verts[1].v = 0.0f;

	trip->verts[2].u = 1.0f;
	trip->verts[2].v = 1.0f;

	trip->verts[3].u = 0.0f;
	trip->verts[3].v = 1.0f;

}

//	----------------------------------------------------------------------------------------------------
//	Given a shield triangle, compute the uv coordinates at its vertices given
//	the center point of the explosion texture, distance to center of shield and
//	right and up vectors.
//	For small distances (relative to radius), coordinates can be computed using
//	distance.  For larger values, should comptue angle.
void rs_compute_uvs(shield_tri* stp, shield_vertex* verts, vec3d* tcp, float radius, vec3d* rightv, vec3d* upv)
{
	int i;
	shield_vertex* sv;

	for (i = 0; i < 3; i++)
	{
		vec3d v2cp;

		sv = &verts[stp->verts[i]];

		vm_vec_sub(&v2cp, &sv->pos, tcp);
		sv->u = vm_vec_dot(&v2cp, rightv) * Shield_scale + 0.5f;
		sv->v = - vm_vec_dot(&v2cp, upv) * Shield_scale + 0.5f;

		if (sv->u > UV_MAX)
		{
			sv->u = UV_MAX;
		}

		if (sv->u < 0.0f)
		{
			sv->u = 0.0f;
		}

		if (sv->v > UV_MAX)
		{
			sv->v = UV_MAX;
		}

		if (sv->v < 0.0f)
		{
			sv->v = 0.0f;
		}

		// mprintf(("u, v = %7.3f %7.3f\n", stp->verts[i].u, stp->verts[i].v));
	}

	// mprintf(("\n"));
}

//	----------------------------------------------------------------------------------------------------
//	Free records in Global_tris previously used by Shield_hits[shnum].tri_list
void free_global_tri_records(int shnum)
{
	int i;

	Assert((shnum >= 0) && (shnum < MAX_SHIELD_HITS));

	//mprintf(("Freeing up %i global records.\n", Shield_hits[shnum].num_tris));

	for (i = 0; i < Shield_hits[shnum].num_tris; i++)
	{
		Global_tris[Shield_hits[shnum].tri_list[i]].used = 0;
	}
}

extern int Cmdline_nohtl;

void render_low_detail_shield_bitmap(gshield_tri* trip, matrix* orient, vec3d* pos, ubyte r, ubyte g, ubyte b)
{
	int j;
	vec3d pnt;
	vertex verts[4];

	for (j = 0; j < 4; j++)
	{
		// Rotate point into world coordinates
		vm_vec_unrotate(&pnt, &trip->verts[j].pos, orient);
		vm_vec_add2(&pnt, pos);

		// Pnt is now the x,y,z world coordinates of this vert.

		if (!Cmdline_nohtl)
			g3_transfer_vertex(&verts[j], &pnt);
		else
			g3_rotate_vertex(&verts[j], &pnt);
		verts[j].u = trip->verts[j].u;
		verts[j].v = trip->verts[j].v;
	}

	verts[0].r = r;
	verts[0].g = g;
	verts[0].b = b;
	verts[1].r = r;
	verts[1].g = g;
	verts[1].b = b;
	verts[2].r = r;
	verts[2].g = g;
	verts[2].b = b;
	verts[3].r = r;
	verts[3].g = g;
	verts[3].b = b;

	vec3d norm;
	vm_vec_perp(&norm, &trip->verts[0].pos, &trip->verts[1].pos, &trip->verts[2].pos);
	vertex* vertlist[4];
	if (vm_vec_dot(&norm, &trip->verts[1].pos) < 0.0)
	{
		vertlist[0] = &verts[3];
		vertlist[1] = &verts[2];
		vertlist[2] = &verts[1];
		vertlist[3] = &verts[0];
		g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_HTL_3D_UNLIT);
	}
	else
	{
		vertlist[0] = &verts[0];
		vertlist[1] = &verts[1];
		vertlist[2] = &verts[2];
		vertlist[3] = &verts[3];
		g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_HTL_3D_UNLIT);
	}
}

//	Render one triangle of a shield hit effect on one ship.
//	Each frame, the triangle needs to be rotated into global coords.
//	trip		pointer to triangle in global array
//	orient	orientation of object shield is associated with
//	pos		center point of object
void render_shield_triangle(gshield_tri* trip, matrix* orient, vec3d* pos, ubyte r, ubyte g, ubyte b)
{
	int j;
	vec3d pnt;
	vertex* verts[3];
	vertex points[3];

	if (trip->trinum == -1)
		return;	//	Means this is a quad, must have switched detail_level.

	for (j = 0; j < 3; j++)
	{
		// Rotate point into world coordinates
		vm_vec_unrotate(&pnt, &trip->verts[j].pos, orient);
		vm_vec_add2(&pnt, pos);

		// Pnt is now the x,y,z world coordinates of this vert.
		// For this example, I am just drawing a sphere at that point.

		if (!Cmdline_nohtl)
			g3_transfer_vertex(&points[j], &pnt);
		else
			g3_rotate_vertex(&points[j], &pnt);

		points[j].u = trip->verts[j].u;
		points[j].v = trip->verts[j].v;
		Assert((trip->verts[j].u >= 0.0f) && (trip->verts[j].u <= UV_MAX));
		Assert((trip->verts[j].v >= 0.0f) && (trip->verts[j].v <= UV_MAX));
		verts[j] = &points[j];
	}

	verts[0]->r = r;
	verts[0]->g = g;
	verts[0]->b = b;
	verts[1]->r = r;
	verts[1]->g = g;
	verts[1]->b = b;
	verts[2]->r = r;
	verts[2]->g = g;
	verts[2]->b = b;

	vec3d norm;
	Poly_count++;
	vm_vec_perp(&norm, (vec3d*)&verts[0]->x, (vec3d*)&verts[1]->x, (vec3d*)&verts[2]->x);

	int flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD;
	if (!Cmdline_nohtl)
		flags |= TMAP_HTL_3D_UNLIT;

	if (vm_vec_dot(&norm, (vec3d*)&verts[1]->x) >= 0.0)
	{
		vertex* vertlist[3];
		vertlist[0] = verts[2];
		vertlist[1] = verts[1];
		vertlist[2] = verts[0];
		g3_draw_poly(3, vertlist, flags);
	}
	else
	{
		g3_draw_poly(3, verts, flags);
	}
}

MONITOR(NumShieldRend)

//	Render a shield mesh in the global array Shield_hits[]
void render_shield(int shield_num) //, matrix *orient, vec3d *centerp)
{
	int i;
	vec3d* centerp;
	matrix* orient;
	object* objp;
	ship* shipp;
	ship_info* si;

	if (Shield_hits[shield_num].type == SH_UNUSED)
	{
		return;
	}

	Assert(Shield_hits[shield_num].objnum >= 0);

	objp = &Objects[Shield_hits[shield_num].objnum];

	if (objp->flags & OF_NO_SHIELDS)
	{
		return;
	}

	//	If this object didn't get rendered, don't render its shields.  In fact, make the shield hit go away.
	if (!(objp->flags & OF_WAS_RENDERED))
	{
		Shield_hits[shield_num].type = SH_UNUSED;
		return;
	}

	//	At detail levels 1, 3, animations play at double speed to reduce load.
	if ((Detail.shield_effects == 1) || (Detail.shield_effects == 3))
	{
		Shield_hits[shield_num].start_time -= Frametime;
	}

	MONITOR_INC(NumShieldRend, 1);

	shipp = &Ships[objp->instance];
	si = &Ship_info[shipp->ship_info_index];

	// objp, shipp, and si are now setup correctly

	//	If this ship is in its deathroll, make the shield hit effects go away faster.
	if (shipp->flags & SF_DYING)
	{
		Shield_hits[shield_num].start_time -= fl2f(2 * flFrametime);
	}

	//	Detail level stuff.  When lots of shield hits, maybe make them go away faster.
	if (Poly_count > 50)
	{
		if (Shield_hits[shield_num].start_time + (SHIELD_HIT_DURATION * 50) / Poly_count < Missiontime)
		{
			Shield_hits[shield_num].type = SH_UNUSED;
			free_global_tri_records(shield_num);
			// nprintf(("AI", "* "));
			return;
		}
	}
	else if ((Shield_hits[shield_num].start_time + SHIELD_HIT_DURATION) < Missiontime)
	{
		Shield_hits[shield_num].type = SH_UNUSED;
		free_global_tri_records(shield_num);
		return;
	}

	orient = &objp->orient;
	centerp = &objp->pos;

	int bitmap_id, frame_num;

	// mprintf(("Percent = %7.3f\n", f2fl(Missiontime - Shield_hits[shield_num].start_time)));

	// Do some sanity checking
	Assert((si->species >= 0) && (si->species < (int)Species_info.size()));

	generic_anim* sa = &Species_info[si->species].shield_anim;

	// don't try to draw if we don't have an ani
	if (sa->first_frame >= 0)
	{
		frame_num = fl2i(f2fl(Missiontime - Shield_hits[shield_num].start_time) * sa->num_frames);
		if (frame_num >= sa->num_frames)
		{
			frame_num = sa->num_frames - 1;
		}
		else if (frame_num < 0)
		{
			mprintf(("HEY! Missiontime went backwards! (Shield.cpp)\n"));
			frame_num = 0;
		}
		bitmap_id = sa->first_frame + frame_num;

		float alpha = 0.9999f;
		if (The_mission.flags & MISSION_FLAG_FULLNEB)
		{
			alpha *= 0.85f;
		}

		gr_set_bitmap(bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);

		if ((Detail.shield_effects == 1) || (Detail.shield_effects == 2))
		{
			if (bitmap_id != - 1)
			{
				render_low_detail_shield_bitmap(&Global_tris[Shield_hits[shield_num].tri_list[0]], orient, centerp,
					Shield_hits[shield_num].rgb[0], Shield_hits[shield_num].rgb[1], Shield_hits[shield_num].rgb[2]);
			}
		}
		else
		{

			// AL 06/01/97 don't use Assert() until issue with Missiontime being reset to 0 are worked out
			if (bitmap_id != - 1)
			{
				for (i = 0; i < Shield_hits[shield_num].num_tris; i++)
				{
					//if (Missiontime == Shield_hits[shield_num].start_time)
					//	nprintf(("AI", "Frame %i: Render triangle %i.\n", Framecount, Global_tris[Shield_hits[shield_num].tri_list[i]].trinum));
					render_shield_triangle(&Global_tris[Shield_hits[shield_num].tri_list[i]], orient, centerp,
						Shield_hits[shield_num].rgb[0], Shield_hits[shield_num].rgb[1],
						Shield_hits[shield_num].rgb[2]);
				}
			}
		}
	}
}

//	Render all the shield hits  in the global array Shield_hits[]
//	This is a temporary function.  Shield hit rendering will at least have to
// occur with the ship, perhaps even internal to the ship.
void render_shields()
{
	int i;

	if (Detail.shield_effects == 0)
	{
		return;	//	No shield effect rendered at lowest detail level.
	}

	for (i = 0; i < MAX_SHIELD_HITS; i++)
	{
		if (Shield_hits[i].type != SH_UNUSED)
		{
			render_shield(i);
		}
	}
}


// -----------------------------------------------------------------------------------------------------
void create_tris_containing(vec3d* vp, matrix* orient, shield_info* shieldp, vec3d* tcp, vec3d* centerp, float radius,
							vec3d* rvec, vec3d* uvec)
{
	int i, j;
	shield_vertex* verts;

	verts = shieldp->verts;

	for (i = 0; i < Num_tris; i++)
	{
		if (!shieldp->tris[i].used)
		{
			for (j = 0; j < 3; j++)
			{
				vec3d v;

				v = verts[shieldp->tris[i].verts[j]].pos;
				if ((vp->xyz.x == v.xyz.x) && (vp->xyz.y == v.xyz.y) && (vp->xyz.z == v.xyz.z))
					create_shield_from_triangle(i, orient, shieldp, tcp, centerp, radius, rvec, uvec);
			}
		}
	}
}

void visit_children(int trinum, int vertex_index, matrix* orient, shield_info* shieldp, vec3d* tcp, vec3d* centerp,
					float radius, vec3d* rvec, vec3d* uvec)
{
	shield_vertex* sv;

	sv = &(shieldp->verts[shieldp->tris[trinum].verts[vertex_index]]);

	if ((sv->u > 0.0f) && (sv->u < UV_MAX) && (sv->v > 0.0f) && (sv->v < UV_MAX))
		create_tris_containing(&sv->pos, orient, shieldp, tcp, centerp, radius, rvec, uvec);
}

int Gi_max = 0;

int get_free_global_shield_index()
{
	int gi = 0;

	while ((gi < MAX_SHIELD_TRI_BUFFER) && (Global_tris[gi].used) && (Global_tris[gi].
																	  creation_time + SHIELD_HIT_DURATION >
																	  Missiontime))
	{
		gi++;
	}

	//	If couldn't find one, choose a random one.
	if (gi == MAX_SHIELD_TRI_BUFFER)
		gi = (int)(frand() * MAX_SHIELD_TRI_BUFFER);

	return gi;
}

int get_global_shield_tri()
{
	int shnum;

	//	Find unused shield hit buffer
	for (shnum = 0; shnum < MAX_SHIELD_HITS; shnum++)
		if (Shield_hits[shnum].type == SH_UNUSED)
			break;

	if (shnum == MAX_SHIELD_HITS)
	{
		//nprintf(("AI", "Warning: Shield_hit buffer full!  Stealing an old one!\n"));
		shnum = myrand() % MAX_SHIELD_HITS;
	}

	Assert((shnum >= 0) && (shnum < MAX_SHIELD_HITS));

	return shnum;
}

void create_shield_from_triangle(int trinum, matrix* orient, shield_info* shieldp, vec3d* tcp, vec3d* centerp,
								 float radius, vec3d* rvec, vec3d* uvec)
{
	//nprintf(("AI", "[%3i] ", trinum));

	rs_compute_uvs(&shieldp->tris[trinum], shieldp->verts, tcp, radius, rvec, uvec);

	//Assert(trinum < MAX_SHIELD_HITS);
	shieldp->tris[trinum].used = 1;

	//mprintf(("%i ", trinum));
	visit_children(trinum, 0, orient, shieldp, tcp, centerp, radius, rvec, uvec);
	visit_children(trinum, 1, orient, shieldp, tcp, centerp, radius, rvec, uvec);
	visit_children(trinum, 2, orient, shieldp, tcp, centerp, radius, rvec, uvec);
}

//	Copy information from Current_tris to Global_tris, stuffing information
//	in a slot in Shield_hits.  The Global_tris array is not a shield_tri structure.
// We need to store vertex information in the global array since the vertex list
// will not be available to us when we actually use the array.
void copy_shield_to_globals(int objnum, shield_info* shieldp)
{
	int i, j;
	int gi = 0;
	int count = 0;			//	Number of triangles in this shield hit.
	int shnum;				//	shield hit number, index in Shield_hits.

	shnum = get_global_shield_tri();

	Shield_hits[shnum].type = SH_TYPE_1;
	// mprintf(("Creating hit #%i at time = %7.3f\n", shnum, f2fl(Missiontime)));

	for (i = 0; i < shieldp->ntris; i++)
	{
		if (shieldp->tris[i].used)
		{
			while ((gi < MAX_SHIELD_TRI_BUFFER) && (Global_tris[gi].used) && (Global_tris[gi].
																			  creation_time + SHIELD_HIT_DURATION >
																			  Missiontime))
			{
				gi++;
			}

			//	If couldn't find one, choose a random one.
			if (gi == MAX_SHIELD_TRI_BUFFER)
				gi = (int)(frand() * MAX_SHIELD_TRI_BUFFER);

			Global_tris[gi].used = shieldp->tris[i].used;
			Global_tris[gi].trinum = i;
			Global_tris[gi].creation_time = Missiontime;

			// copy the pos/u/v elements of the shield_vertex structure into the shield vertex structure for this global triangle.
			for (j = 0; j < 3; j++)
				Global_tris[gi].verts[j] = shieldp->verts[shieldp->tris[i].verts[j]];
			Shield_hits[shnum].tri_list[count++] = gi;

			if (count >= MAX_TRIS_PER_HIT)
			{
				mprintf(("Warning: Too many triangles in shield hit.\n"));
				break;
			}
		}
	}

	Shield_hits[shnum].num_tris = count;
	Shield_hits[shnum].start_time = Missiontime;
	Shield_hits[shnum].objnum = objnum;

	Shield_hits[shnum].rgb[0] = 255;
	Shield_hits[shnum].rgb[1] = 255;
	Shield_hits[shnum].rgb[2] = 255;
	if ((objnum >= 0) && (objnum < MAX_OBJECTS) && (Objects[objnum].type == OBJ_SHIP) &&
		(Objects[objnum].instance >= 0) && (Objects[objnum].instance < MAX_SHIPS) &&
		(Ships[Objects[objnum].instance].ship_info_index >= 0) && (Ships[Objects[objnum].instance].ship_info_index <
																   Num_ship_classes))
	{
		ship_info* sip = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];

		Shield_hits[shnum].rgb[0] = sip->shield_color[0];
		Shield_hits[shnum].rgb[1] = sip->shield_color[1];
		Shield_hits[shnum].rgb[2] = sip->shield_color[2];
	}
}

//	***** This is the version that works on a quadrant basis.
//	Return absolute amount of damage not applied.
float apply_damage_to_shield(object* objp, int quadrant, float damage)
{
	ai_info* aip;

	// multiplayer clients bail here if nodamage
	// if(MULTIPLAYER_CLIENT && (Netgame.debug_flags & NETD_FLAG_CLIENT_NODAMAGE)){
	if (MULTIPLAYER_CLIENT)
	{
		return damage;
	}

	if ((quadrant < 0) || (quadrant >= MAX_SHIELD_SECTIONS))
		return damage;

	Assert(objp->type == OBJ_SHIP);
	aip = &Ai_info[Ships[objp->instance].ai_index];
	aip->last_hit_quadrant = quadrant;

	objp->shield_quadrant[quadrant] -= damage;

	if (objp->shield_quadrant[quadrant] < 0.0f)
	{
		float remaining_damage;

		remaining_damage = -objp->shield_quadrant[quadrant];
		objp->shield_quadrant[quadrant] = 0.0f;
		//nprintf(("AI", "Applied %7.3f damage to quadrant #%i, %7.3f passes through\n", damage - remaining_damage, quadrant_num, remaining_damage));
		return remaining_damage;
	}
	else
	{
		//nprintf(("AI", "Applied %7.3f damage to quadrant #%i\n", damage, quadrant_num));
		return 0.0f;
	}

}
/**
 * This function needs to be called by big ships which have shields. It should be able to be modified to deal with
 * the large polygons we use for their shield meshes - unknownplayer
 */
//	At lower detail levels, shield hit effects are a single texture, applied to one enlarged triangle.
void create_shield_low_detail(int objnum, int model_num, matrix* orient, vec3d* centerp, vec3d* tcp, int tr0,
							  shield_info* shieldp)
{
	matrix tom;
	int gi;
	int shnum;

	shnum = get_global_shield_tri();
	Shield_hits[shnum].type = SH_TYPE_1;

	gi = get_free_global_shield_index();

	Global_tris[gi].used = 1;
	Global_tris[gi].trinum = -1;		//	This tells triangle renderer to not render in case detail_level was switched.
	Global_tris[gi].creation_time = Missiontime;

	Shield_hits[shnum].tri_list[0] = gi;
	Shield_hits[shnum].num_tris = 1;
	Shield_hits[shnum].start_time = Missiontime;
	Shield_hits[shnum].objnum = objnum;

	Shield_hits[shnum].rgb[0] = 255;
	Shield_hits[shnum].rgb[1] = 255;
	Shield_hits[shnum].rgb[2] = 255;
	if ((objnum >= 0) && (objnum < MAX_OBJECTS) && (Objects[objnum].type == OBJ_SHIP) &&
		(Objects[objnum].instance >= 0) && (Objects[objnum].instance < MAX_SHIPS) &&
		(Ships[Objects[objnum].instance].ship_info_index >= 0) && (Ships[Objects[objnum].instance].ship_info_index <
																   Num_ship_classes))
	{
		ship_info* sip = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];

		Shield_hits[shnum].rgb[0] = sip->shield_color[0];
		Shield_hits[shnum].rgb[1] = sip->shield_color[1];
		Shield_hits[shnum].rgb[2] = sip->shield_color[2];
	}

	vm_vector_2_matrix(&tom, &shieldp->tris[tr0].norm, NULL, NULL);
	//rs_compute_uvs( &shieldp->tris[tr0], shieldp->verts, tcp, Objects[objnum].radius, &tom.rvec, &tom.uvec);

	create_low_detail_poly(gi, tcp, &tom.vec.rvec, &tom.vec.uvec);

}

// Algorithm for shrink-wrapping a texture across a triangular mesh.
// 
// - Given a point of intersection, tcp (local to objnum)
// - Vector to center of shield from tcp is v2c.
// - Using v2c, compute right and down vectors.  These are the vectors of
//   increasing u and v, respectively.
// - Triangle of intersection of tcp is tr0.
// - For 3 points in tr0, compute u,v coordinates using up and down vectors
//   from center point, tcp.  Need to know size of explosion texture.  N units
//   along right vector corresponds to O units in explosion texture space.
// - For each edge, if either endpoint was outside texture bounds, recursively
//   apply previous and current step.
// 
// Output of above is a list of triangles with u,v coordinates.  These u,v
// coordinates will have to be clipped against the explosion texture bounds.

void create_shield_explosion(int objnum, int model_num, matrix* orient, vec3d* centerp, vec3d* tcp, int tr0)
{
	//	vec3d	v2c;		//	Vector to center from point tcp
	matrix tom;		//	Texture Orientation Matrix
	//	float		radius;	// Radius of shield, computed as distance from tcp to objp->pos.
	shield_info* shieldp;
	polymodel* pm;
	int i;

	if (Objects[objnum].flags & OF_NO_SHIELDS)
		return;

	pm = model_get(model_num);
	Num_tris = pm->shield.ntris;
	//Assert(Num_tris < MAX_SHIELD_HITS);
	shieldp = &pm->shield;

	if (Num_tris == 0)
		return;

	//nprintf(("AI", "Frame %i: Creating explosion on %i.\n", Framecount, objnum));

	if ((Detail.shield_effects == 1) || (Detail.shield_effects == 2))
	{
		create_shield_low_detail(objnum, model_num, orient, centerp, tcp, tr0, shieldp);
		return;
	}

	for (i = 0; i < Num_tris; i++)
		shieldp->tris[i].used = 0;

	//	Compute orientation matrix from normal of surface hit.
	//	Note, this will cause the shape of the bitmap to change abruptly
	//	as the impact point moves to another triangle.  To prevent this,
	//	you could average the normals at the vertices, then interpolate the
	//	normals from the vertices to get a smoothly changing normal across the face.
	//	I had tried using the vector from the impact point to the center, which
	//	changes smoothly, but this looked surprisingly bad.
	vm_vector_2_matrix(&tom, &shieldp->tris[tr0].norm, NULL, NULL);
	//vm_vec_sub(&v2c, tcp, &Objects[objnum].pos);

	//	Create the shield from the current triangle, as well as its neighbors.
	create_shield_from_triangle(tr0, orient, shieldp, tcp, centerp, Objects[objnum].radius, &tom.vec.rvec, &tom.vec.
		uvec);
	//nprintf(("AI", "\n"));
	for (i = 0; i < 3; i++)
	{
		create_shield_from_triangle(shieldp->tris[tr0].neighbors[i], orient, shieldp, tcp, centerp,
			Objects[objnum].radius, &tom.vec.rvec, &tom.vec.uvec);

		for (int j = 0; j < 3; j++)
		{
			create_shield_from_triangle(shieldp->tris[shieldp->tris[tr0].neighbors[i]].neighbors[j], orient, shieldp,
				tcp, centerp, Objects[objnum].radius, &tom.vec.rvec, &tom.vec.uvec);
		}
	}

	copy_shield_to_globals(objnum, shieldp);
	// render_shield(orient, centerp);
}

MONITOR(NumShieldHits)

//	Add data for a shield hit.
void add_shield_point(int objnum, int tri_num, vec3d* hit_pos)
{
	//Assert(Num_shield_points < MAX_SHIELD_POINTS);
	if (Num_shield_points >= MAX_SHIELD_POINTS)
		return;

	MONITOR_INC(NumShieldHits, 1);

	Shield_points[Num_shield_points].objnum = objnum;
	Shield_points[Num_shield_points].shield_tri = tri_num;
	Shield_points[Num_shield_points].hit_point = *hit_pos;

	Num_shield_points++;

	// in multiplayer -- send the shield hit data to the clients
	// if ( MULTIPLAYER_MASTER && !(Netgame.debug_flags & NETD_FLAG_CLIENT_NODAMAGE)){
	// send_shield_explosion_packet( objnum, tri_num, *hit_pos );
	// }

	Ships[Objects[objnum].instance].shield_hits++;
}

// ugh!  I wrote a special routine to store shield points for clients in multiplayer
// games.  Problem is initilization and flow control of normal gameplay make this problem
// more than trivial to solve.  Turns out that I think I can just keep track of the
// shield_points for multiplayer in a separate count -- then assign the multi count to
// the normal count at the correct time.
void add_shield_point_multi(int objnum, int tri_num, vec3d* hit_pos)
{
	//Assert(Num_multi_shield_points < MAX_SHIELD_POINTS);

	if (Num_multi_shield_points >= MAX_SHIELD_POINTS)
		return;

	Shield_points[Num_shield_points].objnum = objnum;
	Shield_points[Num_shield_points].shield_tri = tri_num;
	Shield_points[Num_shield_points].hit_point = *hit_pos;

	Num_multi_shield_points++;
}

// sets up the shield point hit information for multiplayer clients
void shield_point_multi_setup()
{
	int i;

	Assert(MULTIPLAYER_CLIENT);

	if (Num_multi_shield_points == 0)
		return;

	Num_shield_points = Num_multi_shield_points;
	for (i = 0; i < Num_shield_points; i++)
	{
		Ships[Objects[Shield_points[i].objnum].instance].shield_hits++;
	}

	Num_multi_shield_points = 0;
}


//	Create all the shield explosions that occurred on object *objp this frame.
void create_shield_explosion_all(object* objp)
{
	int i;
	int num;
	int count;
	int objnum;
	ship* shipp;

	if (Detail.shield_effects == 0)
	{
		return;
	}

	num = objp->instance;
	shipp = &Ships[num];

	count = shipp->shield_hits;
	objnum = objp - Objects;

	for (i = 0; i < Num_shield_points; i++)
	{
		if (Shield_points[i].objnum == objnum)
		{
			create_shield_explosion(objnum, Ship_info[shipp->ship_info_index].model_num, &objp->orient, &objp->
				pos, &Shield_points[i].hit_point, Shield_points[i].shield_tri);
			count--;
			if (count <= 0)
			{
				break;
			}
		}
	}

	//mprintf(("Creating %i explosions took %7.3f seconds\n", shipp->shield_hits, (float) (timer_get_milliseconds() - start_time)/1000.0f));

	// some some reason, clients seem to have a bogus count valud on occation.  I"ll chalk it up
	// to missed packets :-)  MWA 2/6/98
	if (!MULTIPLAYER_CLIENT)
	{
		Assert(count == 0);	//	Couldn't find all the alleged shield hits.  Bogus!
		//cometed out becase fighterbeams were tripping this -Bobboau
		// urg - Bobboau, please fix bugs; don't hide them :-/
	}
}

int Break_value = -1;

//	This is a debug function.
//	Draw the whole shield as a wireframe mesh, not looking at the current
//	integrity.
#ifndef NDEBUG
void ship_draw_shield(object* objp)
{
	int model_num;
	int i;
	vec3d pnt;
	polymodel* pm;

	if (objp->flags & OF_NO_SHIELDS)
		return;

	Assert(objp->instance >= 0);

	model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num;

	if (Fred_running)
		return;

	pm = model_get(model_num);

	if (pm->shield.ntris < 1)
		return;

	//	Scan all the triangles in the mesh.
	for (i = 0; i < pm->shield.ntris; i++)
	{
		int j;
		vec3d gnorm, v2f, tri_point;
		vertex prev_pnt, pnt0;
		shield_tri* tri;

		tri = &pm->shield.tris[i];

		if (i == Break_value)
			Int3();

		//	Hack! Only works for object in identity orientation.
		//	Need to rotate eye position into object's reference frame.
		//	Only draw facing triangles.
		vm_vec_rotate(&tri_point, &pm->shield.verts[tri->verts[0]].pos, &Eye_matrix);
		vm_vec_add2(&tri_point, &objp->pos);

		vm_vec_sub(&v2f, &tri_point, &Eye_position);
		vm_vec_unrotate(&gnorm, &tri->norm, &objp->orient);

		if (vm_vec_dot(&gnorm, &v2f) < 0.0f)
		{
			int intensity;

			intensity = (int)(Ships[objp->instance].shield_integrity[i] * 255);

			if (intensity < 0)
				intensity = 0;
			else if (intensity > 255)
				intensity = 255;

			gr_set_color(0, 0, intensity);

			//	Process the vertices.
			//	Note this rotates each vertex each time it's needed, very dumb.
			for (j = 0; j < 3; j++)
			{
				vertex tmp;

				// Rotate point into world coordinates
				vm_vec_unrotate(&pnt, &pm->shield.verts[tri->verts[j]].pos, &objp->orient);
				//vm_vec_unrotate(&pnt,&pm->shield[i].pnt[j],objp->orient);
				vm_vec_add2(&pnt, &objp->pos);

				// Pnt is now the x,y,z world coordinates of this vert.
				// For this example, I am just drawing a sphere at that
				// point.
				g3_rotate_vertex(&tmp, &pnt);

				if (j)
					g3_draw_line(&prev_pnt, &tmp);
				else
					pnt0 = tmp;
				prev_pnt = tmp;
			}

			g3_draw_line(&pnt0, &prev_pnt);
		}
	}
}
#endif

// Returns true if the shield presents any opposition to something 
// trying to force through it.
// If quadrant is -1, looks at entire shield, otherwise
// just one quadrant
int ship_is_shield_up(object* obj, int quadrant)
{
	if ((quadrant >= 0) && (quadrant < MAX_SHIELD_SECTIONS))
	{
		// Just check one quadrant
		if (obj->shield_quadrant[quadrant] > MAX(2.0f, 0.1f * get_max_shield_quad(obj)))
		{
			return 1;
		}
	}
	else
	{
		// Check all quadrants
		float strength = shield_get_strength(obj);

		if (strength > MAX(2.0f * 4.0f, 0.1f * Ships[obj->instance].ship_max_shield_strength))
		{
			return 1;
		}
	}
	return 0;	// no shield strength
}


/*
//-- CODE TO "BOUNCE" AN ARRAY FROM A GIVEN POINT.
//-- LIKE A MATTRESS.
#define BOUNCE_SIZE ???

byte Bouncer1[BOUNCE_SIZE];
byte Bouncer2[BOUNCE_SIZE];

byte * Bouncer = Bouncer1;
byte * OldBouncer = Bouncer2;

// To wiggle, add value to Bouncer[] 

void bounce_it()
{
	int i, tmp;


	for (i=0; i<BOUNCE_SIZE; i++ )	{
 		int t = 0;

		t += OldBouncer[ LEFT ];
		t += OldBouncer[ RIGHT ];
		t += OldBouncer[ UP ];
		t += OldBouncer[ DOWN ];

		t = (t/2) - Bouncer[i];
		tmp = t - t/16;		// 8
		
		if ( tmp < -127 ) tmp = -127;
		if ( tmp > 127 ) tmp = 127;
		Bouncer[i] = tmp;
	}

	if ( Bouncer == Bouncer1 )	{
		OldBouncer = Bouncer1;
		Bouncer = Bouncer2;
	} else {
		OldBouncer = Bouncer2;
		Bouncer = Bouncer1;
	}
}
*/

#else

// stub out shield functions for the demo
void shield_hit_init() {}
void create_shield_explosion_all(object *objp) {}
void shield_frame_init() {}
void add_shield_point(int objnum, int tri_num, vec3d *hit_pos) {}
void add_shield_point_multi(int objnum, int tri_num, vec3d *hit_pos) {}
void shield_point_multi_setup() {}
void shield_hit_close() {}
void ship_draw_shield( object *objp) {}
void shield_hit_page_in() {}
void render_shields() {}
float apply_damage_to_shield(object *objp, int quadrant, float damage) {return damage;}
int ship_is_shield_up( object *obj, int quadrant ) {return 0;}

#endif // DEMO


//	return quadrant containing hit_pnt.
//	\  1  /.
//	3 \ / 0
//	  / \.
//	/  2  \.
//	Note: This is in the object's local reference frame.  Do _not_ pass a vector in the world frame.
int get_quadrant(vec3d* hit_pnt)
{
	int result = 0;

	if (hit_pnt->xyz.x < hit_pnt->xyz.z)
		result |= 1;

	if (hit_pnt->xyz.x < -hit_pnt->xyz.z)
		result |= 2;

	return result;
}
