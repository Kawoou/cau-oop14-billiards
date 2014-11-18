#include "AWindow.h"

CATexture::CATexture (D3DPOOL dwPoolingType)
	:CAObject ()
{
	m_pTexture = NULL;
	m_pVertexBuffer = NULL;

	m_nTextureWidthSize = 0;
	m_nTextureHeightSize = 0;
	m_dwColorkey = 0x00000000;
	m_pTextureFilepath = NULL;

	m_dwPoolingType = dwPoolingType;

	m_bOrtho2D = false;
	m_bBlend = false;
	m_bEnable = true;
}

CATexture::~CATexture ()
{
	SAFE_RELEASE (m_pTexture);
	SAFE_RELEASE (m_pVertexBuffer);
	SAFE_DELETE (m_pTextureFilepath);

	DebugConsoleLog (DebugClear, "CATexture: Texture Data Destroy!");
}

void CATexture::SetTexture (LPDIRECT3DTEXTURE9 pTexture, AUInt Width, AUInt Height)
{
	m_nTextureWidthSize = Width;
	m_nTextureHeightSize = Height;
	m_dwColorkey = 0x00000000;
	SAFE_DELETE(m_pTextureFilepath);

	m_pTexture = pTexture;

	SAFE_RELEASE (m_pVertexBuffer);
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (4 * sizeof (Texture2DVertex), D3DUSAGE_WRITEONLY,
												D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL) ) )
	{
		DebugLog ("SetTexture(Create vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CATexture: SetTexture(Create vertex buffer Error) 실패!");
		return;
	}
	Texture2DVertex* pVertices;
	Texture2DVertex vertices [] = {
		{m_vPosition.x,							m_vPosition.y,							0.0f, 1.0f, 0.0f, 0.0f},
		{m_vPosition.x + m_nTextureWidthSize,	m_vPosition.y,							0.0f, 1.0f, 1.0f, 0.0f},
		{m_vPosition.x,							m_vPosition.y + m_nTextureHeightSize,	0.0f, 1.0f, 0.0f, 1.0f},
		{m_vPosition.x + m_nTextureWidthSize,	m_vPosition.y + m_nTextureHeightSize,	0.0f, 1.0f, 1.0f, 1.0f},
	};
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0) ) )
	{
		DebugLog ("SetTexture(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CATexture: SetTexture(Lock vertex buffer Error) 실패!");
		return;
	}
	memcpy (pVertices, vertices, sizeof (*vertices) * 4);
	m_pVertexBuffer->Unlock();
}

void CATexture::SetTexture (const char* pTextureFilepath, AUInt Width, AUInt Height, AUInt32 dwColorKey)
{
	SAFE_RELEASE (m_pTexture);
	SAFE_RELEASE (m_pVertexBuffer);
	SAFE_DELETE (m_pTextureFilepath);

	m_nTextureWidthSize = Width;
	m_nTextureHeightSize = Height;
	m_dwColorkey = dwColorKey;
	m_pTextureFilepath = new char[strlen(pTextureFilepath) + 1];
	strcpy(m_pTextureFilepath, pTextureFilepath);

	if (FAILED (D3DXCreateTextureFromFileExA (APROJECT_WINDOW->GetD3DDevice (), m_pTextureFilepath, Width, Height,
		D3DX_DEFAULT, NULL, D3DFMT_UNKNOWN, m_dwPoolingType, D3DX_DEFAULT, D3DX_DEFAULT, dwColorKey, NULL, NULL, &m_pTexture) ) )
	{
		DebugLog ("SetTexture(Could not find %s) 실패!", pTextureFilepath);
		DebugConsoleLog (DebugError, "CATexture: SetTexture(Could not find %s) 실패!", pTextureFilepath);
		return;
	}
	DebugConsoleLog (DebugClear, "CATexture: Create Texture File, the success");
	m_pTexture->GetLevelDesc (0, &m_descSurface);

	SAFE_RELEASE (m_pVertexBuffer);
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (4 * sizeof (Texture2DVertex), D3DUSAGE_WRITEONLY,
												D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL) ) )
	{
		DebugLog ("SetTexture(Create vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CATexture: SetTexture(Create vertex buffer Error) 실패!");
		return;
	}
	Texture2DVertex* pVertices;
	Texture2DVertex vertices [] = {
		{m_vPosition.x,							m_vPosition.y,							0.0f, 1.0f, 0.0f, 0.0f},
		{m_vPosition.x + m_descSurface.Width,	m_vPosition.y,							0.0f, 1.0f, 1.0f, 0.0f},
		{m_vPosition.x,							m_vPosition.y + m_descSurface.Height,	0.0f, 1.0f, 0.0f, 1.0f},
		{m_vPosition.x + m_descSurface.Width,	m_vPosition.y + m_descSurface.Height,	0.0f, 1.0f, 1.0f, 1.0f},
	};
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0) ) )
	{
		DebugLog ("SetTexture(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CATexture: SetTexture(Lock vertex buffer Error) 실패!");
		return;
	}
	memcpy (pVertices, vertices, sizeof (*vertices) * 4);
	m_pVertexBuffer->Unlock();
}

