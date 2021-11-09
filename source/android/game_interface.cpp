

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

extern "C"
{
	extern int SDL_SendKeyboardKey(Uint8 state, SDL_Scancode scancode);
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
//			buttonChange(state, Button_Use);
		}
	}
	else
	{
		switch(action)
		{
			/*
		case PORT_ACT_LEFT:
			buttonChange(state, gamefunc_Turn_Left);
			break;

		case PORT_ACT_RIGHT:
			buttonChange(state, gamefunc_Turn_Right);
			break;

		case PORT_ACT_FWD:
			buttonChange(state, gamefunc_Move_Forward);
			break;

		case PORT_ACT_BACK:
			buttonChange(state, gamefunc_Move_Backward);
			break;

		case PORT_ACT_MOVE_LEFT:
			buttonChange(state, gamefunc_Strafe_Left);
			break;

		case PORT_ACT_MOVE_RIGHT:
			buttonChange(state, gamefunc_Strafe_Right);
			break;

		case PORT_ACT_FLY_UP:
			//buttonChange(state, Button_MoveUp);
			break;

		case PORT_ACT_FLY_DOWN:
			//buttonChange(state, Button_MoveDown);
			break;

		case PORT_ACT_USE:
			buttonChange(state, gamefunc_Open);
			break;

		case PORT_ACT_ATTACK:
			buttonChange(state, gamefunc_Fire);
			break;

		case PORT_ACT_ALT_ATTACK:
			buttonChange(state, gamefunc_Alt_Fire);
			break;

		case PORT_ACT_TOGGLE_ALT_ATTACK:
			if(state)
			{
				if(buttonDown(gamefunc_Alt_Fire))
					buttonChange(0, gamefunc_Alt_Fire);
				else
					buttonChange(1, gamefunc_Alt_Fire);
			}

			break;

		case PORT_ACT_JUMP:
			buttonChange(state, gamefunc_Jump);
			break;

		case PORT_ACT_DOWN:
			buttonChange(state, gamefunc_Crouch);
			break;

		case PORT_ACT_TOGGLE_CROUCH:
		{
			static SmartToggle_t smartToggle;
			int activate = SmartToggleAction(&smartToggle, state, buttonDown(gamefunc_Crouch));
			buttonChange(activate, gamefunc_Crouch);
		}
		break;

		case PORT_ACT_NEXT_WEP:
			if(state)
				PortableCommand("weapnext");
			break;

		case PORT_ACT_PREV_WEP:
			if(state)
				PortableCommand("weapprev");
			break;

		case PORT_ACT_MAP:
			if(state)
				PortableCommand("togglemap");
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
			if(state)
				PortableCommand("slot 10");
			break;

		case PORT_ACT_WEAP1:
			if(state)
				PortableCommand("slot 1");
			break;

		case PORT_ACT_WEAP2:
			if(state)
				PortableCommand("slot 2");
			break;

		case PORT_ACT_WEAP3:
			if(state)
				PortableCommand("slot 3");
			break;

		case PORT_ACT_WEAP4:
			if(state)
				PortableCommand("slot 4");
			break;

		case PORT_ACT_WEAP5:
			if(state)
				PortableCommand("slot 5");
			break;

		case PORT_ACT_WEAP6:
			if(state)
				PortableCommand("slot 6");
			break;

		case PORT_ACT_WEAP7:
			if(state)
				PortableCommand("slot 7");
			break;

		case PORT_ACT_WEAP8:
			if(state)
				PortableCommand("slot 8");
			break;

		case PORT_ACT_WEAP9:
			if(state)
				PortableCommand("slot 9");
			break;

		case PORT_ACT_CONSOLE:
			if(state)
				PortableCommand("toggleconsole");
			break;

		case PORT_ACT_INVUSE:
			if(state)
				PortableCommand("invuse");
			break;

		case PORT_ACT_INVDROP:
			if(state)
				PortableCommand("invdrop");
			break;

		case PORT_ACT_INVPREV:
			if(state)
				PortableCommand("invprev");
			break;

		case PORT_ACT_INVNEXT:
			if(state)
				PortableCommand("invnext");
			break;

		case PORT_ACT_WEAP_ALT:
			if(state)
				PortableCommand("weapalt");
			break;
			 */
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
	eduke32_android_main(argc, (char **)argv);
}

bool            g_bindingbutton = false;
extern playerdata_t     *const g_player;
touchscreemode_t PortableGetScreenMode()
{
	if(g_player[0].ps->gm & MODE_MENU)
		return TS_MENU;
	else
		return TS_GAME;
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
/*
void Mobile_IN_Move(ControlInfo &input)
{
	int blockMove = blockGamepad() & ANALOGUE_AXIS_FWD;
	int blockLook = blockGamepad() & ANALOGUE_AXIS_PITCH;

	if(!blockMove)
	{
		input.dz += forwardmove_android ;
		input.dx -= sidemove_android;
	}

	if(!blockLook)
	{
		// Add pitch
		input.dpitch += -look_pitch_mouse * 35;
		look_pitch_mouse = 0;
		input.dpitch += look_pitch_joy * 1;

		// Add yaw
		input.dyaw += -look_yaw_mouse * 100;
		look_yaw_mouse = 0;
		input.dyaw += -look_yaw_joy * 1;
	}

	if(cmd_to_run)
	{
		AddCommandString((char*)cmd_to_run);
		cmd_to_run = NULL;
	}
}
*/


