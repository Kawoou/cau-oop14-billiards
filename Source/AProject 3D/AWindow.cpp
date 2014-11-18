#include "AWindow.h"

CAWindow* CAWindow::m_pSingleton = NULL;

CAWindow::CAWindow ()
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	m_bInitalize = false;

	if (m_pSingleton == NULL)
		m_pSingleton = this;

	m_pkInput = NULL;
	m_pkMouse = NULL;
	m_pkKeyboard = NULL;

	m_pLightEffect = NULL;

	m_pCurScene = NULL;

	m_mapTable = new map<wstring, CARootScene*>();
	m_mapTable->clear ();

	m_bResetStep = true;
}

CAWindow::~CAWindow ()
{
	Destroy ();

	SAFE_DELETE(m_mapTable);
	m_pSingleton = NULL;

#if defined (DEBUG) | defined (_DEBUG) 
	system ("pause");
#endif
}

void CAWindow::Destroy (void)
{
	map<wstring, CARootScene *>::iterator itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		itr->second->Destroy();
		SAFE_DELETE(itr->second);
		itr++;
	}
	m_mapTable->clear();

	SAFE_RELEASE(m_pDiffuse);
	SAFE_RELEASE(m_pNormal);
	SAFE_RELEASE(m_pDepth);
	SAFE_RELEASE(m_pXYMap);
	SAFE_RELEASE(m_pLightMap);
	SAFE_RELEASE(m_pSpecularMap);
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pSphereMesh);

	//m_pHDRSystem->Lost ();
	//m_pHDRSystem->Destroy ();
	//SAFE_DELETE (m_pHDRSystem);
	SAFE_RELEASE (m_pLightEffect);

	SAFE_DELETE (m_pkInput);
	SAFE_DELETE (m_pkMouse);
	SAFE_DELETE (m_pkKeyboard);

	SAFE_RELEASE (m_pD3D);
	SAFE_RELEASE (m_pD3DDevice);
}

CAWindow* CAWindow::GetSingleton (void)
{
	return m_pSingleton;
}

void CAWindow::SetHinstance (HINSTANCE hInst)
{
	if (hInst)
		m_hInstance = hInst;
	else
		m_hInstance = (HINSTANCE) GetModuleHandle (NULL);
}

void CAWindow::SetApplicationIcon (HICON hIcon)
{
	m_hIcon = hIcon;
}

void CAWindow::SetApplicationMenu (HMENU hMenu)
{
	m_hMenu = hMenu;
}

void WINAPI CAWindow::SetCallbackD3D9DeviceCreated (LPCALLBACKD3D9DEVICECREATED pCallback)
{
	m_pCallbackDeviceCreated = pCallback;
}

void WINAPI CAWindow::SetCallbackD3D9DeviceReseted (LPCALLBACKD3D9DEVICERESETED pCallback)
{
	m_pCallbackDeviceReseted = pCallback;
}

void WINAPI CAWindow::SetCallbackD3D9DeviceDestoryed (LPCALLBACKD3D9DEVICEDESTROYED pCallback)
{
	m_pCallbackDeviceDestoryed = pCallback;
}

LRESULT CALLBACK CAWindow::WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAWindow*	pWindow   = NULL;
	ABool32		bProcessed = AFalse;

	// Window message, Let our handler process it
	switch (msg)
	{
		case WM_DESTROY:
		case WM_CLOSE:
			APROJECT_WINDOW->SetExitLoop ();
			break;
		default:
			break;
	}

	// Message not processed, let windows handle it
	return DefWindowProc (hWnd, msg, wParam, lParam);
}

void CAWindow::InputUpdate (void)
{
	if (m_pkMouse) m_pkMouse->Update ();
	if (m_pkKeyboard) m_pkKeyboard->Update ();
}

DWORD WINAPI CAWindow::Update (LPVOID Param)
{
	DWORD dwCurTime;
	bool* pContinue = (bool*) Param;
	static DWORD dwLastTime = timeGetTime ();
	do
	{
		dwCurTime = timeGetTime ();

		AReal32 dt = (AReal32)(dwCurTime - dwLastTime) / 1000.0f;

		APROJECT_WINDOW->InputUpdate ();
		if (APROJECT_WINDOW->GetCurScene ())
			APROJECT_WINDOW->GetCurScene ()->Update (APROJECT_WINDOW->GetD3DDevice (), dt);

		dwLastTime = dwCurTime;
	} while (APROJECT_WINDOW->GetMultiThreadSysteming () && !(*pContinue));

	return 0;
}

