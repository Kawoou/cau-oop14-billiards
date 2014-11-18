#include "AWindow.h"

CAInputKeyboard* CAInputKeyboard::m_pSingleton = NULL;

CAInputKeyboard::CAInputKeyboard (HWND hWnd, bool bForegroundMode, bool bMsgExclusiveMode, bool bNoWindowKey)
{
	int i;
	m_pSingleton = this;
	m_pSceneEvent = NULL;

	m_pDIDevice = NULL;
	ZeroMemory (m_iKeyBuffer, sizeof (AUInt8) * 256);

	memset (m_dwKeyUp, 1, sizeof (AUInt32) * 256);
	ZeroMemory (m_dwKeyDown, sizeof (AUInt32) * 256);
	for (i = 0; i < 256; i ++)
	{
		ZeroMemory (m_bKeyState [i], 5);
		m_bKeyState [i][0] = true;
	}
		
	if (FAILED (CAInput::GetSingleton ()->GetInput ()->CreateDevice (GUID_SysKeyboard, &m_pDIDevice, NULL) ) )
	{
		// Error Keyboard Create Device
		DebugLog ("CAInputKeyboard(Create Device Error) 실패!");
		DebugConsoleLog (DebugError, "CAInputKeyboard: CAInputKeyboard(Create Device Error) 실패!");
		return;
	}

	if (FAILED (m_pDIDevice->SetDataFormat (&c_dfDIKeyboard) ) )
	{
		// Error Keyboard Set Data Format
		DebugLog ("CAInputKeyboard(Select data format Error) 실패!");
		DebugConsoleLog (DebugError, "CAInputKeyboard: CAInputKeyboard(Select data format Error) 실패!");
		return;
	}

	AUInt32 unFlags = ( (bForegroundMode) ? DISCL_FOREGROUND : DISCL_BACKGROUND);
	unFlags |= ( (bMsgExclusiveMode) ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE);
	unFlags |= ( (bNoWindowKey) ? DISCL_NOWINKEY : 0x00);
	if (FAILED (m_pDIDevice->SetCooperativeLevel (hWnd, unFlags) ) )
	{
		// Error Mouse Set Cooperative Level
		DebugLog ("CAInputKeyboard(Select data format Error) 실패!");
		DebugConsoleLog (DebugError, "CAInputKeyboard: CAInputKeyboard(Failed to set cooperative level) 실패!");
		return;
	}

	m_pDIDevice->Acquire ();

	//DebugConsoleLog (DebugClear, "CAInputKeyboard: Acquire a keyboard device, the success");
	DebugConsoleLog (DebugMsg, "CAInputKeyboard: Keyboard Acquire!");
	return;
}
	
CAInputKeyboard::~CAInputKeyboard ()
{
	m_pSingleton = NULL;

	if (NULL != m_pDIDevice)
	{
		m_pDIDevice->Unacquire ();
		m_pDIDevice->Release ();
		m_pDIDevice = NULL;

		DebugConsoleLog (DebugMsg, "CAInputKeyboard: Keyboard UnAcquire!");
	}
}

