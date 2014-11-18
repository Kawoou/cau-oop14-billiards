#include "AWindow.h"

CAInputMouse* CAInputMouse::m_pSingleton = NULL;

CAInputMouse::CAInputMouse (HWND hWnd, bool bForegroundMode, bool bMsgExclusiveMode)
	: m_hWnd (hWnd)
{
	m_pDIDevice = NULL;
	m_pSceneEvent = NULL;
	ZeroMemory (&m_arrMouseState, sizeof (DIMOUSESTATE2) );

	m_pSingleton = this;
	
	if (FAILED (CAInput::GetSingleton ()->GetInput ()->CreateDevice (GUID_SysMouse, &m_pDIDevice, NULL) ) )
	{
		// Error Mouse Create Device
		DebugLog ("CAInputMouse(Create Device Error) 실패!");
		DebugConsoleLog (DebugError, "CAInputMouse: CAInputMouse(Create Device Error) 실패!");
		return;
	}

	if (FAILED (m_pDIDevice->SetDataFormat (&c_dfDIMouse) ) )
	{
		// Error Mouse Set Data Format
		DebugLog ("CAInputMouse(Select data format Error) 실패!");
		DebugConsoleLog (DebugError, "CAInputMouse: CAInputMouse(Select data format Error) 실패!");
		return;
	}

	AUInt32 unFlags = ( (bForegroundMode) ? DISCL_FOREGROUND : DISCL_BACKGROUND);
	unFlags |= ( (bMsgExclusiveMode) ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE);
	if (FAILED (m_pDIDevice->SetCooperativeLevel (hWnd, unFlags) ) )
	{
		// Error Mouse Set Cooperative Level
		DebugLog ("CAInputMouse(Failed to set cooperative level) 실패!");
		DebugConsoleLog (DebugError, "CAInputMouse: CAInputMouse(Failed to set cooperative level) 실패!");
		return;
	}

	DIPROPDWORD kDIProp;
	
	kDIProp.diph.dwSize = sizeof (DIPROPDWORD);
	kDIProp.diph.dwHeaderSize = sizeof (DIPROPHEADER);
	kDIProp.diph.dwObj = 0;
	kDIProp.diph.dwHow = DIPH_DEVICE;
	kDIProp.dwData = 8;

	if (FAILED (m_pDIDevice->SetProperty (DIPROP_BUFFERSIZE, &kDIProp.diph) ) )
	{
		// Error Mouse Set Property
		DebugLog ("CAInputMouse(Failed to set mouse property) 실패!");
		DebugConsoleLog (DebugError, "CAInputMouse: CAInputMouse(Failed to set mouse property) 실패!");
		return;
	}

	m_pDIDevice->Acquire ();

	m_nX = GetMousePos().x;
	m_nY = GetMousePos().y;

	//DebugConsoleLog (DebugClear, "CAInputMouse: Acquire a mouse device, the success");
	DebugConsoleLog (DebugMsg, "CAInputMouse: Mouse Acquire!");
}

CAInputMouse::~CAInputMouse ()
{
	m_pSingleton = NULL;

	if (m_pDIDevice != NULL)
	{
		m_pDIDevice->Unacquire ();
		m_pDIDevice->Release ();
		m_pDIDevice = NULL;

		DebugConsoleLog (DebugMsg, "CAInputMouse: Mouse UnAcquire!");
	}
}