void CATexture::SetDrawFlag (MultiTextureFlag mtFlag)
{
	m_mtFlag = mtFlag;
}

void CATexture::SetColor (AColor cColor)
{
	m_cColor = cColor;
}

void CATexture::SetBlend (bool bState)
{
	m_bBlend = bState;
}

LPDIRECT3DTEXTURE9 CATexture::GetTexture (void)
{
	return m_pTexture; 
}

MultiTextureFlag CATexture::GetDrawFlag (void)
{
	return m_mtFlag;
}

AColor CATexture::GetColor (void)
{
	return m_cColor;
}

bool CATexture::GetBlend (void)
{
	return m_bBlend;
}

AInt CATexture::GetTextureWidthSize (void)
{
	return m_descSurface.Width;
}

AInt CATexture::GetTextureWidthHalfSize (void)
{
	return m_descSurface.Width / 2;
}

AInt CATexture::GetTextureHeightSize (void)
{
	return m_descSurface.Height;
}

AInt CATexture::GetTextureHeightHalfSize (void)
{
	return m_descSurface.Height / 2;
}

D3DPOOL CATexture::GetPoolingType (void)
{
	return m_dwPoolingType;
}

void CATexture::Reset (void)
{
	SAFE_RELEASE (m_pTexture);

	if (m_pTextureFilepath)
	{
		if (FAILED (D3DXCreateTextureFromFileExA (APROJECT_WINDOW->GetD3DDevice (), m_pTextureFilepath, GetTextureWidthSize (), GetTextureHeightSize (),
			D3DX_DEFAULT, NULL, D3DFMT_UNKNOWN, m_dwPoolingType, D3DX_FILTER_TRIANGLE ,D3DX_FILTER_TRIANGLE, m_dwColorkey, NULL, NULL, &m_pTexture) ) )
		{
			DebugLog ("Reset(Could not find %s) 실패!", m_pTextureFilepath);
			DebugConsoleLog (DebugError, "CATexture: Reset(Could not find %s) 실패!", m_pTextureFilepath);
		}
		m_pTexture->GetLevelDesc (0, &m_descSurface);

		SAFE_RELEASE (m_pVertexBuffer);
		if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (4 * sizeof (Texture2DVertex), D3DUSAGE_WRITEONLY,
													D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL) ) )
		{
			DebugLog ("Reset(Create vertex buffer Error) 실패!");
			DebugConsoleLog (DebugError, "CATexture: Reset(Create vertex buffer Error) 실패!");
			return;
		}
		Texture2DVertex* pVertices;
		Texture2DVertex vertices [] = {
			{m_vPosition.x,							m_vPosition.y,							m_vPosition.z, 1.0f, 0.0f, 0.0f},
			{m_vPosition.x + m_descSurface.Width,	m_vPosition.y,							m_vPosition.z, 1.0f, 1.0f, 0.0f},
			{m_vPosition.x,							m_vPosition.y + m_descSurface.Height,	m_vPosition.z, 1.0f, 0.0f, 1.0f},
			{m_vPosition.x + m_descSurface.Width,	m_vPosition.y + m_descSurface.Height,	m_vPosition.z, 1.0f, 1.0f, 1.0f},
		};
		if (FAILED (m_pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0) ) )
		{
			DebugLog ("Reset(Lock vertex buffer Error) 실패!");
			DebugConsoleLog (DebugError, "CATexture: Reset(Lock vertex buffer Error) 실패!");
			return;
		}
		memcpy (pVertices, vertices, sizeof (*vertices) * 4);
		m_pVertexBuffer->Unlock();
	}
	DebugConsoleLog(DebugClear, "CATexture: Create Texture File, the success");
}

