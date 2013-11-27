/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "object/objcollide.h"
#include "object/object.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "ship/shiphit.h"
#include "playerman/player.h"
#include "hud/hudshield.h"
#include "hud/hudwingmanstatus.h"
#include "io/timer.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "parse/lua.h"
#include "parse/scripting.h"
#include "ship/shipfx.h"
#include "particle/particle.h"


extern float ai_endangered_time(object* ship_objp, object* weapon_objp);
int check_inside_radius_for_big_ships(object* ship, object* weapon, obj_pair* pair);
float estimate_ship_speed_upper_limit(object* ship, float time);
extern float flFrametime;
extern int Cmdline_decals;


//	If weapon_obj is likely to hit ship_obj sooner than current aip->danger_weapon_objnum,
//	then update danger_weapon_objnum.
void update_danger_weapon(object* ship_obj, object* weapon_obj)
{
	ai_info* aip;

	Assert(ship_obj->type == OBJ_SHIP);

	aip = &Ai_info[Ships[ship_obj->instance].ai_index];

	if (aip->danger_weapon_objnum == -1)
	{
		aip->danger_weapon_objnum = weapon_obj - Objects;
		aip->danger_weapon_signature = weapon_obj->signature;
	}
	else if (aip->danger_weapon_signature == Objects[aip->danger_weapon_objnum].signature)
	{
		float danger_old_time, danger_new_time;

		danger_old_time = ai_endangered_time(ship_obj, &Objects[aip->danger_weapon_objnum]);
		danger_new_time = ai_endangered_time(ship_obj, weapon_obj);

		if (danger_new_time < danger_old_time)
		{
			aip->danger_weapon_objnum = weapon_obj - Objects;
			aip->danger_weapon_signature = weapon_obj->signature;
		}
	}
}

