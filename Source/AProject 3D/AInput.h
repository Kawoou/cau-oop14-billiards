#ifndef __APROJECT_INPUT__
#define __APROJECT_INPUT__

#pragma once

class CAInput
{
protected:
	LPDIRECTINPUT8			m_pDInput;
	static CAInput*			m_pSingleton;

public:
	CAInput (HINSTANCE hInstace);
	~CAInput ();

	LPDIRECTINPUT8 GetInput(void);
	static CAInput* GetSingleton (void);
};

#endif