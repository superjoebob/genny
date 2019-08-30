#ifndef __GENERICTRANSPORT_H
#define __GENERICTRANSPORT_H

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	unsigned long Msg;
	long Index;
	long Value;
	long Result;  // 1 if handled
} TTransportMsg, *PTransportMsg;

#pragma pack(pop)


// see FPD_Transport
// (button) means: 0 for release, 1 for switch (if release is not supported), 2 for hold (if release should be expected)
// (hold) means: 0 for release, 2 for hold
// (jog) means: value is an integer increment
// if FPT_Jog, FPT_StripJog, FPT_MarkerJumpJog, FPT_MarkerSelJog, FPT_Previous or FPT_Next don't answer, FPT_PreviousNext will be tried. So it's best to implement at least FPT_PreviousNext
// if FPT_PunchIn or FPT_PunchOut don't answer, FPT_Punch will be tried
// if FPT_UndoUp doesn't answer, FPT_UndoJog will be tried
// if FPT_AddAltMarker doesn't answer, FPT_AddMarker will be tried
// if FPT_Cut, FPT_Copy, FPT_Paste, FPT_Insert, FPT_Delete, FPT_NextWindow, FPT_Enter, FPT_Escape, FPT_Yes, FPT_No, FPT_Fx don't answer, standard keystrokes will be simulated
const int FPT_Jog               =0;     // (jog) generic jog (can be used to select stuff)
const int FPT_Jog2              =1;     // (jog) alternate generic jog (can be used to relocate stuff)
const int FPT_Strip             =2;     // touch-sensitive jog strip, value will be in -65536..65536 for leftmost..rightmost
const int FPT_StripJog          =3;     // (jog) touch-sensitive jog in jog mode
const int FPT_StripHold         =4;     // value will be 0 for release, 1,2 for 1,2 fingers centered mode, -1,-2 for 1,2 fingers jog mode (will then send FPT_StripJog)
const int FPT_Previous          =5;     // (button)
const int FPT_Next              =6;     // (button)
const int FPT_PreviousNext      =7;     // (jog) generic track selection
const int FPT_MoveJog           =8;     // (jog) used to relocate items

const int FPT_Play              =10;    // (button) play/pause
const int FPT_Stop              =11;    // (button)
const int FPT_Record            =12;    // (button)
const int FPT_Rewind            =13;    // (hold)
const int FPT_FastForward       =14;    // (hold)
const int FPT_Loop              =15;    // (button)
const int FPT_Mute              =16;    // (button)
const int FPT_Mode              =17;    // (button) generic or record mode

const int FPT_Undo              =20;    // (button) undo/redo last, or undo down in history
const int FPT_UndoUp            =21;    // (button) undo up in history (no need to implement if no undo history)
const int FPT_UndoJog           =22;    // (jog) undo in history (no need to implement if no undo history)

const int FPT_Punch             =30;    // (hold) live selection
const int FPT_PunchIn           =31;    // (button)
const int FPT_PunchOut          =32;    // (button)
const int FPT_AddMarker         =33;    // (button)
const int FPT_AddAltMarker      =34;    // (button) add alternate marker
const int FPT_MarkerJumpJog     =35;    // (jog) marker jump
const int FPT_MarkerSelJog      =36;    // (jog) marker selection

const int FPT_Up                =40;    // (button)
const int FPT_Down              =41;    // (button)
const int FPT_Left              =42;    // (button)
const int FPT_Right             =43;    // (button)
const int FPT_HZoomJog          =44;    // (jog)
const int FPT_VZoomJog          =45;    // (jog)
const int FPT_Snap              =48;    // (button) snap on/off
const int FPT_SnapMode          =49;    // (jog) snap mode

const int FPT_Cut               =50;    // (button)
const int FPT_Copy              =51;    // (button)
const int FPT_Paste             =52;    // (button)
const int FPT_Insert            =53;    // (button)
const int FPT_Delete            =54;    // (button)
const int FPT_NextWindow        =58;    // (button) TAB
const int FPT_WindowJog         =59;    // (jog) window selection

const int FPT_F1                =60;    // button
const int FPT_F2                =61;    // button
const int FPT_F3                =62;    // button
const int FPT_F4                =63;    // button
const int FPT_F5                =64;    // button
const int FPT_F6                =65;    // button
const int FPT_F7                =66;    // button
const int FPT_F8                =67;    // button
const int FPT_F9                =68;    // button
const int FPT_F10               =69;    // button

const int FPT_Enter             =80;    // (button) enter/accept
const int FPT_Escape            =81;    // (button) escape/cancel
const int FPT_Yes               =82;    // (button) yes
const int FPT_No                =83;    // (button) no

const int FPT_Menu              =90;    // (button) generic menu
const int FPT_ItemMenu          =91;    // (button) item edit/tool/contextual menu
const int FPT_Save              =92;    // (button)
const int FPT_SaveNew           =93;    // (button) save as new version

#endif