void CATexture::Update (void)
{
	Texture2DVertex* pVertices;

	float rx = m_descSurface.Width / 2;
	float ry = m_descSurface.Height / 2;

	float fLTx = cos (m_fRotZ) * (-rx) - sin (m_fRotZ) * (-ry);
	float fLTy = sin (m_fRotZ) * (-rx) + cos (m_fRotZ) * (-ry);
	float fRTx = cos (m_fRotZ) * (rx) - sin (m_fRotZ) * (-ry);
	float fRTy = sin (m_fRotZ) * (rx) + cos (m_fRotZ) * (-ry);
	float fLDx = cos (m_fRotZ) * (-rx) - sin (m_fRotZ) * (ry);
	float fLDy = sin (m_fRotZ) * (-rx) + cos (m_fRotZ) * (ry);
	float fRDx = cos (m_fRotZ) * (rx) - sin (m_fRotZ) * (ry);
	float fRDy = sin (m_fRotZ) * (rx) + cos (m_fRotZ) * (ry);

	Texture2DVertex vertices [] = {
		{ m_vPosition.x + fLTx * m_vSize.x, m_vPosition.y + fLTy * m_vSize.y, m_vPosition.z, 1.0f, 0.0f, 0.0f },
		{ m_vPosition.x + fRTx * m_vSize.x, m_vPosition.y + fRTy * m_vSize.y, m_vPosition.z, 1.0f, 1.0f, 0.0f },
		{ m_vPosition.x + fLDx * m_vSize.x, m_vPosition.y + fLDy * m_vSize.y, m_vPosition.z, 1.0f, 0.0f, 1.0f },
		{ m_vPosition.x + fRDx * m_vSize.x, m_vPosition.y + fRDy * m_vSize.y, m_vPosition.z, 1.0f, 1.0f, 1.0f },
	};
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0) ) )
	{
		DebugLog ("Update(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CATexture: Update(Lock vertex buffer Error) 실패!");
		return;
	}
	memcpy (pVertices, vertices, sizeof (*vertices) * 4);
	m_pVertexBuffer->Unlock();

	CAObject::Update ();
}

