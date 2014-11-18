#ifndef __APROJECT_INPUT_MOUSE__
#define __APROJECT_INPUT_MOUSE__

#pragma once

class CAInputMouse
{
protected:
	LPDIRECTINPUTDEVICE8W	m_pDIDevice;
	DIMOUSESTATE			m_arrLastMouseState;
	DIMOUSESTATE			m_arrMouseState;
	HWND					m_hWnd;
	static CAInputMouse*	m_pSingleton;

	AInt32					m_nX;
	AInt32					m_nY;
	CARootScene*			m_pSceneEvent;

public:
	CAInputMouse (HWND hWnd, bool bForegroundMode = true, bool bMsgExclusiveMode = false);
	~CAInputMouse ();

	HRESULT Update (void);

	bool IsButtonDown (AInt32 iButton);
	bool IsButtonUp (AInt32 iButton);
	AUInt32 GetXAxis (void);
	AUInt32 GetYAxis (void);
	AUInt32 GetZAxis (void);
	POINT	GetMousePos (void);

	void SetCallbackSceneEvent (CARootScene* pSceneEvent);

	bool Clear (void);

	static CAInputMouse* GetSingleton (void);
};

#endif