void CAWindow::Render (void)
{
	if (!m_pCurScene)
		return;

	//m_pHDRSystem->PreRender (m_pD3DDevice);

	LPDIRECT3DSURFACE9 pBackbuffer;

	LPDIRECT3DSURFACE9 pDiffuseSurface = NULL;
	LPDIRECT3DSURFACE9 pNormalSurface = NULL;
	LPDIRECT3DSURFACE9 pDepthSurface = NULL;
	LPDIRECT3DSURFACE9 pXYSurface = NULL;

	LPDIRECT3DSURFACE9 pLightSurface = NULL;
	LPDIRECT3DSURFACE9 pSpecularSurface = NULL;

	m_pD3DDevice->GetRenderTarget (0, &pBackbuffer);


	// Render - 1 Pass
	m_pDiffuse->GetSurfaceLevel (0, &pDiffuseSurface);
	m_pD3DDevice->SetRenderTarget (0, pDiffuseSurface);
	SAFE_RELEASE (pDiffuseSurface);
	m_pD3DDevice->Clear (0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);

	m_pNormal->GetSurfaceLevel(0, &pNormalSurface);
	m_pD3DDevice->SetRenderTarget (1, pNormalSurface);
	SAFE_RELEASE (pNormalSurface);
	m_pD3DDevice->Clear (1, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);

	m_pDepth->GetSurfaceLevel(0, &pDepthSurface);
	m_pD3DDevice->SetRenderTarget (2, pDepthSurface);
	SAFE_RELEASE (pDepthSurface);
	m_pD3DDevice->Clear (2, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);

	m_pXYMap->GetSurfaceLevel(0, &pXYSurface);
	m_pD3DDevice->SetRenderTarget(3, pXYSurface);
	SAFE_RELEASE(pXYSurface);
	m_pD3DDevice->Clear (3, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);

	m_pCurScene->Render (m_pD3DDevice, FALSE);


	// Render - 2 Pass
	DWORD state;
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_FILLMODE, &state);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_FILLMODE, D3DFILL_SOLID);
	
	stLight light [4];
	{
		D3DXMATRIX matView, matProj;
		D3DXMatrixLookAtLH (&matView, &D3DXVECTOR3 (40.f, 60.f, -40.f), &D3DXVECTOR3 (0.f, 0.f, 0.f), &D3DXVECTOR3 (0.f, 1.f, 0.f) );
		D3DXMatrixOrthoLH (&matProj, 45.0f, 45.0f, 1.0f, 1024.0f);
		stLight tlight [4] = {
			{
				stLight::DIR_LIGHT,					// Type
				D3DXVECTOR3(-0.894427f, -0.447214f, 0.0f),	// Direction
				D3DXVECTOR3 (0.0f, 50.0f, 0.0f),	// Position
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),	// Diffuse
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),	// Specular
				D3DXToRadian (10.f),				// spotInnerCone
				D3DXToRadian (30.f),				// spotOuterCone
				100.f,								// Radiaus

				matView * matProj,

					// Matarials
				{
					D3DXCOLOR (0.3f, 0.3f, 0.3f, 0.0f),	// Ambient
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Diffuse
					D3DXCOLOR (0.0f, 0.0f, 0.0f, 0.0f),	// Emissive
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Specular
					90.0f								// Shininess
				}
			},
			{
				stLight::POINT_LIGHT,						// Type
				D3DXVECTOR3 (1.0f, -1.0f, 0.0f),		// Direction
				D3DXVECTOR3 (0.0f, 2.0f, 0.0f),			// Position
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),		// Diffuse
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),		// Specular
				D3DXToRadian (10.f),					// spotInnerCone
				D3DXToRadian (30.f),					// spotOuterCone
				50.f,									// Radiaus

				matView * matProj,

				// Matarials
				{
					D3DXCOLOR (0.3f, 0.3f, 0.3f, 0.0f),	// Ambient
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Diffuse
					D3DXCOLOR (0.0f, 0.0f, 0.0f, 0.0f),	// Emissive
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Specular
					100.0f								// Shininess
				}
			},
			{
				stLight::DIR_LIGHT,					// Type
				D3DXVECTOR3 (0.0f, -1.0f, 1.0f),	// Direction
				D3DXVECTOR3 (0.0f, 50.0f, 0.0f),	// Position
				D3DXCOLOR (0.0f, 1.0f, 0.0f, 0.f),	// Diffuse
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),	// Specular
				D3DXToRadian (10.f),				// spotInnerCone
				D3DXToRadian (30.f),				// spotOuterCone
				100.f,								// Radiaus

				matView * matProj,

				// Matarials
				{
					D3DXCOLOR (0.3f, 0.3f, 0.3f, 0.0f),	// Ambient
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Diffuse
					D3DXCOLOR (0.0f, 0.0f, 0.0f, 0.0f),	// Emissive
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Specular
					90.0f								// Shininess
				}
			},
			{
				stLight::DIR_LIGHT,					// Type
				D3DXVECTOR3 (0.0f, -1.0f, -1.0f),	// Direction
				D3DXVECTOR3 (0.0f, 50.0f, 0.0f),	// Position
				D3DXCOLOR (0.0f, 0.0f, 1.0f, 0.f),	// Diffuse
				D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.f),	// Specular
				D3DXToRadian (10.f),				// spotInnerCone
				D3DXToRadian (30.f),				// spotOuterCone
				100.f,								// Radiaus

				matView * matProj,

				// Matarials
				{
					D3DXCOLOR (0.3f, 0.3f, 0.3f, 0.0f),	// Ambient
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Diffuse
					D3DXCOLOR (0.0f, 0.0f, 0.0f, 0.0f),	// Emissive
					D3DXCOLOR (1.0f, 1.0f, 1.0f, 0.0f),	// Specular
					90.0f								// Shininess
				}
			}
		};
		memcpy (light, tlight, sizeof (tlight) );
	}

	m_pLightMap->GetSurfaceLevel(0, &pLightSurface);
	m_pD3DDevice->SetRenderTarget (0, pLightSurface);
	SAFE_RELEASE (pLightSurface);

	m_pSpecularMap->GetSurfaceLevel(0, &pSpecularSurface);
	m_pD3DDevice->SetRenderTarget (1, pSpecularSurface);
	m_pD3DDevice->SetRenderTarget (2, NULL);
	m_pD3DDevice->SetRenderTarget (3, NULL);
	SAFE_RELEASE (pSpecularSurface);

	m_pD3DDevice->Clear (0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);
	m_pD3DDevice->Clear (1, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);

	UINT totalPasses;
	D3DXHANDLE hTechnique;

	//m_pHDRSystem->ClearLight ();
	for (int j = 0; j < 1; j ++)
	{
		int i = j % 4;
		APROJECT_WINDOW->GetLightEffect ()->SetFloat ("lightIntensity", 3);
		APROJECT_WINDOW->GetLightEffect ()->SetValue ("g_lLight", &(light [i]), sizeof (light [i]) );

		//m_pHDRSystem->AddLight (D3DXVECTOR4(light [i].LightPosition.x, light [i].LightPosition.y, light [i].LightPosition.z, 1.0f), i * i + 1);

		if (light [i].LightType == stLight::DIR_LIGHT)
			hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName ("LightScene_Dir");
		else
			hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName ("LightScene_Point");

		D3DXMATRIXA16 mat;
		D3DXMatrixIdentity (&mat);
		if (light [i].LightType == stLight::DIR_LIGHT)
			APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &mat);
		else
		{
			D3DXMATRIXA16 tmpMat1, tmpMat2;
			D3DXMatrixIdentity (&tmpMat2);
			D3DXMatrixTranslation (&tmpMat1, light [i].LightPosition.x, light [i].LightPosition.y, light [i].LightPosition.z);
			D3DXMatrixScaling (&mat, light [i].Radius, light [i].Radius, light [i].Radius);

			tmpMat2 = tmpMat2 * mat * tmpMat1;

			APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &tmpMat2);
		}

		mat = *GetCurScene ()->GetCamera ()->GetViewMatrix () * *GetCurScene ()->GetCamera ()->GetProjectionMatrix ();
		D3DXMatrixInverse (&mat, NULL, &mat);

		float halfVector [2] = {0.5f / (float)APROJECT_WINDOW->GetSetting ()->BackBufferWidth, 0.5f / (float)APROJECT_WINDOW->GetSetting ()->BackBufferHeight};
		//APROJECT_WINDOW->GetLightEffect ()->SetValue ("halfPixel", halfVector, 8);
		APROJECT_WINDOW->GetLightEffect ()->SetValue ("cameraPosition", GetCurScene ()->GetCamera ()->GetFrustum ()->GetPos (), 12);
		APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("ITViewProjection", &mat);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture1", m_pDiffuse);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture2", m_pNormal);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture3", m_pDepth);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture4", m_pXYMap);

		if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
		{
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
					{
						if (light[i].LightType == stLight::DIR_LIGHT)
						{
							struct PlaneVertex {
								float x, y, z, w;
								float u, v;
							};
							static PlaneVertex axPlaneVertices[] =
							{
								{ 0, 0, .5f, 1, 0 + .5f / m_nWidth, 0 + .5f / m_nHeight },
								{ m_nWidth, 0, .5f, 1, 1 + .5f / m_nWidth, 0 + .5f / m_nHeight },
								{ m_nWidth, m_nHeight, .5f, 1, 1 + .5f / m_nWidth, 1 + .5f / m_nHeight },
								{ 0, m_nHeight, .5f, 1, 0 + .5f / m_nWidth, 1 + .5f / m_nHeight }
							};
							m_pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
							m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, axPlaneVertices, sizeof(PlaneVertex));
						}
						else
						{
							m_pSphereMesh->DrawSubset (0);
						}
						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End ();
			}
		}
	}

	m_pD3DDevice->SetRenderTarget (0, pBackbuffer);
	SAFE_RELEASE (pBackbuffer);

	// Render - 3 Pass
	m_pD3DDevice->Clear (0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
	if (SUCCEEDED (m_pD3DDevice->BeginScene () ))
	{
		hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName ("LastScene");
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture2", m_pLightMap);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture3", m_pSpecularMap);

		if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
		{
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
					{
						struct PlaneVertex {
							float x, y, z, w;
							float u, v;
						};
						static PlaneVertex axPlaneVertices[] =
						{
							{ 0, 0, .5f, 1, 0 + .5f / m_nWidth, 0 + .5f / m_nHeight },
							{ m_nWidth, 0, .5f, 1, 1 + .5f / m_nWidth, 0 + .5f / m_nHeight },
							{ m_nWidth, m_nHeight, .5f, 1, 1 + .5f / m_nWidth, 1 + .5f / m_nHeight },
							{ 0, m_nHeight, .5f, 1, 0 + .5f / m_nWidth, 1 + .5f / m_nHeight }
						};
						m_pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
						m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, axPlaneVertices, sizeof(PlaneVertex));

						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End ();
			}
		}
		m_pCurScene->Render(m_pD3DDevice, TRUE);

		// Render - 4 Pass
		/*
		pSprite->Begin (D3DXSPRITE_DONOTSAVESTATE);

		D3DXMATRIX mat1, mat2;
		D3DXMatrixIdentity (&mat1);
		D3DXMatrixScaling (&mat2, 0.2f, 0.2f, 1.0f);
		mat2 = mat1 * mat2;

		pSprite->SetTransform (&mat2);
		pSprite->Draw(pDiffuse, NULL, NULL, &D3DXVECTOR3(0, GetSetting()->BackBufferHeight, 0.0f), 0xFFFFFFFF);
		pSprite->Draw(pNormal, NULL, NULL, &D3DXVECTOR3(GetSetting()->BackBufferWidth, GetSetting()->BackBufferHeight, 0.0f), 0xFFFFFFFF);
		pSprite->Draw(pDepth, NULL, NULL, &D3DXVECTOR3(GetSetting()->BackBufferWidth * 2, GetSetting()->BackBufferHeight, 0.0f), 0xFFFFFFFF);
		pSprite->Draw(pXYMap, NULL, NULL, &D3DXVECTOR3(GetSetting()->BackBufferWidth * 3, GetSetting()->BackBufferHeight, 0.0f), 0xFFFFFFFF);

		pSprite->Draw (pLightMap, NULL, NULL, &D3DXVECTOR3 (0.0f, GetSetting ()->BackBufferHeight * 2, 0.0f), 0xFFFFFFFF);
		pSprite->Draw (pSpecularMap, NULL, NULL, &D3DXVECTOR3 (GetSetting ()->BackBufferWidth, GetSetting ()->BackBufferHeight * 2, 0.0f), 0xFFFFFFFF);
		pSprite->SetTransform (&mat1);

		pSprite->End ();
		*/
		m_pD3DDevice->EndScene ();

		//m_pHDRSystem->PostRender (m_pD3DDevice);

		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_FILLMODE, state);
	}

	//	APROJECT_WINDOW->GetHDRSystem ()->PostRender (m_pD3DDevice);
	//}
	if (m_pD3DDevice->Present (NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
	{
		CheckDevice ();
	}
}

void CAWindow::SetExitLoop (void)
{
	m_bEndStep = true;
}

CARootScene* CAWindow::GetCurScene (void)
{
	return m_pCurScene;
}

HRESULT CAWindow::SetCurrentScene (const WCHAR* pTitle)
{
	map<wstring, CARootScene*>::iterator itr = m_mapTable->find(pTitle);

	if (itr != m_mapTable->end())
	{
		if (m_pCurScene)
			m_pCurScene->Destroy ();

		m_pCurScene = itr->second;

		m_pCurScene->Create (m_pD3DDevice);
		m_pkMouse->SetCallbackSceneEvent (m_pCurScene);
		m_pkKeyboard->SetCallbackSceneEvent (m_pCurScene);
		
		return AE_SUCCESS;
	}

	return AE_ERROR_NOT_FIND_NAME;
}

bool CAWindow::CheckDevice (void)
{
	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 backbuffer = NULL;

	// Test the cooperative level to see if it's okay to render.
	switch (m_pD3DDevice->TestCooperativeLevel () )
	{
		// The device has been lost but cannot be reset at this time.
		// So wait until it can be reset.
		case D3DERR_DEVICELOST:
			return false;

		// Try to reset the device
		case D3DERR_DEVICENOTRESET:
			m_pCallbackDeviceDestoryed ();
			DebugConsoleLog (DebugChange, "CAWindow: Device Not Reset!");

			if (FAILED (m_pD3DDevice->Reset (&m_d3dpp) ) )
			{
				DebugLog ("Reset() 실패!");
				DebugConsoleLog (DebugError, "CAWindow: Reset() 실패!");

				Destroy ();
				CreateDirect3D (m_bForegroundMode, m_bMsgExclusiveMode, m_bNoWindowKey, !m_d3dpp.Windowed);

				return false;
			}

			m_pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			backbuffer->GetDesc (&desc);
			m_pCallbackDeviceReseted (m_pD3DDevice, &desc);

			m_pLightEffect->OnResetDevice ();
			//m_pHDRSystem->Reset (m_pD3DDevice, &desc);

			m_pCallbackDeviceCreated (m_pD3DDevice, &desc);

			DebugConsoleLog (DebugChange, "CAWindow: Reset Device!");

			return true;
		default:
			return true;
	}
}

HRESULT CAWindow::Initialize (bool bMultiThreadSystem, bool bEscapeToQuit, bool bPauseToToggleTimePause)
{
	if (m_bInitalize)
	{
		DebugLog ("Initialize (Already Initializing) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: Initialize (Already Initializing) 실패!");
		return AE_ERROR_ALREADY_INITIALIZE;
	}

	DebugConsoleLog (AE_MSG, ">== AProject System Start ==<");

	m_bInitalize = true;

	m_pD3D = NULL;
	m_pD3DDevice = NULL;

	m_bMultiThreadSystem = bMultiThreadSystem;
	m_bEscapeToQuit = bEscapeToQuit;
	m_bPauseToToggleTimePause = bPauseToToggleTimePause;

	m_hWindow = NULL;
	m_hInstance = NULL;

	m_hIcon = NULL;
	m_hMenu = NULL;

	m_bEndStep = false;

	if (m_bMultiThreadSystem)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Multi Threading Mode");
	else
		DebugConsoleLog (DebugSelect, "CAWindow: Select Single Threading Mode");
	if (m_bEscapeToQuit)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Escape Key To Quit Mode");
	if (m_bPauseToToggleTimePause)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Pause To Toggle Time Pause Mode");

	return AE_SUCCESS;
}

HRESULT CAWindow::CreateDefaultWindow (LPWSTR szTitle, AInt nWidth, AInt nHeight, AUInt32 nStyle)
{
	m_nStyle = nStyle;
	if (!m_bInitalize)
	{
		DebugLog ("CreateDefaultWindow (Not Initializing) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDefaultWindow (Not Initializing) 실패!");
		return AE_ERROR_NOT_INITIALIZE;
	}

	if (this->GetHWND () )
	{
		DebugLog ("CreateDefaultWindow (Already Create) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDefaultWindow (Already Create) 실패!");
		return AE_ERROR_ALREADY_HWND;
	}

	if (this->GetHinstance () == NULL)
		this->SetHinstance (NULL);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	WCHAR szExePath [MAX_PATH];
	GetModuleFileName (NULL, szExePath, MAX_PATH);
	if (m_hIcon == NULL)												// If the icon is NULL, then use the first one found in the exe
		m_hIcon = ExtractIcon (this->GetHinstance (), szExePath, 0);

	// Register the windows class
	WNDCLASS wndClass;
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = this->GetHinstance ();
	wndClass.hIcon = m_hIcon;
	wndClass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"AWindowsWindow";

	// Try to register class
	if (!RegisterClass (&wndClass) )
    {
		DebugLog ("CreateDefaultWindow (Register Window Class) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDefaultWindow (Register Window Class) 실패!");

		return AE_ERROR_REGISTER_WINDOW;
    }

	// Find the window's initial size, but it might be changed later
	RECT rc;
	SetRect (&rc, 0, 0, nWidth, nHeight);
	AdjustWindowRect (&rc, nStyle, (this->GetApplicationMenu () != NULL) ? true : false);

	// Create the render window
	m_hWindow = CreateWindow (wndClass.lpszClassName, szTitle, nStyle,
							CW_USEDEFAULT, CW_USEDEFAULT,
							(rc.right - rc.left), (rc.bottom - rc.top), 
							NULL, this->GetApplicationMenu (), this->GetHinstance (), NULL);

	if (!m_hWindow)
	{
		DebugLog ("CreateDefaultWindow (Create Window) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDefaultWindow (Create Window) 실패!");

		return AE_ERROR_CREATEWINDOW;
	}

	m_bEndStep = false;

	DebugConsoleLog (DebugClear, "CAWindow: Create Window (%d)", m_hWindow);

	return AE_SUCCESS;
}

HRESULT CAWindow::ResetDirect3D (bool bFullScreen)
{
	HRESULT hr;

	if (NULL == (m_pD3D = Direct3DCreate9 (D3D_SDK_VERSION) ) )
	{
		DebugLog ("ResetDirect3D (Failed to create Direct3D) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: ResetDirect3D (Failed to create Direct3D) 실패!");

		return AE_ERROR_DIRECT3D_CREATE;
	}

	D3DCAPS9 caps;
	m_pD3D->GetDeviceCaps (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	D3DDISPLAYMODE d3ddm;
	if (FAILED (m_pD3D->GetAdapterDisplayMode (D3DADAPTER_DEFAULT, &d3ddm) ) )
	{
		DebugLog ("CreateDirect3D (Display Mode Get failed to...) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Display Mode Get failed to...) 실패!");

		return AE_ERROR_GET_DISPLAYMODE;
	}

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		DebugConsoleLog (DebugSelect, "CAWindow: Select Hardware Vertexprocessing");
		
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		DebugConsoleLog (DebugSelect, "CAWindow: Select Software Vertexprocessing");

		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	ZeroMemory (&m_d3dpp, sizeof (m_d3dpp) );
	m_d3dpp.Windowed = !bFullScreen;

	m_d3dpp.BackBufferWidth            = m_nWidth;
	m_d3dpp.BackBufferHeight           = m_nHeight;
	m_d3dpp.BackBufferFormat           = d3ddm.Format;							// 현재 바탕화면 모드에 맞춰서 후면버퍼를 생성
	m_d3dpp.BackBufferCount            = 1;
	m_d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	m_d3dpp.MultiSampleQuality         = 0;
	m_d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;					// 가장 효율적인 SWAP효과
	m_d3dpp.hDeviceWindow              = m_hWindow;
	m_d3dpp.EnableAutoDepthStencil     = TRUE;
	m_d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
	m_d3dpp.Flags                      = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	m_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	m_d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;			// 자동으로 그려주는 간격을 조정

	hr = m_pD3D->CreateDevice (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWindow, vp | D3DCREATE_MULTITHREADED, &m_d3dpp, &m_pD3DDevice); // 정밀한 연산을 하려면 D3DCREATE_FPU_PRESERVE 추가

	if (FAILED (hr) )
	{
		DebugLog ("CreateDirect3D (Failed to create device) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Failed to create device) 실패!");

		return AE_ERROR_FAILED_CREATEDEVICE;
	}

	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 backbuffer = NULL;
	m_pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	backbuffer->GetDesc (&desc);
	m_pCallbackDeviceCreated (m_pD3DDevice, &desc);

	//m_pHDRSystem->Reset (m_pD3DDevice, &desc);
	m_pLightEffect->OnResetDevice();

	{
		SAFE_RELEASE(m_pDiffuse);
		SAFE_RELEASE(m_pNormal);
		SAFE_RELEASE(m_pDepth);
		SAFE_RELEASE(m_pXYMap);
		SAFE_RELEASE(m_pLightMap);
		SAFE_RELEASE(m_pSpecularMap);
		SAFE_RELEASE(m_pSprite);
		SAFE_RELEASE(m_pSphereMesh);

		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pDiffuse);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pNormal);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_G16R16F, D3DPOOL_DEFAULT, &m_pDepth);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pXYMap);

		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pLightMap);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pSpecularMap);
		D3DXCreateSprite(m_pD3DDevice, &m_pSprite);

		D3DXCreateSphere(m_pD3DDevice, 1.0f, 20, 20, &m_pSphereMesh, NULL);
	}

	SetWindowLong (m_hWindow, GWL_STYLE, m_nStyle);
	SetWindowPos (m_hWindow, HWND_NOTOPMOST, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | ((!bFullScreen) ? SWP_DRAWFRAME : 0) );

	ShowWindow (m_hWindow, SW_SHOW);
	UpdateWindow (m_hWindow);
	
	DebugConsoleLog (DebugClear, "CAWindow: Direct3D Setup success (%d)", m_pD3DDevice);

	return AE_SUCCESS;
}

