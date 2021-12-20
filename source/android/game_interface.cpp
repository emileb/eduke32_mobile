

#include "game_interface.h"

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "SDL.h"
#include "SDL_keycode.h"

#include "SDL_beloko_extra.h"

#include "player.h"

#include "keyboard.h"
#include "control.h"
#include "_control.h"
#include "function.h"
#include "anim.h"


#include "gameTypes.h" // From Clibs_OpenTouch repo

extern "C"
{
	extern int SDL_SendKeyboardKey(Uint8 state, SDL_Scancode scancode);
	extern int gameType;
#include "SmartToggle.h"
}

//Move left/right fwd/back
static float forwardmove_android = 0;
static float sidemove_android = 0;

//Look up and down
static float look_pitch_mouse = 0;
static float look_pitch_joy = 0;

//left right
static float look_yaw_mouse = 0;
static float look_yaw_joy = 0;

// Fill in to set the sub folder for the game in the user_files folder
char userFilesSubFolder[256];

int PortableKeyEvent(int state, int code, int unicode)
{
	LOGI("PortableKeyEvent %d %d %d\n", state, code, unicode);

	if(state)
		SDL_SendKeyboardKey(SDL_PRESSED, (SDL_Scancode)code);
	else
		SDL_SendKeyboardKey(SDL_RELEASED, (SDL_Scancode) code);

	return 0;

}

void PortableBackButton()
{
	PortableKeyEvent(1, SDL_SCANCODE_ESCAPE, 0);
	usleep(1000*100);
	PortableKeyEvent(0, SDL_SCANCODE_ESCAPE, 0);
}

#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_MOVE 2
#define ACTION_MOVE_REL 3
#define ACTION_HOVER_MOVE 7
#define ACTION_SCROLL 8
#define BUTTON_PRIMARY 1
#define BUTTON_SECONDARY 2
#define BUTTON_TERTIARY 4
#define BUTTON_BACK 8
#define BUTTON_FORWARD 16


void PortableMouse(float dx, float dy)
{
	static float mx = 0;
	static float my = 0;
	//LOGI("%f %f",dx,dy);
	//mx += -dx * screen->GetWidth();
	//my +=  -dy * screen->GetHeight();

	if((fabs(mx) > 1) || (fabs(my) > 1))
	{
		SDL_InjectMouse(0, ACTION_MOVE, mx, my, SDL_TRUE);
	}

	if(fabs(mx) > 1)
		mx = 0;

	if(fabs(my) > 1)
		my = 0;
}

void PortableMouseButton(int state, int button, float dx, float dy)
{
	if(state)
		SDL_InjectMouse(BUTTON_PRIMARY, ACTION_DOWN, 0, 0, SDL_TRUE);
	else
		SDL_InjectMouse(0, ACTION_UP, 0, 0, SDL_TRUE);
}

static uint64_t functionSticky = 0;
static uint64_t functionHeld = 0;

void changeActionState(int state, int action)
{
	if (state)
	{
		functionSticky  |= ((uint64_t)1<<((uint64_t)(action)));
		functionHeld    |= ((uint64_t)1<<((uint64_t)(action)));
		return;
	}

	functionHeld  &= ~((uint64_t) 1<<((uint64_t) (action)));
}

bool buttonDown(int action)
{
	return false; // TODO
}