// function to actually deal with weapon-ship hit stuff.  separated from check_collision routine below
// because of multiplayer reasons.
void ship_weapon_do_hit_stuff(object* ship_obj, object* weapon_obj, vec3d* world_hitpos, vec3d* hitpos,
							  int quadrant_num, int submodel_num, vec3d /*not a pointer intentionaly*/ hit_dir)
{
	weapon* wp = &Weapons[weapon_obj->instance];
	weapon_info* wip = &Weapon_info[wp->weapon_info_index];
	ship* shipp = &Ships[ship_obj->instance];
	float damage;
	vec3d force;

	// Apply hit & damage & stuff to weapon
	weapon_hit(weapon_obj, ship_obj, world_hitpos, quadrant_num);

	damage = wip->damage;

	// deterine whack whack
	float blast = wip->mass;
	vm_vec_copy_scale(&force, &weapon_obj->phys_info.vel, blast);

	// send player pain packet
	if ((MULTIPLAYER_MASTER) && !(shipp->flags & SF_DYING))
	{
		int np_index = multi_find_player_by_object(ship_obj);

		// if this is a player ship
		if ((np_index >= 0) && (np_index != MY_NET_PLAYER_NUM) && (wip->subtype == WP_LASER))
		{
			send_player_pain_packet(&Net_players[np_index], wp->weapon_info_index,
				wip->damage * weapon_get_damage_scale(wip, weapon_obj, ship_obj), &force, hitpos, quadrant_num);
		}
	}

	ship_apply_local_damage(ship_obj, weapon_obj, world_hitpos, damage, quadrant_num, CREATE_SPARKS, submodel_num);

	// let the hud shield gauge know when Player or Player target is hit
	hud_shield_quadrant_hit(ship_obj, quadrant_num);

	// Let wingman status gauge know a wingman ship was hit
	if ((Ships[ship_obj->instance].wing_status_wing_index >= 0) && ((Ships[ship_obj->instance].wing_status_wing_pos >=
																	 0)))
	{
		hud_wingman_status_start_flash(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
	}

	// Apply a wack.  This used to be inside of ship_hit... duh! Ship_hit
	// is to apply damage, not physics, so I moved it here.
	// don't apply whack for multiplayer_client from laser - will occur with pain packet
	if (!((wip->subtype == WP_LASER) && MULTIPLAYER_CLIENT))
	{
		// apply a whack		
		ship_apply_whack(&force, hitpos, ship_obj);
	}

	if ((quadrant_num == -1) && Cmdline_decals)
	{
		weapon_info* wip = &Weapon_info[Weapons[weapon_obj->instance].weapon_info_index];
		decal_point dec;
		dec.orient = weapon_obj->orient;
		vec3d hit_fvec;
		vm_vec_negate(&hit_dir);
		vm_vec_avg(&hit_fvec, &hit_dir, &weapon_obj->orient.vec.fvec);
		vm_vec_normalize(&hit_fvec);
		dec.orient.vec.fvec = hit_fvec;
		vm_fix_matrix(&dec.orient);
		dec.pnt.xyz = hitpos->xyz;
		dec.radius = wip->decal_rad;

		if ((dec.radius > 0) && (wip->decal_texture.bitmap_id > -1))
			decal_create(ship_obj, &dec, submodel_num, wip->decal_texture.bitmap_id,
				wip->decal_backface_texture.bitmap_id, wip->decal_glow_texture_id, wip->decal_burn_texture_id,
				wip->decal_burn_time);
	}


}

extern int Framecount;

int ship_weapon_check_collision(object* ship_objp, object* weapon_objp, float time_limit = 0.0f, int* next_hit = NULL)
{
	mc_info mc, mc_shield, mc_hull;
	ship* shipp;
	ship_info* sip;
	weapon* wp;
	weapon_info* wip;

	Assert(ship_objp != NULL);
	Assert(ship_objp->type == OBJ_SHIP);
	Assert(ship_objp->instance >= 0);

	shipp = &Ships[ship_objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	Assert(weapon_objp != NULL);
	Assert(weapon_objp->type == OBJ_WEAPON);
	Assert(weapon_objp->instance >= 0);

	wp = &Weapons[weapon_objp->instance];
	wip = &Weapon_info[wp->weapon_info_index];


	Assert(shipp->objnum == OBJ_INDEX(ship_objp));

	// Make ships that are warping in not get collision detection done
	if (shipp->flags & SF_ARRIVING)
		return 0;

	// if one object is a capital, only check player and player weapons with
	// the capital -- too slow for now otherwise.
	//	if ( Polygon_models[Ships[num].modelnum].use_grid && !( (other_objp == Player_obj) || (&Objects[other_objp->parent] == Player_obj)) )
	//		return 0;

	//	If either of these objects doesn't get collision checks, abort.
	if (Ship_info[shipp->ship_info_index].flags & SIF_NO_COLLIDE)
		return 0;

	//	Return information for AI to detect incoming fire.
	//	Could perhaps be done elsewhere at lower cost --MK, 11/7/97
	float dist = vm_vec_dist_quick(&ship_objp->pos, &weapon_objp->pos);
	if (dist < weapon_objp->phys_info.speed)
	{
		update_danger_weapon(ship_objp, weapon_objp);
	}

	ship_model_start(ship_objp);

	int valid_hit_occurred = 0;				// If this is set, then hitpos is set
	int quadrant_num = -1;
	polymodel* pm = model_get(sip->model_num);

	//	total time is flFrametime + time_limit (time_limit used to predict collisions into the future)
	vec3d weapon_end_pos;
	vm_vec_scale_add(&weapon_end_pos, &weapon_objp->pos, &weapon_objp->phys_info.vel, time_limit);


	// Goober5000 - I tried to make collision code here much saner... here begin the (major) changes

	// set up collision structs
	mc.model_num = sip->model_num;
	mc.submodel_num = -1;
	mc.orient = &ship_objp->orient;
	mc.pos = &ship_objp->pos;
	mc.p0 = &weapon_objp->last_pos;
	mc.p1 = &weapon_end_pos;
	mc.radius = 0.01f;
	mc.flags = MC_CHECK_SPHERELINE;
	memcpy(&mc_shield, &mc, sizeof(mc_info));
	memcpy(&mc_hull, &mc, sizeof(mc_info));

	// (btw, these are leftover comments from below...)
	//
	//	Note: This code is obviously stupid. We want to add the shield point if there is shield to hit, but:
	//		1. We want the size/color of the hit effect to indicate shield damage done.  (i.e., for already-weak shield, smaller effect)
	//		2. Currently (8/9/97), apply_damage_to_shield() passes lefer damage to hull, which might not make sense.  If
	//			wouldn't have collided with hull, shouldn't do damage.  Once this is fixed, the code below needs to cast the
	//			vector through to the hull if there is leftover damage.
	//
	// WIF2_PIERCE_SHIELDS pierces shields
	// AL 1-14-97: "Puncture" doesn't mean penetrate shield anymore, it means that it punctures
	//					hull to inflict maximum subsystem damage
	//
	// _argv[-1], 16 Jan 2005: Surface shields.
	// Surface shields allow for shields on a ship without a shield mesh.  Good for putting real shields
	// on the Lucifer.  This also fixes the strange bug where shots will occasionally go through the
	// shield mesh when they shouldn't.  I don't know what causes this, but this fixes that -- shields
	// will absorb it when it hits the hull instead.  This has no fancy graphical effect, though.
	// Someone should make one.

	// set flags
	mc_shield.flags |= MC_CHECK_SHIELD;
	mc_hull.flags |= MC_CHECK_MODEL;

	// check both kinds of collisions
	int shield_collision = (pm->shield.ntris > 0) ? model_collide(&mc_shield) : 0;
	int hull_collision = model_collide(&mc_hull);
	bool shield_effect_needed = false;

	// check shields for impact
	if (!(ship_objp->flags & OF_NO_SHIELDS))
	{
		// pick out the shield quadrant
		if (shield_collision)
		{
			quadrant_num = get_quadrant(&mc_shield.hit_point);
		}

		// KeldorKatarn:
		// Plug even the last hole in the shield
		if (hull_collision && !shield_collision && (pm->shield.ntris > 0) && !(wip->wi_flags2 & WIF2_PIERCE_SHIELDS))
		{
			mc_info mc_ray_shield;

			memcpy(&mc_ray_shield, &mc_shield, sizeof(mc_info));

			mc_ray_shield.p0 = &weapon_objp->last_pos;
			mc_ray_shield.p1 = &mc_hull.hit_point_world;

			vm_vec_scale_add(mc_ray_shield.p0, mc_ray_shield.p1, &weapon_objp->phys_info.vel, -pm->rad * 10.0f);

			int redundant_shield_collision = model_collide(&mc_ray_shield);

			float dist_to_shield = vm_vec_dist_squared(&mc_ray_shield.hit_point_world, &mc_hull.hit_point_world);
			float dist_to_start = vm_vec_dist_squared(&wp->start_pos, &mc_hull.hit_point_world);

			if (redundant_shield_collision && (dist_to_shield < dist_to_start))
			{
				int redundant_quadrant_num = get_quadrant(&mc_ray_shield.hit_point);

				if (ship_is_shield_up(ship_objp, redundant_quadrant_num))
				{
					shield_collision = redundant_shield_collision;
					quadrant_num = redundant_quadrant_num;

					float hit_dist = vm_vec_dist(mc_shield.p0, &mc_ray_shield.hit_point_world);

					memcpy(&mc_shield, &mc_ray_shield, sizeof(mc_info));
					mc_shield.hit_dist = hit_dist;
				}
			}
		}

		if (!shield_collision && hull_collision && (sip->flags2 & SIF2_SURFACE_SHIELDS))
		{
			vec3d local_pos, local_pos_rot;
			vm_vec_sub(&local_pos, &mc_hull.hit_point_world, &ship_objp->pos);
			vm_vec_rotate(&local_pos_rot, &local_pos, &ship_objp->orient);

			quadrant_num = get_quadrant(&local_pos_rot);
		}

		// make sure that the shield is active in that quadrant
		if ((quadrant_num >= 0) && ((shipp->flags & SF_DYING) || !ship_is_shield_up(ship_objp, quadrant_num)))
			quadrant_num = -1;

		// see if we hit the shield
		if (quadrant_num >= 0)
		{
			shield_effect_needed = true;

			// if this weapon pierces the shield, then do the hit effect, but act like a shield collision never occurred;
			// otherwise, we have a valid hit on this shield
			if (wip->wi_flags2 & WIF2_PIERCE_SHIELDS)
				quadrant_num = -1;
			else
				valid_hit_occurred = 1;
		}
	}

	// see which impact we use
	if (shield_collision && valid_hit_occurred)
	{
		memcpy(&mc, &mc_shield, sizeof(mc_info));
		Assert(quadrant_num >= 0);
	}
	else if (hull_collision)
	{
		memcpy(&mc, &mc_hull, sizeof(mc_info));
		valid_hit_occurred = 1;
	}


	ship_model_stop(ship_objp);

	// check if the hit point is beyond the clip plane when warping out.
	if ((shipp->flags & SF_DEPART_WARP) &&
		(shipp->warpout_effect) &&
		(valid_hit_occurred))
	{
		vec3d warp_pnt, hit_direction;
		matrix warp_orient;

		shipp->warpout_effect->getWarpPosition(&warp_pnt);
		shipp->warpout_effect->getWarpOrientation(&warp_orient);

		vm_vec_sub(&hit_direction, &mc.hit_point_world, &warp_pnt);

		if (vm_vec_dot(&hit_direction, &warp_orient.vec.fvec) < 0.0f)
		{
			valid_hit_occurred = false;
		}
	}

	// deal with predictive collisions.  Find their actual hit time and see if they occured in current frame
	if (next_hit)
	{
		// find hit time
		*next_hit = (int)(1000.0f * (mc.hit_dist * (flFrametime + time_limit) - flFrametime));

		if (*next_hit > 0)
		{
			shield_effect_needed = false;

			if (valid_hit_occurred)
			{
				// if hit occurs outside of this frame, do not do damage 
				return 1;
			}
		}
	}

	if (shield_effect_needed)
	{
		mc_info* mc = shield_collision ? &mc_shield : &mc_hull;
		do_shield_effect(ship_objp, mc, (shield_collision != 0));
	}

	if (valid_hit_occurred)
	{
		Script_system.SetHookObjects(4, "Ship", ship_objp, "Weapon", weapon_objp, "Self", ship_objp, "Object",
			weapon_objp);
		bool ship_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, ship_objp);

		Script_system.SetHookObjects(2, "Self", weapon_objp, "Object", ship_objp);
		bool weapon_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, weapon_objp);

		if (!ship_override && !weapon_override)
		{
			ship_weapon_do_hit_stuff(ship_objp, weapon_objp, &mc.hit_point_world, &mc.hit_point, quadrant_num,
				mc.hit_submodel, mc.hit_normal);
		}

		Script_system.SetHookObjects(2, "Self", ship_objp, "Object", weapon_objp);
		if (!(weapon_override && !ship_override))
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, ship_objp);

		Script_system.SetHookObjects(2, "Self", weapon_objp, "Object", ship_objp);
		if ((weapon_override && !ship_override) || (!weapon_override && !ship_override))
			Script_system.RunCondition(CHA_COLLIDESHIP, '\0', NULL, weapon_objp);

		Script_system.RemHookVars(4, "Ship", "Weapon", "Self", "Object");
	}

	return valid_hit_occurred;
}