HRESULT CAWindow::CreateDirect3D (bool bForegroundMode, bool bMsgExclusiveMode, bool bNoWindowKey, bool bFullScreen)
{
	HRESULT hr;

	if (!m_bInitalize)
	{
		DebugLog ("CreateDirect3D (Not Initializing) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Not Initializing) 실패!");
		return AE_ERROR_NOT_INITIALIZE;
	}

	// Not allowed to call this from inside the device callbacks
    if (m_pCallbackDeviceCreated == NULL || m_pCallbackDeviceReseted == NULL || m_pCallbackDeviceDestoryed == NULL)
	{
		DebugLog ("CreateDirect3D (Device Callbacks are not set) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Device Callbacks are not set) 실패!");

		return AE_ERROR_NOTSET_DEVICECALLBACKS;
	}

	// If CreateDefaultWindow() has not already been called, 
    // then call CreateDefaultWindow() with the default parameters.
	if (!this->GetHWND () )
	{
		DebugLog ("CreateDirect3D (Not Already HWND) 실패!");
		DebugConsoleLog (DebugChange, "CAWindow: CreateDirect3D (Not Already HWND) 실패!");

		// If CreateDefaultWindow() hasn't been called, then 
        // automatically call CreateDefaultWindow() with default params
		if (hr = CreateDefaultWindow () )
			return hr;
	}

	if (m_pD3D != NULL || m_pD3DDevice != NULL)
	{
		DebugLog ("CreateDirect3D (Already Direct3D) 실패!");
		DebugConsoleLog (DebugChange, "CAWindow: CreateDirect3D (Already Direct3D) 실패!");

		return AE_ERROR_ALREADY_DIRECT3D;
	}

	m_bForegroundMode = bForegroundMode;
	m_bMsgExclusiveMode = bMsgExclusiveMode;
	m_bNoWindowKey = bNoWindowKey;

	m_pkInput = new CAInput (this->GetHinstance () );
	m_pkMouse = new CAInputMouse (this->GetHWND (), m_bForegroundMode, m_bMsgExclusiveMode);
	m_pkKeyboard = new CAInputKeyboard (this->GetHWND (), m_bForegroundMode, m_bMsgExclusiveMode, m_bNoWindowKey);

	if (m_bForegroundMode)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Forground Input Mode");
	else
		DebugConsoleLog (DebugSelect, "CAWindow: Select Background Input Mode");
	if (m_bMsgExclusiveMode)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Exclusive Message Mode");
	else
		DebugConsoleLog (DebugSelect, "CAWindow: Select Non Exclusive Message Mode");
	if (m_bNoWindowKey)
		DebugConsoleLog (DebugSelect, "CAWindow: Select Non Window Key Mode");

	if (NULL == (m_pD3D = Direct3DCreate9 (D3D_SDK_VERSION) ) )
	{
		DebugLog ("CreateDirect3D (Failed to create Direct3D) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Failed to create Direct3D) 실패!");

		return AE_ERROR_DIRECT3D_CREATE;
	}

	D3DCAPS9 caps;
	m_pD3D->GetDeviceCaps (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	D3DDISPLAYMODE d3ddm;
	if (FAILED (m_pD3D->GetAdapterDisplayMode (D3DADAPTER_DEFAULT, &d3ddm) ) )
	{
		DebugLog ("CreateDirect3D (Display Mode Get failed to...) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Display Mode Get failed to...) 실패!");

		return AE_ERROR_GET_DISPLAYMODE;
	}

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT && caps.VertexShaderVersion >= D3DVS_VERSION (2, 0))
	{
		DebugConsoleLog (DebugSelect, "CAWindow: Select Hardware Vertexprocessing");

		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		DebugConsoleLog (DebugSelect, "CAWindow: Select Software Vertexprocessing");

		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	ZeroMemory (&m_d3dpp, sizeof (m_d3dpp) );
	m_d3dpp.Windowed = !bFullScreen;

	m_d3dpp.BackBufferWidth            = m_nWidth;
	m_d3dpp.BackBufferHeight           = m_nHeight;
	m_d3dpp.BackBufferFormat           = d3ddm.Format;							// 현재 바탕화면 모드에 맞춰서 후면버퍼를 생성
	m_d3dpp.BackBufferCount            = 1;
	m_d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	m_d3dpp.MultiSampleQuality         = 0;
	m_d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;					// 가장 효율적인 SWAP효과
	m_d3dpp.hDeviceWindow              = m_hWindow;
	m_d3dpp.EnableAutoDepthStencil     = TRUE;
	m_d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
	m_d3dpp.Flags                      = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	m_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	m_d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;			// 자동으로 그려주는 간격을 조정

	hr = m_pD3D->CreateDevice (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWindow, vp | D3DCREATE_MULTITHREADED, &m_d3dpp, &m_pD3DDevice); // 정밀한 연산을 하려면 D3DCREATE_FPU_PRESERVE 추가

	if (FAILED (hr) )
	{
		DebugLog ("CreateDirect3D (Failed to create device) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: CreateDirect3D (Failed to create device) 실패!");

		return AE_ERROR_FAILED_CREATEDEVICE;
	}

	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 backbuffer = NULL;
	m_pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	backbuffer->GetDesc (&desc);

	ID3DXBuffer *pCompilationErrors = 0;
	hr = D3DXCreateEffectFromFileW (m_pD3DDevice, L"./Deferred Rendering.fx", nullptr, nullptr,
		D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER, nullptr, &m_pLightEffect, &pCompilationErrors);
	if (FAILED (hr) )
    {
        if (pCompilationErrors)
        {
            std::string compilationErrors (static_cast<const char *>(pCompilationErrors->GetBufferPointer () ) );

			//DebugConsoleLog (DebugError, "Shader Load Error: %s", compilationErrors);
			printf ("Shader Load Error: %s", compilationErrors.c_str());

            pCompilationErrors->Release ();
            throw std::runtime_error (compilationErrors);
			return 0;
        }
    }
    if (pCompilationErrors)
        pCompilationErrors->Release ();

	m_pCallbackDeviceCreated(m_pD3DDevice, &desc);

	//m_pHDRSystem = new CAHDRLighting ();
	//m_pHDRSystem->CheckDevice (&caps, m_d3dpp.BackBufferFormat, m_d3dpp.BackBufferFormat, m_d3dpp.Windowed);
	//m_pHDRSystem->Create (m_pD3DDevice, &desc);
	//m_pHDRSystem->Reset (m_pD3DDevice, &desc);
	{
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pDiffuse);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pNormal);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_G16R16F, D3DPOOL_DEFAULT, &m_pDepth);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pXYMap);

		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pLightMap);
		D3DXCreateTexture(m_pD3DDevice, m_nWidth, m_nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pSpecularMap);
		D3DXCreateSprite(m_pD3DDevice, &m_pSprite);

		D3DXCreateSphere(m_pD3DDevice, 1.0f, 20, 20, &m_pSphereMesh, NULL);
	}

	SAFE_RELEASE (backbuffer);

	ShowWindow (m_hWindow, SW_SHOW);
	UpdateWindow (m_hWindow);
	
	DebugConsoleLog (DebugClear, "CAWindow: Direct3D Setup success (%d)", m_pD3DDevice);

	return AE_SUCCESS;
}

