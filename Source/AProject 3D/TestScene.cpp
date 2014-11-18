#include "Include.h"

CTestScene::CTestScene (void)
	: CARootScene ()
{
	CARootScene::Initialize ();
}

CTestScene::~CTestScene (void)
{
}

void CTestScene::DefaultSetting (LPDIRECT3DDEVICE9 pD3DDevice)
{
	GetCamera ()->SetView (&D3DXVECTOR3 (0.0f, 0.0f, 0.0f), &D3DXVECTOR3 (0.0f, 0.0f, 1.0f), &D3DXVECTOR3 (0.0f, 1.0f, 0.0f) );
}

void CTestScene::Create (LPDIRECT3DDEVICE9 pD3DDevice)
{
	CARootScene::Create (pD3DDevice);

	DefaultSetting (pD3DDevice);
}

void CTestScene::Reset (LPDIRECT3DDEVICE9 pD3DDevice)
{
	DefaultSetting (pD3DDevice);

	CARootScene::Reset (pD3DDevice);
}

void CTestScene::Update (LPDIRECT3DDEVICE9 pD3DDevice, AReal32 dt)
{
	CARootScene::Update (pD3DDevice, dt);
}

void CTestScene::Render (LPDIRECT3DDEVICE9 pD3DDevice, BOOL bRenderOrtho2D)
{
	CARootScene::Render (pD3DDevice, bRenderOrtho2D);
}
	
void CTestScene::Destroy (void)
{
	CARootScene::Destroy ();
}

void CALLBACK CTestScene::MouseEvent (AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown)
{
}

void CALLBACK CTestScene::KeyboardEvent (AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown)
{
}

void CALLBACK CTestScene::ObjectEvent (const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam)
{
}