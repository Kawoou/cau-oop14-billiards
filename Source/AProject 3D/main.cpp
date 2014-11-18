#include "Include.h"

CAWindow* g_kWindow = NULL;

HRESULT CALLBACK OnDeviceCreate (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	// Culling Mode OFF, 삼각형의 앞면.뒷면을 모두 렌더링한다.
	pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW);

	// 광원기능 OFF
	pD3DDevice->SetRenderState (D3DRS_LIGHTING, FALSE);

	// Z버퍼 기능을 킨다.
	pD3DDevice->SetRenderState (D3DRS_ZENABLE, TRUE);

	pD3DDevice->SetRenderState (D3DRS_FOGENABLE, FALSE);

	/* Setup a render state */
	pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//g_kWindow->Commit (L"Test Scene", new CTestScene () );
	g_kWindow->Commit (L"Game Scene", new CGameScene () );
	//g_kWindow->SetCurrentScene (L"Game Scene");

	return AE_SUCCESS;
}

HRESULT CALLBACK OnDeviceReset (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	return AE_SUCCESS;
}

void CALLBACK OnDeviceDestroy ()
{
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_kWindow = new CAWindow ();

	g_kWindow->Initialize (false, true, false);

	g_kWindow->SetCallbackD3D9DeviceCreated (OnDeviceCreate);
	g_kWindow->SetCallbackD3D9DeviceReseted (OnDeviceReset);
	g_kWindow->SetCallbackD3D9DeviceDestoryed (OnDeviceDestroy);
	g_kWindow->SetHinstance (hInstance);

	//g_kWindow->CreateDefaultWindow (L"AProject Engine", 1024, 768);
	g_kWindow->CreateDefaultWindow(L"AProject Engine", 1440, 900);
#if defined (DEBUG) | defined (_DEBUG)
	g_kWindow->CreateDirect3D (true, false, false, false);
#else
	g_kWindow->CreateDirect3D(true, false, false, true);
#endif

	g_kWindow->RunLoop ();

	SAFE_DELETE (g_kWindow);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtDumpMemoryLeaks();
}