// Checks ship-weapon collisions.  pair->a is ship and pair->b is weapon.
// Returns 1 if all future collisions between these can be ignored
int collide_ship_weapon(obj_pair* pair)
{
	int did_hit;
	object* ship = pair->a;
	object* weapon = pair->b;

	Assert(ship->type == OBJ_SHIP);
	Assert(weapon->type == OBJ_WEAPON);

	// Don't check collisions for player if past first warpout stage.
	if (Player->control_mode > PCM_WARPOUT_STAGE1)
	{
		if (ship == Player_obj)
			return 0;
	}

	if (reject_due_collision_groups(ship, weapon))
		return 0;

	// Cull lasers within big ship spheres by casting a vector forward for (1) exit sphere or (2) lifetime of laser
	// If it does hit, don't check the pair until about 200 ms before collision.  
	// If it does not hit and is within error tolerance, cull the pair.
	if ((Ship_info[Ships[ship->instance].ship_info_index].flags & (SIF_BIG_SHIP |
																   SIF_HUGE_SHIP)) &&
		(Weapon_info[Weapons[weapon->instance].weapon_info_index].subtype == WP_LASER))
	{
		if (vm_vec_dist_squared(&ship->pos, &weapon->pos) < (1.2f * ship->radius * ship->radius))
		{
			return check_inside_radius_for_big_ships(ship, weapon, pair);
		}
	}

	did_hit = ship_weapon_check_collision(ship, weapon);

	if (!did_hit)
	{
		// Since we didn't hit, check to see if we can disable all future collisions
		// between these two.
		return weapon_will_never_hit(weapon, ship, pair);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// upper limit estimate ship speed at end of time
float estimate_ship_speed_upper_limit(object* ship, float time)
{
	float exponent;
	float delta_v;
	float factor;

	delta_v = Ship_info[Ships[ship->instance].ship_info_index].max_vel.xyz.z - ship->phys_info.speed;
	if (ship->phys_info.forward_accel_time_const == 0)
	{
		return ship->phys_info.speed;
	}
	exponent = time / ship->phys_info.forward_accel_time_const;
	//Assert( exponent >= 0);


	factor = 1.0f - (float)exp(-exponent);
	return ship->phys_info.speed + factor * delta_v;
}

// maximum error allowed in detecting collisions between laser and big ship inside the radius
// this is set for a 150 m radius ship.  For ships with larger radius, the error scales according
// to the ration of radii, but is never less than 2 m
#define ERROR_STD	2

// ----------------------------------------------------------------------------							
// check_inside_radius_for_big_ships()							
// when inside radius of big ship, check if we can cull collision pair
// determine the time when pair should next be checked
// return 1 if pair can be culled
// return 0 if pair can not be culled
int check_inside_radius_for_big_ships(object* ship, object* weapon, obj_pair* pair)
{
	vec3d error_vel;		// vel perpendicular to laser
	float error_vel_mag;	// magnitude of error_vel
	float time_to_max_error, time_to_exit_sphere;
	float ship_speed_at_exit_sphere, error_at_exit_sphere;
	float max_error = (float)ERROR_STD / 150.0f * ship->radius;
	if (max_error < 2)
		max_error = 2.0f;

	time_to_exit_sphere = (ship->radius + vm_vec_dist(&ship->pos, &weapon->pos)) / (weapon->phys_info.max_vel.xyz.z -
																					ship->phys_info.max_vel.xyz.z);
	ship_speed_at_exit_sphere = estimate_ship_speed_upper_limit(ship, time_to_exit_sphere);
	// update estimated time to exit sphere
	time_to_exit_sphere = (ship->radius + vm_vec_dist(&ship->pos, &weapon->pos)) / (weapon->phys_info.max_vel.xyz.z -
																					ship_speed_at_exit_sphere);
	vm_vec_scale_add(&error_vel, &ship->phys_info.vel, &weapon->orient.vec.fvec, -vm_vec_dotprod(&ship->phys_info.
		vel, &weapon->orient.vec.fvec));
	error_vel_mag = vm_vec_mag_quick(&error_vel);
	error_vel_mag += 0.5f * (ship->phys_info.max_vel.xyz.z - error_vel_mag) * (time_to_exit_sphere /
																			   ship->phys_info.
																			   forward_accel_time_const);
	// error_vel_mag is now average velocity over period
	error_at_exit_sphere = error_vel_mag * time_to_exit_sphere;
	time_to_max_error = max_error / error_at_exit_sphere * time_to_exit_sphere;

	// find the minimum time we can safely check into the future.
	// limited by (1) time to exit sphere (2) time to weapon expires
	// if ship_weapon_check_collision comes back with a hit_time > error limit, ok
	// if ship_weapon_check_collision comes finds no collision, next check time based on error time
	float limit_time;		// furthest time to check (either lifetime or exit sphere)
	if (time_to_exit_sphere < Weapons[weapon->instance].lifeleft)
	{
		limit_time = time_to_exit_sphere;
	}
	else
	{
		limit_time = Weapons[weapon->instance].lifeleft;
	}

	// Note:  when estimated hit time is less than 200 ms, look at every frame
	int hit_time;	// estimated time of hit in ms

	// modify ship_weapon_check_collision to do damage if hit_time is negative (ie, hit occurs in this frame)
	if (ship_weapon_check_collision(ship, weapon, limit_time, &hit_time))
	{
		// hit occured in while in sphere
		if (hit_time < 0)
		{
			// hit occured in the frame
			return 1;
		}
		else if (hit_time > 200)
		{
			pair->next_check_time = timestamp(hit_time - 200);
			return 0;
			// set next check time to time - 200
		}
		else
		{
			// set next check time to next frame
			pair->next_check_time = 1;
			return 0;
		}
	}
	else
	{
		if (limit_time > time_to_max_error)
		{
			// no hit, but beyond error tolerance
			if (1000 * time_to_max_error > 200)
			{
				pair->next_check_time = timestamp((int)(1000 * time_to_max_error) - 200);
			}
			else
			{
				pair->next_check_time = 1;
			}
			return 0;
		}
		else
		{
			// no hit and within error tolerance
			return 1;
		}
	}
}
