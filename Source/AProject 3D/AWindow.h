#ifndef __APROJECT_WINDOW__
#define __APROJECT_WINDOW__

#pragma once

#pragma warning (disable: 4005)
#pragma warning (disable: 4244)
#pragma warning (disable: 4800)
#pragma warning (disable: 4995)
#pragma warning (disable: 4996)

// Direct Input Version Setting
#define DIRECTINPUT_VERSION 0x0800

// Singleton Class Definded
#define APROJECT_WINDOW	CAWindow::GetSingleton ()
#define AINPUT_KEYBOARD	CAInputKeyboard::GetSingleton ()
#define AINPUT_MOUSE	CAInputMouse::GetSingleton ()
#define ACAMERA			APROJECT_WINDOW->GetCurScene ()->GetCamera ()
//#define ACAMERA			this->GetCamera ()

// Standard Windows includes
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <hash_map>
#include <algorithm>
#include <msxml2.h>
#include <comdef.h>
#include <io.h>

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Direct3D9 includes
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <d3dx9effect.h>
#include <dinput.h>

// AProject Engines includes
#include "AUserDefine.h"

#include "ADataTypes.h"
#include "AMath.h"
#include "AList.h"
#include "ACamera.h"
#include "AAlgorithm.h"
#include "AGlareDefine.h"
#include "AHDRSystem.h"
#include "AXMLFile.h"
#include "APFinclude.h"

#include "AObject.h"
#include "AGrid.h"
#include "ATexture.h"
#include "AGeometry.h"
#include "AText.h"
#include "AXModel.h"
#include "AModelInclude.h"
#include "AMapInclude.h"

#include "ASprite.h"

#include "AScene.h"

#include "AInput.h"
#include "AMouse.h"
#include "AKeyboard.h"

#ifdef AUTO_LIBRARY
#pragma comment (lib, "d3d9.lib")
#if defined (DEBUG) | defined (_DEBUG)
#pragma comment (lib, "d3dx9d.lib")
#else
#pragma comment( lib, "d3dx9.lib" )
#endif
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "msvcrtd.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "msxml2.lib")
#endif

#if defined (DEBUG) | defined (_DEBUG) 
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#define CRTDBG_MAP_ALLOC
#if defined(DEBUG) || defined(_DEBUG)
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace std;

struct stMaterial
{
	D3DXCOLOR	AmbientColor;
	D3DXCOLOR	DiffuseColor;
	D3DXCOLOR	EmissiveColor;
	D3DXCOLOR	SpecularColor;
	float		Shininess;
};

struct stLight
{
	enum {DIR_LIGHT, POINT_LIGHT, SPOT_LIGHT};

	int			LightType;

	D3DXVECTOR3	LightDirection;		// world space direction
	D3DXVECTOR3	LightPosition;		// world space position
	
	D3DXCOLOR	DiffuseColor;
	D3DXCOLOR	SpecularColor;

	float		SpotInnerCone;		// spot light inner cone (theta) angle
	float		SpotOuterCone;		// spot light outer cone (phi) angle
	float		Radius;				// applies to point and spot lights only

	D3DXMATRIX	ViewProjMatrix;

	stMaterial	material;
};

class CAWindow
{
private:
	static CAWindow*				m_pSingleton;

	CAInput*						m_pkInput;
	CAInputMouse*					m_pkMouse;
	CAInputKeyboard*				m_pkKeyboard;
	map<wstring, CARootScene*>*		m_mapTable;
	LPD3DXEFFECT					m_pLightEffect;
	CAHDRLighting*					m_pHDRSystem;

private:
	LPDIRECT3DTEXTURE9				m_pDiffuse;
	LPDIRECT3DTEXTURE9				m_pNormal;
	LPDIRECT3DTEXTURE9				m_pDepth;
	LPDIRECT3DTEXTURE9				m_pXYMap;
	LPDIRECT3DTEXTURE9				m_pLightMap;
	LPDIRECT3DTEXTURE9				m_pSpecularMap;
	LPD3DXMESH						m_pSphereMesh;
	LPD3DXSPRITE					m_pSprite;

private:
	HICON							m_hIcon;
	HMENU							m_hMenu;

