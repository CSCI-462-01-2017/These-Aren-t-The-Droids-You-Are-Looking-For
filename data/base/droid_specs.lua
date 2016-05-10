------------------------------------------------------------------------------
--
--  Copyright (c) 2002, 2003 Johannes Prix
--  Copyright (c) 2002 Reinhard Prix
--  Copyright (c) 2004-2007 Arthur Huillet
--  Copyright (c) 2007-2010 Stefan Huszics
--
--  This file is part of Freedroid
--
--  Freedroid is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  Freedroid is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with Freedroid; see the file COPYING. If not, write to the 
--  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
--  MA  02111-1307  USA
--
-- 
-- Feel free to make any modifications you like.  If you set up 
-- something cool, please send your file in to the FreedroidRPG project.
--
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- This file defines the behavior and rendering of the droids.
------------------------------------------------------------------------------

---------
-- Human/NPCs 8
-- Civilians 2
-- Spectral 10 (Can't see invisible tux. Sensor ID: spectral. Most common and default.)
-- IR 11 (Sees only heatsources: Can see invisible Tux. Sensor ID: infrared. Use on advanced droids.)
-- X-Ray 12 (Sees through the walls. Sensor ID: xray.)
-- Radar 16 (Ultrasonic + IR. Use only on the most advanced droids. Caution to don't affect the game balance.)-
-- Subsonic 6 (soundwaves/vibrations, see through walls, invisible if not moving) // TODO: To be implemented.
---------

sfx_sounds = {
	g0  = "effects/bot_sounds/First_Contact_Sound_0.ogg",
	g1  = "effects/bot_sounds/First_Contact_Sound_1.ogg",
	g2  = "effects/bot_sounds/First_Contact_Sound_2.ogg",
	g3  = "effects/bot_sounds/First_Contact_Sound_3.ogg",
	g4  = "effects/bot_sounds/First_Contact_Sound_4.ogg",
	g5  = "effects/bot_sounds/First_Contact_Sound_5.ogg",
	g6  = "effects/bot_sounds/First_Contact_Sound_6.ogg",
	g7  = "effects/bot_sounds/First_Contact_Sound_7.ogg",
	g8  = "effects/bot_sounds/First_Contact_Sound_8.ogg",
	g9  = "effects/bot_sounds/First_Contact_Sound_9.ogg",
	g10 = "effects/bot_sounds/First_Contact_Sound_10.ogg",
	g11 = "effects/bot_sounds/First_Contact_Sound_11.ogg",
	g12 = "effects/bot_sounds/First_Contact_Sound_12.ogg",
	g13 = "effects/bot_sounds/First_Contact_Sound_13.ogg",
	g14 = "effects/bot_sounds/First_Contact_Sound_14.ogg",
	g15 = "effects/bot_sounds/First_Contact_Sound_15.ogg",
	g16 = "effects/bot_sounds/First_Contact_Sound_16.ogg",
	g17 = "effects/bot_sounds/First_Contact_Sound_17.ogg",
	g18 = "effects/bot_sounds/First_Contact_Sound_18.ogg",

	a0  = "effects/bot_sounds/Start_Attack_Sound_0.ogg",
	a1  = "effects/bot_sounds/Start_Attack_Sound_1.ogg",
	a2  = "effects/bot_sounds/Start_Attack_Sound_2.ogg",
	a6  = "effects/bot_sounds/Start_Attack_Sound_0.ogg",
	a7  = "effects/bot_sounds/Start_Attack_Sound_1.ogg",
	a8  = "effects/bot_sounds/Start_Attack_Sound_2.ogg",
	a9  = "effects/bot_sounds/Start_Attack_Sound_9.ogg",
	a10 = "effects/bot_sounds/Start_Attack_Sound_10.ogg",
	a11 = "effects/bot_sounds/Start_Attack_Sound_11.ogg",
	a12 = "effects/bot_sounds/Start_Attack_Sound_12.ogg",
	a13 = "effects/bot_sounds/Start_Attack_Sound_13.ogg",
	a14 = "effects/bot_sounds/Start_Attack_Sound_14.ogg",
	a15 = "effects/bot_sounds/Start_Attack_Sound_15.ogg",
	a16 = "effects/bot_sounds/Start_Attack_Sound_16.ogg",
	a17 = "effects/bot_sounds/Start_Attack_Sound_17.ogg",
	a18 = "effects/bot_sounds/Start_Attack_Sound_18.ogg",

	d123 = "effects/bot_sounds/death_sound_123.ogg",
	d247 = "effects/bot_sounds/death_sound_247.ogg",
	d302 = "effects/bot_sounds/death_sound_302.ogg",

	voice_samples = {
		path = "effects/bot_sounds/voice_samples",
		first = 1,
		last = 62,
		probability = 20,
	},
}

droid_list {
{
	name = "123",
	desc = _"123 Acolyte",
	notes = _"A simple rubbish disposal robot. Common device in most factories to maintain a clean yard.",
	is_human = 0,
	abilities = {
		aggression_distance = 11,
	},
	equip = {
		weapon = "Droid 123 Weak Robotic Arm",
	},
	drop_draw = {
		class = 1,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/123",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g0,
		attack = sfx_sounds.a0,
		death = sfx_sounds.d123,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "139",
	desc = _"139 Templar",
	notes = _"Created by Dr. Masternak to clean up large heaps of rubbish. Its large scoop is used to collect rubbish. It is then crushed internally.",
	is_human = 0,
	abilities = {
		energy_max = 15,
		hit_draw = 60,
		xp_reward = 50,
	},
	equip = {
		weapon = "Droid 139 Plasma Trash Vaporiser",
	},
	drop_draw = {
		class = 1,
		plasma_transistors = 40,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/139",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g1,
		attack = sfx_sounds.a1,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "247",
	desc = _"247 Banshee",
	notes = _"A light duty servant robot. One of the first to use the anti-grav system.",
	is_human = 0,
	class = 2,
	abilities = {
		speed_max = 5,
		energy_max = 16,
		hit_draw = 60,
		recover_time = 0.80,
		xp_reward = 70,
	},
	equip = {
		weapon = "Droid 247 Robotic Arm",
	},
	drop_draw = {
		class = 2,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/247",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g9,
		attack = sfx_sounds.a9,
		death = sfx_sounds.d247,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "249",
	desc = _"249 Chicago",
	notes = _"Cheaper version of the anti-grav servant robot. This one is armed with rapid fire weaponry.",
	is_human = 0,
	class = 2,
	abilities = {
		speed_max = 3,
		energy_max = 20,
		hit_draw = 82,
		recover_time = 0.80,
		xp_reward = 90,
	},
	equip = {
		weapon = "Droid 249 Pulse Laser Welder",
	},
	drop_draw = {
		class = 2,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 5,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/249",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g10,
		attack = sfx_sounds.a10,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "296",
	desc = _"296 Sawmill",
	notes = _"Originally designed by Orchard and Marsden Enterprises, this robot was used as a servant droid in drinking establishments. A tray is mounted on the head for carrying drinks and a wide wheel base provided droid stability to prevent spillage. WARNING: You will lose something if you try to steal a drink!",
	is_human = 0,
	class = 2,
	abilities = {
		energy_max = 300,
		hit_draw = 60,
		recover_time = 0.80,
		xp_reward = 250,
	},
	equip = {
		weapon = "Droid 296 Plasmabeam Cutter",
	},
	drop_draw = {
		class = 2,
		plasma_transistors = 60,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/296",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g11,
		attack = sfx_sounds.a11,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "302",
	desc = _"302 Nemesis",
	notes = _"Common device for moving small packages. Clamp is mounted on the lower body.",
	is_human = 0,
	class = 3,
	abilities = {
		speed_max = 6,
		energy_max = 30,
		hit_draw = 60,
		time_eyeing = 0.25,
		recover_time = 0.60,
		xp_reward = 200,
	},
	equip = {
		weapon = "Droid 302 Overcharged Barcode Reader",
	},
	drop_draw = {
		class = 3,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 15,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/302",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g12,
		attack = sfx_sounds.a12,
		death = sfx_sounds.d302,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "329",
	desc = _"329 Sparkie",
	notes = _"Early type messenger robot. Large wheels impede motion on small craft.",
	is_human = 0,
	class = 3,
	abilities = {
		speed_max = 1,
		energy_max = 35,
		hit_draw = 60,
		time_eyeing = 2.75,
		recover_time = 0.60,
		xp_reward = 250,
	},
	equip = {
		weapon = "Droid 329 Dual Overcharged Barcode Reader",
	},
	drop_draw = {
		class = 3,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/329",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g13,
		attack = sfx_sounds.a13,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "420",
	desc = _"420 Surgeon",
	notes = _"A slow maintenance robot. Confined to drive maintenance during flight.",
	is_human = 0,
	class = 4,
	abilities = {
		energy_max = 40,
		hit_draw = 60,
		time_eyeing = 2.0,
		recover_time = 0.50,
		xp_reward = 300,
	},
	equip = {
		weapon = "Droid 420 Laser Scalpel",
	},
	drop_draw = {
		class = 4,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/420",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g14,
		attack = sfx_sounds.a14,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "476",
	desc = _"476 Coward",
	notes = _"Ship maintenance robot. Fitted with multiple arms to carry out repairs to the ship efficiently. All craft built after the Jupiter-incident are supplied with a team of these.",
	is_human = 0,
	class = 4,
	abilities = {
		speed_max = 3,
		energy_max = 15,
		hit_draw = 60,
		aggression_distance = 11,
		recover_time = 0.50,
		xp_reward = 400,
	},
	equip = {
		weapon = "Droid 476 Small Laser",
	},
	drop_draw = {
		class = 4,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/476",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g15,
		attack = sfx_sounds.a15,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "493",
	desc = _"493 Spinster",
	notes = _"Slave maintenance droid. Standard version will carry its own toolbox.",
	is_human = 0,
	class = 4,
	abilities = {
		speed_max = 3,
		energy_max = 80,
		healing_rate = 3,
		time_eyeing = 1.75,
		recover_time = 0.50,
		xp_reward = 600,
	},
	equip = {
		weapon = "Droid 493 Power Hammer",
	},
	drop_draw = {
		class = 4,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/493",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g16,
		attack = sfx_sounds.a16,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "516",
	desc = _"516 Ghoul",
	notes = _"Early crew droid. Able to carry out simple flight checks only. No longer supplied.",
	is_human = 0,
	class = 5,
	abilities = {
		speed_max = 3.5,
		energy_max = 70,
		healing_rate = 3,
		hit_draw = 65,
		recover_time = 0.40,
		xp_reward = 600,
	},
	equip = {
		weapon = "Droid 516 Robotic Fist",
	},
	drop_draw = {
		class = 5,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/516",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g17,
		attack = sfx_sounds.a17,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "543",
	desc = _"543 Forest Harvester",
	notes = _"This bot was designed for logging, but instead of harvesting trees, it's now harvesting people!!! Run away! This bot will kill you!",
	is_human = 0,
	class = 5,
	abilities = {
		energy_max = 300,
		hit_draw = 60,
		recover_time = 0.40,
		xp_reward = 250,
	},
	equip = {
		weapon = "Droid 543 Tree Harvester",
	},
	drop_draw = {
		class = 5,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/harvester",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g11,
		attack = sfx_sounds.a11,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "571",
	desc = _"571 Apollo",
	notes = _"A standard crew droid. Supplied with the ship.",
	is_human = 0,
	class = 5,
	abilities = {
		speed_max = 4,
		energy_max = 50,
		healing_rate = 3,
		hit_draw = 65,
		time_eyeing = 1.5,
		recover_time = 0.40,
		xp_reward = 320,
	},
	equip = {
		weapon = "Droid 571 Robotic Fist",
	},
	drop_draw = {
		class = 5,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/571",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g18,
		attack = sfx_sounds.a18,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "598",
	desc = _"598 Minister",
	notes = _"A highly sophisticated device. Able to control the Robo-Freighter on its own.",
	is_human = 0,
	class = 5,
	abilities = {
		speed_max = 3,
		energy_max = 120,
		healing_rate = 3,
		hit_draw = 60,
		time_eyeing = 1.5,
		recover_time = 0.40,
		xp_reward = 400,
	},
	equip = {
		weapon = "Droid 598 Robotic Fist",
	},
	drop_draw = {
		class = 5,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/598",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "603",
	desc = _"603 Lawnmower",
	notes = _"The 603 MowDown is a state-of-the-art lawnmower manufactured by the Nicholson company, and combines the functions of a regular lawnmower, a pet and a vicious guard dog in a single device.",
	is_human = 0,
	class = 4,
	abilities = {
		speed_max = 4,
		energy_max = 50,
		healing_rate = 20,
		hit_draw = 60,
		aggression_distance = 5,
		time_eyeing = 1.5,
		recover_time = 0.30,
		xp_reward = 250,
	},
	equip = {
		weapon = "Chainsaw",
	},
	drop_draw = {
		class = 5,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/lawnmower",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g15,
		attack = sfx_sounds.a15,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "614",
	desc = _"614 Security Bot",
	notes = _"A low security sentinel droid. Used to protect areas of the ship from intruders. A slow but sure device.",
	is_human = 0,
	class = 6,
	abilities = {
		energy_max = 130,
		healing_rate = 3,
		hit_draw = 65,
		aggression_distance = 10.6,
		time_eyeing = 1.25,
		recover_time = 0.30,
		xp_reward = 460,
	},
	equip = {
		weapon = "Exterminator",
	},
	drop_draw = {
		class = 6,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/614",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g5,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "615",
	desc = _"615 Firedevil",
	notes = _"Sophisticated sentinel droid. Only 2000 built by the Nicholson Company. These are now very rare.",
	is_human = 0,
	class = 6,
	abilities = {
		speed_max = 4,
		energy_max = 140,
		healing_rate = 3.5,
		hit_draw = 65,
		aggression_distance = 11,
		time_eyeing = 1.25,
		recover_time = 0.30,
		xp_reward = 500,
	},
	equip = {
		weapon = "Droid Advanced Twin Laser",
	},
	drop_draw = {
		class = 6,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/615",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "629",
	desc = _"629 Spitfire",
	notes = _"A low sentinel droid. Lasers are built into the turret. These are mounted on a small tank body. May be fitted with an auto-cannon on-the Grillen version.",
	is_human = 0,
	class = 6,
	abilities = {
		energy_max = 160,
		healing_rate = 3,
		aggression_distance = 10.6,
		recover_time = 0.30,
		xp_reward = 600,
	},
	equip = {
		weapon = "Droid Advanced Twin Laser",
	},
	drop_draw = {
		class = 6,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/629",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "711",
	desc = _"711 Grillmeister",
	notes = _"A heavy duty battle droid. Disruptor is built into the head. One of the first in service with the Military.",
	is_human = 0,
	class = 7,
	abilities = {
		speed_max = 5,
		energy_max = 180,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16.12,
		recover_time = 0.20,
		xp_reward = 650,
	},
	equip = {
		weapon = "Droid 7xx Tux Seeking Missiles",
		sensor = "xray",
	},
	drop_draw = {
		class = 7,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/711",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "742",
	desc = _"742 Zeus",
	notes = _"This version is the one mainly used by the military.",
	is_human = 0,
	class = 7,
	abilities = {
		speed_max = 4,
		energy_max = 210,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16,
		time_eyeing = 0.75,
		recover_time = 0.20,
		xp_reward = 700,
	},
	equip = {
		weapon = "Droid 7xx Tux Seeking Missiles",
	},
	drop_draw = {
		class = 7,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/742",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "751",
	desc = _"751 Soviet",
	notes = _"A very heavy duty battle droid. Only a few have so far entered service. These are the most powerful battle units ever built.",
	is_human = 0,
	class = 7,
	abilities = {
		speed_max = 5,
		energy_max = 150,
		healing_rate = 4,
		hit_draw = 65,
		time_eyeing = 0.75,
		recover_time = 0.20,
		xp_reward = 800,
	},
	equip = {
		weapon = "Droid 7xx Tux Seeking Missiles",
	},
	drop_draw = {
		class = 7,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/751",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "821",
	desc = _"821 Ufo",
	notes = _"A very reliable anti-grav unit is fitted into this droid. It will patrol the ship and eliminate intruders as soon as detected by powerful sensors.",
	is_human = 0,
	class = 8,
	abilities = {
		speed_max = 6,
		energy_max = 200,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16.11,
		time_eyeing = 0.5,
		recover_time = 0.30,
		xp_reward = 1000,
	},
	equip = {
		weapon = "Droid Advanced Twin Laser",
	},
	drop_draw = {
		class = 8,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/821",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "834",
	desc = _"834 Wisp",
	notes = _"Early type anti-grav security droid. Fitted with an over-driven anti-grav unit. This droid is very fast but is not reliable.",
	is_human = 0,
	class = 8,
	abilities = {
		speed_max = 7,
		energy_max = 220,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16,
		time_eyeing = 0.5,
		recover_time = 0.30,
		xp_reward = 1200,
	},
	equip = {
		weapon = "Droid Advanced Twin Laser",
	},
	drop_draw = {
		class = 8,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/834",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "883",
	desc = _"883 Dalex",
	notes = _"This droid was designed from archive data. For some unknown reason it instils great fear in Human adversaries.",
	is_human = 0,
	class = 8,
	abilities = {
		energy_max = 120,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16,
		time_eyeing = 0.25,
		recover_time = 0.30,
		xp_reward = 1500,
	},
	equip = {
		weapon = "Droid 883 Exterminator",
	},
	drop_draw = {
		class = 8,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/883",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g2,
		attack = sfx_sounds.a2,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "999",
	desc = _"999 Cerebrum",
	notes = _"Experimental command cyborg. Fitted with a new type of brain. Mounted on a security droid anti-grav unit for convenience.",
	is_human = 0,
	class = 9,
	abilities = {
		speed_max = 6,
		energy_max = 300,
		healing_rate = 15,
		hit_draw = 70,
		aggression_distance = 16.116,
		time_eyeing = 0.25,
		recover_time = 0.05,
		xp_reward = 2000,
	},
	equip = {
		weapon = "Droid Advanced Twin Laser",
		sensor = "infrared",
	},
	drop_draw = {
		class = 9,
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/999",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "PRO",
	desc = _"Older Man",
	notes = _"This is a typical professor/sage character of some older age.  They are not armed as powerfully as e.g. the red guards.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/professor",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "DOC",
	desc = _"Older Man",
	notes = _"Some people say that every doctor is addicted to drugs. You estimate that right now this one is high on at least four different narcotics. Doc Moore seems very happy indeed...",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
		weapon = "Laser Scalpel",
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/professor",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "GUA",
	desc = _"Red Guard",
	notes = _"This is a typical red (male) guard/fighter character.  They are equipped with powerful PowerExterminators.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		aggression_distance = 8,
		time_eyeing = 0.5,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
		weapon = "Exterminator",
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/red_guard",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g6,
		attack = sfx_sounds.a6,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "DIX",
	desc = _"Red Guard",
	notes = _"This is Dixon, unlike a typical red (male) guard/fighter character, he has a knife weapon, and lowered aggression.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		aggression_distance = 5,
		time_eyeing = 0.5,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
		weapon = "Nobody's edge",
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/red_guard",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g6,
		attack = sfx_sounds.a6,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "MUM",
	desc = _"Matron",
	notes = _"This is the hot_mama shopkeeper character model.",
	class = 18,
	abilities = {
		speed_max = 3,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/hot_mama",
		animation = {
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "FSC",
	desc = _"Female Scientist",
	notes = _"This is the female scientist character model.  They are not armed as powerfully as e.g. the red guards.",
	class = 18,
	abilities = {
		speed_max = 3,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 3,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/female_scientist",
		animation = {
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "WOR",
	desc = _"Mine Worker",
	notes = _"This is the brown worker model.  They are not armed as powerfully as e.g. the red guards.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		aggression_distance = 2,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/default_male",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g8,
		attack = sfx_sounds.a8,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "GUB",
	desc = _"Blue Brother",
	notes = _"This is a useless comment. Thanks for your attention.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		time_eyeing = 0.5,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
		weapon = "Exterminator",
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/blue_guard",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g7,
		attack = sfx_sounds.a7,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "COO",
	desc = _"Cook",
	notes = _"This is the cook.",
	class = 18,
	abilities = {
		speed_max = 3,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/cook",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g6,
		attack = sfx_sounds.a6,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "KEV",
	desc = _"Balding Man",
	notes = _"This is a typical professor/sage character of some older age. They are not armed as powerfully as e.g. the red guards.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 2,
		time_eyeing = 1.5,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/kevin",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "JAS",
	desc = _"Pretty Woman",
	notes = _"This is a typical sex bomb.  This one is even armed.",
	is_human = 0,
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 30,
		aggression_distance = 2,
		time_eyeing = 3,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/jasmine",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "TAN",
	desc = _"Woman",
	notes = _"This is Tania. She is human.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 30,
		aggression_distance = 2,
		time_eyeing = 3,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/jasmine",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "BAR",
	desc = _"Bartender",
	notes = _"This is the bartender. Care for a drink?",
	class = 18,
	abilities = {
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 70,
		aggression_distance = 2,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/bartender",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g6,
		attack = sfx_sounds.a6,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "FEM",
	desc = _"Female Hunter",
	notes = _"This hunter has fought with many robots before, and will not be scared by a mere linarian.",
	class = 18,
	abilities = {
		speed_max = 6,
		energy_max = 100,
		healing_rate = 15,
		hit_draw = 60,
		aggression_distance = 8,
		time_eyeing = 1.5,
		recover_time = 0.30,
		xp_reward = 200,
	},
	equip = {
	},
	drop_draw = {
		class = 9,
	},
	gfx = {
		prefix = "droids/female_hunter",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "TRM",
	desc = _"ERROR: THIS IS A TERMINAL",
	notes = _"This is a terminal.",
	is_human = 0,
	class = 100,
	abilities = {
		speed_max = 0,
		energy_max = 1,
		healing_rate = 1,
		hit_draw = 0,
		aggression_distance = 0,
		time_eyeing = 0,
		recover_time = 0,
		xp_reward = 0,
	},
	equip = {
	},
	drop_draw = {
		class = 0,
	},
	gfx = {
		prefix = "droids/terminal",
		animation = {
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "GUN",
	desc = _"Autogun",
	notes = _"Carefully designed by Future Ballistics(R), this autogun will shred you into pieces if you try pass across them.",
	is_human = 0,
	class = 7,
	abilities = {
		speed_max = 0,
		energy_max = 150,
		healing_rate = 4,
		hit_draw = 65,
		aggression_distance = 16,
		time_eyeing = 2,
		recover_time = 0.30,
		xp_reward = 360,
	},
	equip = {
		weapon = "Autogun Laser Pistol",
		sensor = "infrared",
	},
	drop_draw = {
		plasma_transistors = 5,
		superconductors = 5,
		antimatter_converters = 2,
		entropy_inverters = 10,
		tachyon_condensators = 10,
	},
	gfx = {
		prefix = "droids/autogun",
		animation = {
			portrait_rotations = 32,
		},
	},
	sound = {
		greeting = sfx_sounds.g1,
		attack = sfx_sounds.a1,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "ARH",
	desc = _"Authority Figure",
	notes = _"It has long since been known that game development is the most important job in the world. Also, the coolest. We know that thanks to this guy.",
	class = 19,
	abilities = {
		speed_max = 5,
		energy_max = 9001,
		healing_rate = 100,
		hit_draw = 100,
		aggression_distance = 11,
		time_eyeing = 0.5,
		xp_reward = 1334,
	},
	equip = {
		weapon = "Sledgehammer",
	},
	drop_draw = {
		class = 9,
		plasma_transistors = 100,
		superconductors = 100,
		antimatter_converters = 100,
		entropy_inverters = 100,
		tachyon_condensators = 100,
	},
	gfx = {
		prefix = "droids/arthur",
		animation = {
		},
	},
	sound = {
		greeting = sfx_sounds.g4,
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
{
	name = "STM",
	desc = _"ERROR: THIS IS A TERMINAL",
	notes = _"This is a secure terminal.",
	is_human = 0,
	class = 100,
	abilities = {
		speed_max = 0,
		energy_max = 1,
		healing_rate = 1,
		hit_draw = 0,
		aggression_distance = 0,
		time_eyeing = 0,
		recover_time = 0,
		xp_reward = 0,
	},
	equip = {
	},
	drop_draw = {
		class = 0,
	},
	gfx = {
		prefix = "droids/terminal_secure",
		animation = {
		},
	},
	sound = {
		voice_samples = {
			path = sfx_sounds.voice_samples.path,
			first = sfx_sounds.voice_samples.first,
			last = sfx_sounds.voice_samples.last,
			probability = sfx_sounds.voice_samples.probability,
		},
	},
}
}