HRESULT CAWindow::RunLoop (void)
{
	HRESULT hr;

	// Not allowed to call this from inside the device callbacks or reenter
    if (m_pCallbackDeviceCreated == NULL || m_pCallbackDeviceReseted == NULL || m_pCallbackDeviceDestoryed == NULL)
    {
		DebugLog ("RunLoop (Device Callbacks are not set) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: RunLoop (Device Callbacks are not set) 실패!");

		return AE_ERROR_NOTSET_DEVICECALLBACKS;
    }

	// If CreateDefaultWindow() has not already been called, 
    // then call CreateDefaultWindow() with the default parameters.
	if (!this->GetHWND () )
	{
		DebugLog ("RunLoop (Not Already HWND) 실패!");
		DebugConsoleLog (DebugChange, "CAWindow: RunLoop (Not Already HWND) 실패!");

		// If CreateDefaultWindow() hasn't been called, then 
        // automatically call CreateDefaultWindow() with default params
		if (hr = CreateDefaultWindow () )
			return hr;
	}

	// If CreateDirect3D() has not already been called, 
    // then call CreateDirect3D() with the default parameters.
	if (!this->GetD3DDevice () )
	{
		DebugLog ("RunLoop (Not Already Direct3D Device) 실패!");
		DebugConsoleLog (DebugChange, "CAWindow: RunLoop (Not Already Direct3D Device) 실패!");

		// If CreateDirect3D() hasn't been called, then 
        // automatically call CreateDirect3D() with default params
		if (hr = CreateDirect3D () )
			return hr;
	}

    HWND hWnd = this->GetHWND ();

	// Initialize() must have been called and succeeded for this function to proceed
    // CreateDefaultWindow() must have been called and succeeded for this function to proceed
    // CreateDirect3D() must have been called and succeeded for this function to proceed
	if (!m_bInitalize || !this->GetHWND () || !this->GetD3DDevice () )
	{
		DebugLog ("RunLoop (Not Initializing) 실패!");
		DebugConsoleLog (DebugError, "CAWindow: RunLoop (Not Initializing) 실패!");

		return AE_ERROR_NOT_INITIALIZE;
	}

	DWORD IngThreadID;
	HANDLE hThread = NULL;
	if (m_bMultiThreadSystem)
		hThread = CreateThread (NULL, 0, Update, &m_bEndStep, 0, &IngThreadID);

	// Now we're ready to receive and process Windows messages.
	MSG msg;
	PeekMessage (&msg, hWnd, 0U, 0U, PM_NOREMOVE);

	while (!m_bEndStep && WM_QUIT != msg.message)
	{
		Sleep(1);

		if (m_pkKeyboard->IsButtonDown (DIK_LMENU) && m_pkKeyboard->IsButtonDown (DIK_RETURN) )
		{
			SuspendThread (hThread);
			//m_pD3DDevice->Reset(&m_d3dpp);
			m_pD3DDevice->Release ();
			m_pD3D->Release();
			SAFE_RELEASE (m_pD3D);
			ResetDirect3D (m_d3dpp.Windowed);
			m_pCurScene->Reset (m_pD3DDevice);
			ResumeThread (hThread);
		}

		// Escape Key Down is Quit Direct3D
		if (m_bEscapeToQuit)
			if (m_pkKeyboard->IsButtonDown (DIK_ESCAPE) )
				SetExitLoop ();

		// Use PeekMessage() so we can use idle time to render the scene.
		if (PeekMessage (&msg, hWnd, 0U, 0U, PM_REMOVE) )
        {
            // Translate and dispatch the message
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
        else
        {
            // Render a frame during idle time (no messages are waiting)
            if (CheckDevice () )
			{
				m_dLastTime = m_dCurTime;

				//__try
				//{
					if (hThread == NULL)
						Update (&m_bEndStep);

					Render ();
				//}
				//__except (CrashPrinting (GetExceptionInformation () ) )
				//{ 
				//	SetExitLoop ();
				//}

				m_dCurTime = timeGetTime ();
				CheckFPS ();
			}
			else
				WaitMessage ();
        }
	}

	DestroyWindow (m_hWindow);

	return AE_SUCCESS;
}

DWORD CAWindow::CrashPrinting (LPEXCEPTION_POINTERS lpException)
{
	DebugConsoleLog (DebugError, "Exception : 0x%08X", lpException->ExceptionRecord->ExceptionCode);
	DebugConsoleLog (DebugError, "Exception Address : 0x%08p", lpException->ExceptionRecord->ExceptionAddress);

	return EXCEPTION_EXECUTE_HANDLER;
}

void CAWindow::CheckFPS (void)
{
	const double reflash = 500.0;
	static DWORD dFrame = 0;
	static DWORD dSumTime = 0;
	static DWORD dLastTime = timeGetTime ();
	DWORD dCurTime = timeGetTime ();

	dFrame ++;
	dSumTime += (dCurTime - dLastTime);

	if (dSumTime > reflash)
	{
		m_fFPS = (float)( (double)dFrame * (reflash / (double)dSumTime) * (1000.0 / reflash) );
		dFrame = 0;
		dSumTime = 0;
	}

	dLastTime = dCurTime;
}

CARootScene* CAWindow::Trans (const WCHAR* pTitle)
{
	map<wstring, CARootScene*>::iterator itr = m_mapTable->find (pTitle);

	if (itr != m_mapTable->end () )
		return itr->second;

	return NULL;
}

HRESULT CAWindow::Commit (const WCHAR* pTitle, CARootScene* pObject)
{
	map<wstring, CARootScene*>::iterator itr = m_mapTable->find (pTitle);

	if (itr == m_mapTable->end () )
	{
		m_mapTable->insert(map<wstring, CARootScene*>::value_type(pTitle, pObject));

		if (m_pCurScene == NULL)
			SetCurrentScene(pTitle);

		return AE_SUCCESS;
	}

	SAFE_DELETE (pObject);
	return AE_ERROR_EQUAL_NAME;
}

HRESULT CAWindow::Destory (const WCHAR* pTitle)
{
	map<wstring, CARootScene*>::iterator itr = m_mapTable->find(pTitle);

	if (itr != m_mapTable->end())
	{
		SAFE_DELETE (itr->second);
		m_mapTable->erase (itr);

		return AE_SUCCESS;
	}

	return AE_ERROR_NOT_FIND_NAME;
}