void CAInputKeyboard::TableUpdate (void)
{
	int vkKey;
	for (vkKey = 0; vkKey < 256; vkKey ++)
	{
		if (IsButtonDown (vkKey) == 0)
		{
			m_dwKeyDown [vkKey] = 0;
			if (m_dwKeyUp [vkKey] == 0)
				m_dwKeyUp [vkKey] = timeGetTime ();

			if (m_bKeyState [vkKey][0] || m_bKeyState [vkKey][1])
			{
				if (m_bKeyState [vkKey][0] == false && m_pSceneEvent)
					m_pSceneEvent->KeyboardEvent (vkKey, AK_KEYNONE, IsButtonDown (DIK_LCONTROL) | IsButtonDown (DIK_RCONTROL), IsButtonDown (DIK_LMENU) | IsButtonDown (DIK_RMENU), IsButtonDown (DIK_LSHIFT) | IsButtonDown (DIK_RSHIFT) );

				m_bKeyState [vkKey][0] = true;
				m_bKeyState [vkKey][1] = false;
				m_bKeyState [vkKey][2] = false;
				m_bKeyState [vkKey][3] = false;
				m_bKeyState [vkKey][4] = false;
			}
			else
			{
				if (m_bKeyState [vkKey][3] || m_bKeyState [vkKey][4])
					m_dwKeyUp [vkKey] = 1;

				if (m_bKeyState [vkKey][1] == false && m_pSceneEvent)
					m_pSceneEvent->KeyboardEvent (vkKey, AK_KEYUP, IsButtonDown (DIK_LCONTROL) | IsButtonDown (DIK_RCONTROL), IsButtonDown (DIK_LMENU) | IsButtonDown (DIK_RMENU), IsButtonDown (DIK_LSHIFT) | IsButtonDown (DIK_RSHIFT) );

				m_bKeyState [vkKey][0] = false;
				m_bKeyState [vkKey][1] = true;
				m_bKeyState [vkKey][2] = false;
				m_bKeyState [vkKey][3] = false;
				m_bKeyState [vkKey][4] = false;
			}
		}
		else
		{
			if (m_dwKeyDown [vkKey] == 0)
				m_dwKeyDown [vkKey] = timeGetTime ();

			if (m_bKeyState [vkKey][2] == false && m_pSceneEvent)
				m_pSceneEvent->KeyboardEvent (vkKey, AK_KEYDOWN, IsButtonDown (DIK_LCONTROL) | IsButtonDown (DIK_RCONTROL), IsButtonDown (DIK_LMENU) | IsButtonDown (DIK_RMENU), IsButtonDown (DIK_LSHIFT) | IsButtonDown (DIK_RSHIFT) );

			m_bKeyState [vkKey][0] = false;
			m_bKeyState [vkKey][1] = false;
			m_bKeyState [vkKey][2] = true;

			if (timeGetTime () - m_dwKeyDown [vkKey] > 500)
			{
				if (m_bKeyState [vkKey][3] == false && m_pSceneEvent)
					m_pSceneEvent->KeyboardEvent (vkKey, AK_KEYLONG, IsButtonDown (DIK_LCONTROL) | IsButtonDown (DIK_RCONTROL), IsButtonDown (DIK_LMENU) | IsButtonDown (DIK_RMENU), IsButtonDown (DIK_LSHIFT) | IsButtonDown (DIK_RSHIFT) );
				m_bKeyState [vkKey][3] = true;
			}
			
			if (m_dwKeyUp [vkKey] == 0 && timeGetTime () - m_dwKeyUp [vkKey] < 500)
			{
				if (m_bKeyState [vkKey][4] == false && m_pSceneEvent)
					m_pSceneEvent->KeyboardEvent (vkKey, AK_KEYDOUBLE, IsButtonDown (DIK_LCONTROL) | IsButtonDown (DIK_RCONTROL), IsButtonDown (DIK_LMENU) | IsButtonDown (DIK_RMENU), IsButtonDown (DIK_LSHIFT) | IsButtonDown (DIK_RSHIFT) );
				m_bKeyState [vkKey][4] = true;
			}

			m_dwKeyUp [vkKey] = 0;
		}
	}
}

HRESULT CAInputKeyboard::Update (void)
{
	memcpy (&m_iLastKeyBuffer, &m_iKeyBuffer, sizeof (m_iKeyBuffer) );

	if (FAILED (m_pDIDevice->GetDeviceState (sizeof (m_iKeyBuffer), (void*) &m_iKeyBuffer) ) )
	{
		if (FAILED (m_pDIDevice->Acquire () ) )
		{
			// Error Keyboard Acquire
			//DebugLog ("Update(Failed to keyboard acquire) 실패!");
			//DebugConsoleLog (DebugError, "CAInputKeyboard: Update(Failed to keyboard acquire) 실패!");
			
			return AE_ERROR_KEYBOARD_ACQUIRE;
		}
		DebugConsoleLog (DebugMsg, "CAInputKeyboard: Keyboard Acquire!");
	}

	TableUpdate ();

	return AE_SUCCESS;
}

ABool32 CAInputKeyboard::IsButtonDown (AInt32 iButton)
{
	if (m_iKeyBuffer [iButton] & (1 << 7) )
		return ATrue;
	else
		return AFalse;
}

ABool32 CAInputKeyboard::IsButtonUp (AInt32 iButton)
{
	if ( (int)!(m_iKeyBuffer [iButton] & (1 << 7) ) )
		return ATrue;
	else
		return AFalse;
}

void CAInputKeyboard::SetCallbackSceneEvent (CARootScene* pSceneEvent)
{
	m_pSceneEvent = pSceneEvent;
}

CAInputKeyboard* CAInputKeyboard::GetSingleton (void)
{
	return m_pSingleton;
}