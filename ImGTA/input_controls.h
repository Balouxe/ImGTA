/*
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

enum CONTROL_TYPE
{
	PLAYER_CONTROL = 0,
	FRONTEND_CONTROL = 2
};

enum CONTROL {
	INPUT_NEXT_CAMERA = 0,
	INPUT_SKIP_CUTSCENE = 18,
	INPUT_CHARACTER_WHEEL = 19,
	INPUT_SPRINT = 21,
	INPUT_JUMP = 22,
	INPUT_ENTER = 23,
	INPUT_ATTACK = 24,
	INPUT_AIM = 25,
	INPUT_LOOK_BEHIND = 26,
	INPUT_PHONE = 27,
	INPUT_SPECIAL_ABILITY = 28,
	INPUT_MOVE_UP_ONLY = 32,
	INPUT_MOVE_DOWN_ONLY = 32,
	INPUT_MOVE_LEFT_ONLY = 32,
	INPUT_MOVE_RIGHT_ONLY = 32,
	INPUT_SELECT_WEAPON = 37,
	INPUT_COVER = 44,
	INPUT_RELOAD = 45,
	INPUT_DETONATE = 47,
	INPUT_VEH_EXIT = 75,
	INPUT_MELEE_ATTACK_LIGHT = 140,
	INPUT_MELEE_ATTACK_HEAVY = 141,
	INPUT_MELEE_ATTACK_ALTERNATE = 142,
	INPUT_SELECT_CHARACTER_MICHAEL = 166,
	INPUT_SELECT_CHARACTER_FRANKLIN = 167,
	INPUT_SELECT_CHARACTER_TREVOR = 168,
	INPUT_SELECT_CHARACTER_MULTIPLAYER = 169,
	INPUT_SPECIAL_ABILITY_PC = 171,
	INPUT_CELLPHONE_UP = 172,
	INPUT_CELLPHONE_DOWN = 173
};