void PortableAction(int state, int action)
{
	LOGI("PortableAction %d   %d", state, action);

	if((action >= PORT_ACT_CUSTOM_0) && (action <= PORT_ACT_CUSTOM_25))
	{
		if(action <= PORT_ACT_CUSTOM_9)
			PortableKeyEvent(state, SDL_SCANCODE_KP_1 + action - PORT_ACT_CUSTOM_0, 0);
		else if(action <= PORT_ACT_CUSTOM_25)
			PortableKeyEvent(state, SDL_SCANCODE_A + action - PORT_ACT_CUSTOM_10, 0);
	}
	else if((PortableGetScreenMode() == TS_MENU) || (PortableGetScreenMode() == TS_BLANK))
	{
		if(action >= PORT_ACT_MENU_UP && action <= PORT_ACT_MENU_BACK)
		{

			int sdl_code [] = { SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT,
			                    SDL_SCANCODE_RIGHT, SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE
			                  };
			PortableKeyEvent(state, sdl_code[action - PORT_ACT_MENU_UP], 0);
			return;
		}
		else if(action == PORT_ACT_USE)   // This is sent from the blank screen
		{
//			changeActionState(state, Button_Use);
		}
	}
	else
	{
		switch(action)
		{
		case PORT_ACT_LEFT:
			changeActionState(state, gamefunc_Turn_Left);
			break;

		case PORT_ACT_RIGHT:
			changeActionState(state, gamefunc_Turn_Right);
			break;

		case PORT_ACT_FWD:
			changeActionState(state, gamefunc_Move_Forward);
			break;

		case PORT_ACT_BACK:
			changeActionState(state, gamefunc_Move_Backward);
			break;

		case PORT_ACT_MOVE_LEFT:
			changeActionState(state, gamefunc_Strafe_Left);
			break;

		case PORT_ACT_MOVE_RIGHT:
			changeActionState(state, gamefunc_Strafe_Right);
			break;

		case PORT_ACT_FLY_UP:
			//changeActionState(state, Button_MoveUp);
			break;

		case PORT_ACT_FLY_DOWN:
			//changeActionState(state, Button_MoveDown);
			break;

		case PORT_ACT_USE:
			changeActionState(state, gamefunc_Open);
			break;

		case PORT_ACT_ATTACK:
			changeActionState(state, gamefunc_Fire);
			break;

		case PORT_ACT_ALT_ATTACK:
			changeActionState(state, gamefunc_Alt_Fire);
			break;

		case PORT_ACT_TOGGLE_ALT_ATTACK:
			if(state)
			{
				if(buttonDown(gamefunc_Alt_Fire))
					changeActionState(0, gamefunc_Alt_Fire);
				else
					changeActionState(1, gamefunc_Alt_Fire);
			}

			break;

		case PORT_ACT_JUMP:
			changeActionState(state, gamefunc_Jump);
			break;

		case PORT_ACT_DOWN:
			changeActionState(state, gamefunc_Crouch);
			break;

		case PORT_ACT_TOGGLE_CROUCH:
		{
			static SmartToggle_t smartToggle;
			int activate = SmartToggleAction(&smartToggle, state, buttonDown(gamefunc_Crouch));
			changeActionState(activate, gamefunc_Crouch);
		}
		break;

		case PORT_ACT_NEXT_WEP:
			changeActionState(state, gamefunc_Next_Weapon);
			break;

		case PORT_ACT_PREV_WEP:
			changeActionState(state, gamefunc_Previous_Weapon);
			break;

		case PORT_ACT_MAP:
			changeActionState(state, gamefunc_Map);
			break;

		case PORT_ACT_QUICKLOAD:
			if(state)
				PortableCommand("quickload");
			break;

		case PORT_ACT_QUICKSAVE:
			if(state)
				PortableCommand("quicksave");
			break;

		case PORT_ACT_WEAP0:
			changeActionState(state, gamefunc_Weapon_10);
			break;

		case PORT_ACT_WEAP1:
			changeActionState(state, gamefunc_Weapon_1);
			break;

		case PORT_ACT_WEAP2:
			changeActionState(state, gamefunc_Weapon_2);
			break;

		case PORT_ACT_WEAP3:
			changeActionState(state, gamefunc_Weapon_3);
			break;

		case PORT_ACT_WEAP4:
			changeActionState(state, gamefunc_Weapon_4);
			break;

		case PORT_ACT_WEAP5:
			changeActionState(state, gamefunc_Weapon_5);
			break;

		case PORT_ACT_WEAP6:
			changeActionState(state, gamefunc_Weapon_6);
			break;

		case PORT_ACT_WEAP7:
			changeActionState(state, gamefunc_Weapon_7);
			break;

		case PORT_ACT_WEAP8:
			changeActionState(state, gamefunc_Weapon_8);
			break;

		case PORT_ACT_WEAP9:
			changeActionState(state, gamefunc_Weapon_9);
			break;

		case PORT_ACT_CONSOLE:
			PortableKeyEvent(state, SDL_SCANCODE_GRAVE, state);
			break;

		case PORT_ACT_INVUSE:
			changeActionState(state, gamefunc_Inventory);
			break;

		case PORT_ACT_INVDROP:
			changeActionState(state, gamefunc_Inventory);
			break;

		case PORT_ACT_INVPREV:
			changeActionState(state, gamefunc_Inventory_Left);
			break;

		case PORT_ACT_INVNEXT:
			changeActionState(state, gamefunc_Inventory_Right);
			break;

		case PORT_ACT_WEAP_ALT:
			changeActionState(state, gamefunc_Alt_Weapon);
			break;
		}
	}
}


// =================== FORWARD and SIDE MOVMENT ==============


void PortableMoveFwd(float fwd)
{
	if(fwd > 1)
		fwd = 1;
	else if(fwd < -1)
		fwd = -1;

	forwardmove_android = fwd;
}

void PortableMoveSide(float strafe)
{
	if(strafe > 1)
		strafe = 1;
	else if(strafe < -1)
		strafe = -1;

	sidemove_android = strafe;
}