void CATexture::Render (void)
{
	if (!m_bEnable)
		return;

	D3DXMATRIX matWorld;
	D3DXMatrixScaling(&matWorld, m_vSize.x, m_vSize.y, m_vSize.z);
	APROJECT_WINDOW->GetLightEffect()->SetMatrix("World", &matWorld);
	APROJECT_WINDOW->GetLightEffect()->SetMatrix("View", APROJECT_WINDOW->GetCurScene()->GetCamera()->GetViewMatrix());
	APROJECT_WINDOW->GetLightEffect()->SetMatrix("Projection", APROJECT_WINDOW->GetCurScene()->GetCamera()->GetProjectionMatrix());
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		if (m_pTexture)
			hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("O2D_Dif");
		else
			hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("O2D_None");

		stMaterial material = {
			D3DXCOLOR(1.0f),
			D3DXCOLOR(m_cColor.r, m_cColor.g, m_cColor.b, m_cColor.a),
			D3DXCOLOR(1.0f),
			D3DXCOLOR(1.0f),
			1.0f
		};
		APROJECT_WINDOW->GetLightEffect()->SetValue("g_mMaterial", &material, sizeof (material));
		APROJECT_WINDOW->GetLightEffect()->SetTexture("Texture1", m_pTexture);
		
		APROJECT_WINDOW->GetD3DDevice()->SetStreamSource(m_nLayer, m_pVertexBuffer, 0, sizeof (Texture2DVertex));
		APROJECT_WINDOW->GetD3DDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

		if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->SetTechnique(hTechnique)))
		{
			if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->Begin(&totalPasses, 0)))
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->BeginPass(pass)))
					{
						APROJECT_WINDOW->GetD3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

						APROJECT_WINDOW->GetLightEffect()->EndPass();
					}
				}
				APROJECT_WINDOW->GetLightEffect()->End();
			}
		}

		APROJECT_WINDOW->GetLightEffect()->SetTexture("Texture1", NULL);
	}
	/*
	DWORD dwAlphaBlendEnable;
	//DWORD dwSrcBlend;
	//DWORD dwDestBlend;
	
	DWORD dwTexcoordIndex;
	DWORD dwTssColorOP;
	DWORD dwTssColorArg1;
	DWORD dwTssColorArg2;
	DWORD dwTssAlphaOP;
	DWORD dwTssAlphaArg1;
	DWORD dwTssAlphaArg2;

	DWORD dwColor;
	DWORD dwZEnable;
	DWORD dwZWriteEnable;

	if (m_bEnable)
	{
		/if (!m_bOrtho2D)
		{
			// Original Data Load
			APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_TEXTUREFACTOR, &dwColor);

			// Set New Data
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_TEXTUREFACTOR, m_cColor.GetD3DColor () );

			// Draw Texture
			APROJECT_WINDOW->GetD3DDevice ()->SetTexture (m_nLayer, m_pTexture);

			// Restore Original Data
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_TEXTUREFACTOR, dwColor);
		}
		else/
		{
			// Original Data Load
			APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ALPHABLENDENABLE, &dwAlphaBlendEnable);
			//APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_SRCBLEND, &dwSrcBlend);
			//APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_DESTBLEND, &dwDestBlend);

			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_TEXCOORDINDEX, &dwTexcoordIndex);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_COLOROP, &dwTssColorOP);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_COLORARG1, &dwTssColorArg1);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_ALPHAARG2, &dwTssColorArg2);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_ALPHAOP, &dwTssAlphaOP);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_ALPHAARG1, &dwTssAlphaArg1);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (m_nLayer, D3DTSS_ALPHAARG2, &dwTssAlphaArg2);

			APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_TEXTUREFACTOR, &dwColor);
			APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ZENABLE, &dwZEnable);
			APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ZWRITEENABLE, &dwZWriteEnable);

			// Set New Data
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_TEXCOORDINDEX, m_mtFlag.m_dwTexcoordIndex);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLOROP, m_mtFlag.m_dwColorOP);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLORARG1, m_mtFlag.m_dwColorArg1);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLORARG2, m_mtFlag.m_dwColorArg2);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAOP, m_mtFlag.m_dwAlphaOP);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAARG1, m_mtFlag.m_dwAlphaArg1);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAARG2, m_mtFlag.m_dwAlphaArg2);

			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_TEXTUREFACTOR, m_cColor.GetD3DColor () );

			if (m_bOrtho2D)
			{
				APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ZENABLE, FALSE);
				APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);
			}

			// Draw Texture + Geometry
			//APROJECT_WINDOW->GetD3DDevice ()->SetTexture (m_nLayer, m_pTexture);
			if (m_bOrtho2D)
			{
				//UINT totalPasses;
				//D3DXHANDLE hTechnique;
				//hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("NoMapping");

				APROJECT_WINDOW->GetD3DDevice ()->SetTexture (m_nLayer, m_pTexture);

				APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (m_nLayer, m_pVertexBuffer, 0, sizeof (Texture2DVertex) );
				APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZRHW | D3DFVF_TEX1);

				//if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
				//{
				//	if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
				//	{
				//		for (UINT pass = 0; pass < totalPasses; ++pass)
				//		{
				//			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
				//			{
								APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_TRIANGLESTRIP, 0, 2);
				//				APROJECT_WINDOW->GetLightEffect ()->EndPass ();
				//			}
				//		}
				//		APROJECT_WINDOW->GetLightEffect ()->End();
				//	}
				//}
				APROJECT_WINDOW->GetD3DDevice ()->SetTexture (m_nLayer, NULL);
			}
			else
				APROJECT_WINDOW->GetLightEffect ()->SetTexture ("colorMapTexture", m_pTexture);

			// Restore Original Data
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable);
			//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, dwSrcBlend);
			//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, dwDestBlend);

			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_TEXCOORDINDEX, dwTexcoordIndex);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLOROP, dwTssColorOP);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLORARG1, dwTssColorArg1);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_COLORARG2, dwTssColorArg2);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAOP, dwTssAlphaOP);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAARG1, dwTssAlphaArg1);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (m_nLayer, D3DTSS_ALPHAARG2, dwTssAlphaArg2);

			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_TEXTUREFACTOR, dwColor);
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ZENABLE, dwZEnable);
			APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ZWRITEENABLE, dwZWriteEnable);
		}
	}
	CAObject::Render ();
	*/
}