HRESULT CAInputMouse::Update (void)
{
	ZeroMemory (&m_arrMouseState, sizeof (DIMOUSESTATE) );
	
	memcpy (&m_arrLastMouseState, &m_arrMouseState, sizeof (DIMOUSESTATE) );
	HRESULT hRet = m_pDIDevice->GetDeviceState (sizeof(DIMOUSESTATE), &m_arrMouseState);
	if (FAILED (hRet))
	{
		m_pDIDevice->Acquire ();
		if (FAILED (m_pDIDevice->GetDeviceState (sizeof(DIMOUSESTATE), &m_arrMouseState) ) )
		{
			// Error Mouse Get Device Data
			//DebugLog ("Update(Failed to get mouse data) 실패!");
			//DebugConsoleLog (DebugError, "CAInputMouse: Update(Failed to get mouse data) 실패!");
			
			return AE_ERROR_GET_MOUSEDATA;
		}
		DebugConsoleLog (DebugMsg, "CAInputMouse: Mouse Acquire!");
	}

	m_nX = GetMousePos().x;
	m_nY = GetMousePos().y;
	//m_nX += m_arrMouseState.lX;
	//m_nY += m_arrMouseState.lY;

	if ((m_arrMouseState.lX || m_arrMouseState.lY) && m_pSceneEvent)
		m_pSceneEvent->MouseEvent (AM_MOUSEMOVE, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
	
	if (m_arrMouseState.lZ && m_pSceneEvent)
		m_pSceneEvent->MouseEvent (AM_MOUSEWHEEL, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );

	if ( (bool)(m_arrLastMouseState.rgbButtons [0] & 0x80) != this->IsButtonDown (0) && m_pSceneEvent)
	{
		if (this->IsButtonDown (0) )
			m_pSceneEvent->MouseEvent (AM_LBUTTONDOWN, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
		else
			m_pSceneEvent->MouseEvent (AM_LBUTTONUP, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
	}

	if ( (bool)(m_arrLastMouseState.rgbButtons [1] & 0x80) != this->IsButtonDown (1) && m_pSceneEvent)
	{
		if (this->IsButtonDown (1) )
			m_pSceneEvent->MouseEvent (AM_RBUTTONDOWN, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
		else
			m_pSceneEvent->MouseEvent (AM_RBUTTONUP, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
	}

	if ( (bool)(m_arrLastMouseState.rgbButtons [2] & 0x80) != this->IsButtonDown (2) && m_pSceneEvent)
	{
		if (this->IsButtonDown (2) )
			m_pSceneEvent->MouseEvent (AM_MBUTTONDOWN, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
		else
			m_pSceneEvent->MouseEvent (AM_MBUTTONUP, m_nX, m_nY, m_arrMouseState.lZ, this->IsButtonDown (0), this->IsButtonDown (1), this->IsButtonDown (2) );
	}

	return AE_SUCCESS;
}

bool CAInputMouse::IsButtonDown (AInt32 iButton)
{
	if (m_arrMouseState.rgbButtons [iButton] & 0x80)
		return true;

	return false;
}

bool CAInputMouse::IsButtonUp (AInt32 iButton)
{
	if (m_arrMouseState.rgbButtons [iButton] & 0x80)
		return false;

	return true;
}

AUInt32 CAInputMouse::GetXAxis (void)
{
	return m_arrMouseState.lX;
}

AUInt32 CAInputMouse::GetYAxis (void)
{
	return m_arrMouseState.lY;
}

AUInt32 CAInputMouse::GetZAxis (void)
{
	return m_arrMouseState.lZ;
}

POINT CAInputMouse::GetMousePos (void)
{
	POINT pt;
	GetCursorPos (&pt);
	ScreenToClient (APROJECT_WINDOW->GetHWND (), &pt);

	return pt;
}

void CAInputMouse::SetCallbackSceneEvent (CARootScene* pSceneEvent)
{
	m_pSceneEvent = pSceneEvent;
}

bool CAInputMouse::Clear (void)
{
	ZeroMemory (&m_arrMouseState, sizeof (m_arrMouseState) );
	
	AUInt32 dwItems = INFINITE;
	if (FAILED (m_pDIDevice->GetDeviceState (sizeof (DIMOUSESTATE), NULL) ) )
	{
		// Error Mouse Get Device Data
		DebugLog ("Update(Failed to get mouse device data) 실패!");
		DebugConsoleLog (DebugError, "CAInputMouse: Update(Failed to get mouse device data) 실패!");
		return false;
	}

	return true;
}

CAInputMouse* CAInputMouse::GetSingleton (void)
{
	return m_pSingleton;
}