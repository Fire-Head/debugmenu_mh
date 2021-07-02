#include "debugmenu.h"

// MH

void **&RwEngineInst = *(void***)0x82279C;

unsigned int &CGameTime::ms_currGameTime = *(unsigned int*)0x756270;

CKeyState *CInputManager::m_keysPressed = (CKeyState*)0x725698;
CMouseState &CInputManager::m_mouseState = *(CMouseState*)0x72568C;

CMouseControllerState &CPad::NewMouseControllerState = *(CMouseControllerState*)0x7E9100;
CMouseControllerState &CPad::OldMouseControllerState = *(CMouseControllerState*)0x7E9110;

CKeyboardState &CPad::NewKeyState = *(CKeyboardState*)0x7E9130;
CKeyboardState &CPad::OldKeyState = *(CKeyboardState*)0x7E93A0;

WRAPPER RwRaster *RwRasterCreate(RwInt32, RwInt32, RwInt32, RwInt32) { EAXJMP(0x63CED0); }
WRAPPER RwRaster *RwRasterSetFromImage(RwRaster*, RwImage*) { EAXJMP(0x62F1A0); }
WRAPPER RwImage *RwImageFindRasterFormat(RwImage*, RwInt32, RwInt32*, RwInt32*, RwInt32*, RwInt32*) { EAXJMP(0x62F1D0); }
WRAPPER RwBool RwImageDestroy(RwImage*) { EAXJMP(0x62DD70); }
WRAPPER RwImage *RwImageCreate(RwInt32, RwInt32, RwInt32) { EAXJMP(0x62DD10); }
WRAPPER RwImage *RwImageAllocatePixels(RwImage *) { EAXJMP(0x62DDD0); }


