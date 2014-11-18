#ifndef __APROJECT_INPUT_KEYBOARD__
#define __APROJECT_INPUT_KEYBOARD__

#pragma once

class CAInputKeyboard
{
private:
	bool						m_bKeyState [256][5];
	AUInt32						m_dwKeyDown [256];
	AUInt32						m_dwKeyUp [256];

protected:
	LPDIRECTINPUTDEVICE8W		m_pDIDevice;
	BYTE						m_iLastKeyBuffer [256];
	BYTE						m_iKeyBuffer [256];
	static CAInputKeyboard*		m_pSingleton;

	CARootScene*				m_pSceneEvent;

private:
	void TableUpdate (void);

public:
	CAInputKeyboard (HWND hWnd, bool bForegroundMode = true, bool bMsgExclusiveMode = false, bool bNoWindowKey = false);
	~CAInputKeyboard ();

	HRESULT Update (void);

	ABool32 IsButtonDown (AInt32 iButton);
	ABool32 IsButtonUp (AInt32 iButton);

	void SetCallbackSceneEvent (CARootScene* pSceneEvent);

	static CAInputKeyboard* GetSingleton (void);
};

#endif