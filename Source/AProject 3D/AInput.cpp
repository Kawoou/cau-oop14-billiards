#include "AWindow.h"

CAInput* CAInput::m_pSingleton = NULL;
	
CAInput::CAInput (HINSTANCE hInstace)
{
	m_pDInput = NULL;

	m_pSingleton = this;

	if (FAILED (DirectInput8Create (hInstace, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDInput, NULL) ) )
	{
		DebugLog ("CAInput(Failed to create DirectInput) 실패!");
		DebugConsoleLog (DebugError, "CAInput: CAInput(Failed to create DirectInput) 실패!");
		return;
	}
	else
		DebugConsoleLog (DebugClear, "CAInput: DirectInput creates success!");
}

CAInput::~CAInput ()
{
	DebugConsoleLog (DebugClear, "CAInput: DirectInput Destroy");

	SAFE_RELEASE (m_pDInput);
	m_pSingleton = NULL;
}

LPDIRECTINPUT8 CAInput::GetInput(void)
{
	return m_pDInput;
}

CAInput* CAInput::GetSingleton (void)
{
	return m_pSingleton;
}