bool CControllerConfigManager::GetIsKeyboardKeyDown(RsKeyCodes keycode)
{
	if (keycode < 255)
	{
		if (CPad::GetPad(PAD1)->GetChar(keycode))
			return true;
	}

	for (int32 i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (CPad::GetPad(PAD1)->GetF(i))
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (CPad::GetPad(PAD1)->GetEscape())
			return true;
		break;
	case rsINS:
		if (CPad::GetPad(PAD1)->GetInsert())
			return true;
		break;
	case rsDEL:
		if (CPad::GetPad(PAD1)->GetDelete())
			return true;
		break;
	case rsHOME:
		if (CPad::GetPad(PAD1)->GetHome())
			return true;
		break;
	case rsEND:
		if (CPad::GetPad(PAD1)->GetEnd())
			return true;
		break;
	case rsPGUP:
		if (CPad::GetPad(PAD1)->GetPageUp())
			return true;
		break;
	case rsPGDN:
		if (CPad::GetPad(PAD1)->GetPageDown())
			return true;
		break;
	case rsUP:
		if (CPad::GetPad(PAD1)->GetUp())
			return true;
		break;
	case rsDOWN:
		if (CPad::GetPad(PAD1)->GetDown())
			return true;
		break;
	case rsLEFT:
		if (CPad::GetPad(PAD1)->GetLeft())
			return true;
		break;
	case rsRIGHT:
		if (CPad::GetPad(PAD1)->GetRight())
			return true;
		break;
	case rsSCROLL:
		if (CPad::GetPad(PAD1)->GetScrollLock())
			return true;
		break;
	case rsPAUSE:
		if (CPad::GetPad(PAD1)->GetPause())
			return true;
		break;
	case rsNUMLOCK:
		if (CPad::GetPad(PAD1)->GetNumLock())
			return true;
		break;
	case rsDIVIDE:
		if (CPad::GetPad(PAD1)->GetDivide())
			return true;
		break;
	case rsTIMES:
		if (CPad::GetPad(PAD1)->GetTimes())
			return true;
		break;
	case rsMINUS:
		if (CPad::GetPad(PAD1)->GetMinus())
			return true;
		break;
	case rsPLUS:
		if (CPad::GetPad(PAD1)->GetPlus())
			return true;
		break;
	case rsPADENTER:
		if (CPad::GetPad(PAD1)->GetPadEnter())
			return true;
		break;
	case rsPADDEL:
		if (CPad::GetPad(PAD1)->GetPadDel())
			return true;
		break;
	case rsPADEND:
		if (CPad::GetPad(PAD1)->GetPad1())
			return true;
		break;
	case rsPADDOWN:
		if (CPad::GetPad(PAD1)->GetPad2())
			return true;
		break;
	case rsPADPGDN:
		if (CPad::GetPad(PAD1)->GetPad3())
			return true;
		break;
	case rsPADLEFT:
		if (CPad::GetPad(PAD1)->GetPad4())
			return true;
		break;
	case rsPAD5:
		if (CPad::GetPad(PAD1)->GetPad5())
			return true;
		break;
	case rsPADRIGHT:
		if (CPad::GetPad(PAD1)->GetPad6())
			return true;
		break;
	case rsPADHOME:
		if (CPad::GetPad(PAD1)->GetPad7())
			return true;
		break;
	case rsPADUP:
		if (CPad::GetPad(PAD1)->GetPad8())
			return true;
		break;
	case rsPADPGUP:
		if (CPad::GetPad(PAD1)->GetPad9())
			return true;
		break;
	case rsPADINS:
		if (CPad::GetPad(PAD1)->GetPad0())
			return true;
		break;
	case rsBACKSP:
		if (CPad::GetPad(PAD1)->GetBackspace())
			return true;
		break;
	case rsTAB:
		if (CPad::GetPad(PAD1)->GetTab())
			return true;
		break;
	case rsCAPSLK:
		if (CPad::GetPad(PAD1)->GetCapsLock())
			return true;
		break;
	case rsENTER:
		if (CPad::GetPad(PAD1)->GetEnter())
			return true;
		break;
	case rsLSHIFT:
		if (CPad::GetPad(PAD1)->GetLeftShift())
			return true;
		break;
	case rsSHIFT:
		if (CPad::GetPad(PAD1)->GetShift())
			return true;
		break;
	case rsRSHIFT:
		if (CPad::GetPad(PAD1)->GetRightShift())
			return true;
		break;
	case rsLCTRL:
		if (CPad::GetPad(PAD1)->GetLeftCtrl())
			return true;
		break;
	case rsRCTRL:
		if (CPad::GetPad(PAD1)->GetRightCtrl())
			return true;
		break;
	case rsLALT:
		if (CPad::GetPad(PAD1)->GetLeftAlt())
			return true;
		break;
	case rsRALT:
		if (CPad::GetPad(PAD1)->GetRightAlt())
			return true;
		break;
	case rsLWIN:
		if (CPad::GetPad(PAD1)->GetLeftWin())
			return true;
		break;
	case rsRWIN:
		if (CPad::GetPad(PAD1)->GetRightWin())
			return true;
		break;
	case rsAPPS:
		if (CPad::GetPad(PAD1)->GetApps())
			return true;
		break;
	default: break;
	}

	return false;
}