void PortableMove(float fwd, float strafe)
{
	PortableMoveFwd(fwd);
	PortableMoveSide(strafe);
}

//======================================================================


void PortableLookPitch(int mode, float pitch)
{
	switch(mode)
	{
	case LOOK_MODE_MOUSE:
		look_pitch_mouse += pitch;
		break;

	case LOOK_MODE_JOYSTICK:
		look_pitch_joy = pitch;
		break;
	}
}


void PortableLookYaw(int mode, float yaw)
{
	switch(mode)
	{
	case LOOK_MODE_MOUSE:
		look_yaw_mouse += yaw;
		break;

	case LOOK_MODE_JOYSTICK:
		look_yaw_joy = yaw;
		break;
	}
}

extern "C"
{
	int eduke32_android_main(int argc, char **argv);
}
// Start game, does not return!
void PortableInit(int argc, const char ** argv)
{
	if(gameType == RAZE_GAME_IONFURY)
		strcpy(userFilesSubFolder, "ionfury");
	else if(gameType == RAZE_GAME_EDUKE32)
		strcpy(userFilesSubFolder, "eduke32");
	else
		strcpy(userFilesSubFolder, "unknown");

	eduke32_android_main(argc, (char **)argv);
}

bool            g_bindingbutton = false;
extern playerdata_t     *const g_player;
extern int inExtraScreens; //In screens.c
extern int myconnectindex;
touchscreemode_t PortableGetScreenMode()
{
	if(g_bindingbutton) {
		g_bindingbutton = false;
		return TS_CUSTOM;
	}
	else if (g_animPtr || inExtraScreens)
		return TS_BLANK;
	else if(g_player[myconnectindex].ps->gm & MODE_MENU)
		return TS_MENU;
	else if ((g_player[myconnectindex].ps->gm & MODE_GAME)) {
		if (g_player[myconnectindex].ps->dead_flag)
			return TS_GAME;
		else
			return TS_GAME;
	}
	else
		return TS_BLANK;
}


int PortableShowKeyboard(void)
{
	return 0;
}

const char *cmd_to_run = NULL;
void PortableCommand(const char * cmd)
{
	static char cmdBuffer[256];
	snprintf(cmdBuffer, 256, "%s", cmd);
	cmd_to_run = cmdBuffer;
}

static float am_zoom = 0;
static float am_pan_x = 0;
static float am_pan_y = 0;

void PortableAutomapControl(float zoom, float x, float y)
{
	am_zoom += zoom * 5;
	am_pan_x += x * 400;
	am_pan_y += y * 400;
	//LOGI("am_pan_x = %f",am_pan_x);
}


void Mobile_AM_controls(double *zoom, double *pan_x, double *pan_y)
{

}

//extern void G_AddViewAngle (int yaw);
//extern void G_AddViewPitch (int look);
//void AddCommandString (char *cmd, int keynum=0);

extern "C" int blockGamepad(void);

#define ANDROIDMOVEFACTOR           64000
#define ANDROIDLOOKFACTOR          1600000

#define ANDROIDPITCHFACTORJOYSTICK          2000
#define ANDROIDYAWFACTORJOYSTICK            4000

void Mobile_IN_Move(ControlInfo *input)
{
	int blockMove = blockGamepad() & ANALOGUE_AXIS_FWD;
	int blockLook = blockGamepad() & ANALOGUE_AXIS_PITCH;

	if(!blockMove)
	{
		input->dz -= forwardmove_android * ANDROIDMOVEFACTOR;
		input->dx += sidemove_android * ANDROIDMOVEFACTOR;
	}

	if(!blockLook)
	{
		// Add pitch
		input->mousey += -look_pitch_mouse * 50000;
		look_pitch_mouse = 0;
		input->dpitch += look_pitch_joy * 30000;

		// Add yaw
		input->mousex += -look_yaw_mouse * 100000;
		look_yaw_mouse = 0;
		input->dyaw += -look_yaw_joy * 20000;
	}

	if(cmd_to_run)
	{
		OSD_Dispatch(cmd_to_run);
		cmd_to_run = NULL;
	}

	for(int n = 0;n < 64; n++)
	{
		if((functionHeld | functionSticky) & ((uint64_t)1<<((uint64_t)(n))))
			CONTROL_ButtonFlags[n] = 1;
		else
			CONTROL_ButtonFlags[n] = 0;
	}
	//CONTROL_ButtonState = functionSticky | functionHeld;
	functionSticky = 0;

	//LOGI("CONTROL_ButtonState = %d", CONTROL_ButtonState);
}