	AInt							m_nWidth;
	AInt							m_nHeight;
	HWND							m_hWindow;
	HINSTANCE						m_hInstance;
	LPDIRECT3D9						m_pD3D;
	LPDIRECT3DDEVICE9				m_pD3DDevice;

	D3DPRESENT_PARAMETERS			m_d3dpp;

	LPCALLBACKD3D9DEVICECREATED		m_pCallbackDeviceCreated;
	LPCALLBACKD3D9DEVICERESETED		m_pCallbackDeviceReseted;
	LPCALLBACKD3D9DEVICEDESTROYED	m_pCallbackDeviceDestoryed;

private:
	AReal32							m_fFPS;

	AUInt32							m_dCurTime;
	AUInt32							m_dLastTime;

	bool							m_bEndStep;
	bool							m_bResetStep;

	bool							m_bInitalize;
	
	bool							m_bForegroundMode;
	bool							m_bMsgExclusiveMode;
	bool							m_bNoWindowKey;

	bool							m_bMultiThreadSystem;
	bool							m_bEscapeToQuit;
	bool							m_bPauseToToggleTimePause;

	AUInt32							m_nStyle;
	CARootScene*					m_pCurScene;

public:
	void WINAPI SetCallbackD3D9DeviceCreated (LPCALLBACKD3D9DEVICECREATED pCallback);
	void WINAPI SetCallbackD3D9DeviceReseted (LPCALLBACKD3D9DEVICERESETED pCallback);
	void WINAPI SetCallbackD3D9DeviceDestoryed (LPCALLBACKD3D9DEVICEDESTROYED pCallback);

private:
	DWORD CrashPrinting (LPEXCEPTION_POINTERS lpException);
	void CheckFPS (void);
	bool CheckDevice (void);
	static LRESULT CALLBACK WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static DWORD WINAPI Update (LPVOID Param);
	void Render (void);
	void Destroy (void);

	HRESULT ResetDirect3D (bool bFullScreen);

public:
	CAWindow ();
	~CAWindow ();

	CARootScene* GetCurScene (void);
	static CAWindow* GetSingleton (void);
	LPD3DXEFFECT GetLightEffect (void) { return m_pLightEffect; }
	CAHDRLighting* GetHDRSystem (void) { return m_pHDRSystem; }
	HWND GetHWND (void) { return m_hWindow; }
	HINSTANCE GetHinstance (void) { return m_hInstance; }
	LPDIRECT3D9 GetD3D (void) { return m_pD3D; }
	LPDIRECT3DDEVICE9 GetD3DDevice (void) { return m_pD3DDevice; }
	D3DPRESENT_PARAMETERS* GetSetting (void) { return &m_d3dpp; }
	bool GetMultiThreadSysteming (void) { return m_bMultiThreadSystem; }
	HICON GetApplicationIcon (void) { return m_hIcon; }
	HMENU GetApplicationMenu (void) { return m_hMenu; }
	AUInt32 GetWidth(void) { return m_nWidth; }
	AUInt32 GetHeight(void) { return m_nHeight; }
	AReal32 GetCurFramerate (void) { return m_fFPS; }

	void SetExitLoop (void);
	void SetHinstance (HINSTANCE hInst);
	void SetApplicationIcon (HICON hIcon);
	void SetApplicationMenu (HMENU hMenu);

	void InputUpdate (void);

public:
	HRESULT Initialize (bool bMultiThreadSystem = false, bool bEscapeToQuit = true, bool bPauseToToggleTimePause = true);
	HRESULT CreateDefaultWindow (LPWSTR szTitle = L"AProject Engine", AInt nWidth = 800, AInt nHeight = 600, AUInt32 nStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	HRESULT CreateDirect3D (bool bForegroundMode = true, bool bMsgExclusiveMode = false, bool bNoWindowKey = false, bool bFullScreen = false);
	HRESULT RunLoop (void);
	HRESULT DestroyDefaultWindow (void);

	CARootScene* Trans (const WCHAR* pTitle);
	HRESULT Commit (const WCHAR* pTitle, CARootScene* pObject);
	HRESULT Destory (const WCHAR* pTitle);
	HRESULT SetCurrentScene (const WCHAR* pTitle);
};

#endif