bool CControllerConfigManager::GetIsKeyboardKeyJustDown(RsKeyCodes keycode)
{
	if (keycode < 255)
	{
		if (CPad::GetPad(PAD1)->GetCharJustDown(keycode))
			return true;
	}

	for (int32 i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (CPad::GetPad(PAD1)->GetFJustDown(i))
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (CPad::GetPad(PAD1)->GetEscapeJustDown())
			return true;
		break;
	case rsINS:
		if (CPad::GetPad(PAD1)->GetInsertJustDown())
			return true;
		break;
	case rsDEL:
		if (CPad::GetPad(PAD1)->GetDeleteJustDown())
			return true;
		break;
	case rsHOME:
		if (CPad::GetPad(PAD1)->GetHomeJustDown())
			return true;
		break;
	case rsEND:
		if (CPad::GetPad(PAD1)->GetEndJustDown())
			return true;
		break;
	case rsPGUP:
		if (CPad::GetPad(PAD1)->GetPageUpJustDown())
			return true;
		break;
	case rsPGDN:
		if (CPad::GetPad(PAD1)->GetPageDownJustDown())
			return true;
		break;
	case rsUP:
		if (CPad::GetPad(PAD1)->GetUpJustDown())
			return true;
		break;
	case rsDOWN:
		if (CPad::GetPad(PAD1)->GetDownJustDown())
			return true;
		break;
	case rsLEFT:
		if (CPad::GetPad(PAD1)->GetLeftJustDown())
			return true;
		break;
	case rsRIGHT:
		if (CPad::GetPad(PAD1)->GetRightJustDown())
			return true;
		break;
	case rsSCROLL:
		if (CPad::GetPad(PAD1)->GetScrollLockJustDown())
			return true;
		break;
	case rsPAUSE:
		if (CPad::GetPad(PAD1)->GetPauseJustDown())
			return true;
		break;
	case rsNUMLOCK:
		if (CPad::GetPad(PAD1)->GetNumLockJustDown())
			return true;
		break;
	case rsDIVIDE:
		if (CPad::GetPad(PAD1)->GetDivideJustDown())
			return true;
		break;
	case rsTIMES:
		if (CPad::GetPad(PAD1)->GetTimesJustDown())
			return true;
		break;
	case rsMINUS:
		if (CPad::GetPad(PAD1)->GetMinusJustDown())
			return true;
		break;
	case rsPLUS:
		if (CPad::GetPad(PAD1)->GetPlusJustDown())
			return true;
		break;
	case rsPADENTER:
		if (CPad::GetPad(PAD1)->GetPadEnterJustDown())
			return true;
		break;
	case rsPADDEL:
		if (CPad::GetPad(PAD1)->GetPadDelJustDown())
			return true;
		break;
	case rsPADEND:
		if (CPad::GetPad(PAD1)->GetPad1JustDown())
			return true;
		break;
	case rsPADDOWN:
		if (CPad::GetPad(PAD1)->GetPad2JustDown())
			return true;
		break;
	case rsPADPGDN:
		if (CPad::GetPad(PAD1)->GetPad3JustDown())
			return true;
		break;
	case rsPADLEFT:
		if (CPad::GetPad(PAD1)->GetPad4JustDown())
			return true;
		break;
	case rsPAD5:
		if (CPad::GetPad(PAD1)->GetPad5JustDown())
			return true;
		break;
	case rsPADRIGHT:
		if (CPad::GetPad(PAD1)->GetPad6JustDown())
			return true;
		break;
	case rsPADHOME:
		if (CPad::GetPad(PAD1)->GetPad7JustDown())
			return true;
		break;
	case rsPADUP:
		if (CPad::GetPad(PAD1)->GetPad8JustDown())
			return true;
		break;
	case rsPADPGUP:
		if (CPad::GetPad(PAD1)->GetPad9JustDown())
			return true;
		break;
	case rsPADINS:
		if (CPad::GetPad(PAD1)->GetPad0JustDown())
			return true;
		break;
	case rsBACKSP:
		if (CPad::GetPad(PAD1)->GetBackspaceJustDown())
			return true;
		break;
	case rsTAB:
		if (CPad::GetPad(PAD1)->GetTabJustDown())
			return true;
		break;
	case rsCAPSLK:
		if (CPad::GetPad(PAD1)->GetCapsLockJustDown())
			return true;
		break;
	case rsENTER:
		if (CPad::GetPad(PAD1)->GetReturnJustDown())
			return true;
		break;
	case rsLSHIFT:
		if (CPad::GetPad(PAD1)->GetLeftShiftJustDown())
			return true;
		break;
	case rsSHIFT:
		if (CPad::GetPad(PAD1)->GetShiftJustDown())
			return true;
		break;
	case rsRSHIFT:
		if (CPad::GetPad(PAD1)->GetRightShiftJustDown())
			return true;
		break;
	case rsLCTRL:
		if (CPad::GetPad(PAD1)->GetLeftCtrlJustDown())
			return true;
		break;
	case rsRCTRL:
		if (CPad::GetPad(PAD1)->GetRightCtrlJustDown())
			return true;
		break;
	case rsLALT:
		if (CPad::GetPad(PAD1)->GetLeftAltJustDown())
			return true;
		break;
	case rsRALT:
		if (CPad::GetPad(PAD1)->GetRightAltJustDown())
			return true;
		break;
	case rsLWIN:
		if (CPad::GetPad(PAD1)->GetLeftWinJustDown())
			return true;
		break;
	case rsRWIN:
		if (CPad::GetPad(PAD1)->GetRightWinJustDown())
			return true;
		break;
	case rsAPPS:
		if (CPad::GetPad(PAD1)->GetAppsJustDown())
			return true;
		break;
	default: break;
	}

	return false;
}