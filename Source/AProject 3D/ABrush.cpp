#include "AMapInclude.h"

void CABrush::DeepCopy (const CABrush& brush)
{
	Release ();

	LPDIRECT3DDEVICE9		pD3DDevice	= NULL;
	D3DVERTEXBUFFER_DESC	d3dVertexBufferDesc;

	ZeroMemory (&d3dVertexBufferDesc, sizeof (D3DVERTEXBUFFER_DESC) );

	brush.m_pVB->GetDevice (&pD3DDevice);
	brush.m_pVB->GetDesc (&d3dVertexBufferDesc);

	if (FAILED (pD3DDevice->CreateVertexBuffer (
			d3dVertexBufferDesc.Size,
			d3dVertexBufferDesc.Usage,
			d3dVertexBufferDesc.FVF,
			d3dVertexBufferDesc.Pool,
			&m_pVB,
			NULL) ) )
	{
		return;
	}

	void* pVerticesSrc	= NULL;
	void* pVerticesDest	= NULL;
	brush.m_pVB->Lock (0, 0, (void**)&pVerticesSrc, 0);
	m_pVB->Lock (0, 0, (void**)&pVerticesDest, 0);

	memcpy (pVerticesDest, pVerticesSrc, d3dVertexBufferDesc.Size);

	m_pVB->Unlock ();
	brush.m_pVB->Unlock ();

	D3DINDEXBUFFER_DESC	d3dIndexBufferDesc;
	ZeroMemory (&d3dIndexBufferDesc, sizeof (D3DINDEXBUFFER_DESC) );
	brush.m_pIB->GetDesc (&d3dIndexBufferDesc);

	if (FAILED (pD3DDevice->CreateIndexBuffer (
			d3dIndexBufferDesc.Size,
			d3dIndexBufferDesc.Usage,
			d3dIndexBufferDesc.Format,
			d3dIndexBufferDesc.Pool,
			&m_pIB,
			NULL) ) )
	{
		return;
	}

	void* pIndicesSrc	= NULL;
	void* pIndicesDest	= NULL;
	brush.m_pIB->Lock (0, 0, (void**)&pIndicesSrc, 0);
	m_pIB->Lock (0, 0, (void**)&pIndicesDest, 0);

	memcpy (pIndicesDest, pIndicesSrc, d3dIndexBufferDesc.Size);

	m_pIB->Unlock ();
	brush.m_pIB->Unlock ();

	if (FAILED (D3DXCreateTextureFromFileA (
			pD3DDevice,
			brush.m_szDiffuse,
			&m_pTexture) ) )
	{
		return;
	}

	m_matWorld		= brush.m_matWorld;
	m_vSize.x		= brush.m_vSize.x;
	m_uCells		= brush.m_uCells;
	m_uIndices		= brush.m_uIndices;
	m_uVertices		= brush.m_uVertices;
	m_fIntensity	= brush.m_fIntensity;

	strcpy (m_szDiffuse, brush.m_szDiffuse);
}

void CABrush::Release ()
{
	SAFE_RELEASE (m_pVB);
	SAFE_RELEASE (m_pIB);
	SAFE_RELEASE (m_pTexture);
}

HRESULT	CABrush::InitVertexBuffer()
{
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (
			m_uVertices * sizeof (BRUSH_VERTEX),
			0,
			BRUSH_VERTEX::FVF,
			D3DPOOL_MANAGED,
			&m_pVB,
			NULL) ) )
	{
		return E_FAIL;
	}

	BRUSH_VERTEX* pVertices = NULL;
	if (FAILED (m_pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
	{
		return E_FAIL;
	}

	for (AUInt32 z = 0; z < m_uCells + 1; ++z)
	{
		for (AUInt32 x = 0; x < m_uCells + 1; ++x)
		{
			pVertices->p.x	= (AReal32)x - m_uCells / 2;
			pVertices->p.z	= -( (AReal32)z - m_uCells / 2);
			pVertices->p.y	= 0.0f;

			pVertices->t.x	= (AReal32)x / m_uCells;
			pVertices->t.y	= (AReal32)z / m_uCells;
			++pVertices;
		}
	}

	m_pVB->Unlock ();

	return S_OK;
}

HRESULT	CABrush::InitIndexBuffer()
{
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateIndexBuffer (
			m_uIndices * sizeof (BRUSH_INDEX),
			0,
			D3DFMT_INDEX32,
			D3DPOOL_MANAGED,
			&m_pIB,
			NULL) ) )
	{
		return E_FAIL;
	}

	BRUSH_INDEX* pIndices = NULL;
	if (FAILED (m_pIB->Lock (0, 0, (void**)&pIndices, 0) ) )
	{
		return E_FAIL;
	}
	
	for (AUInt32 z = 0; z < m_uCells; ++z)
	{
		for (AUInt32 x = 0; x < m_uCells; ++x)
		{
			pIndices->_0	= z * (m_uCells + 1) + x;
			pIndices->_1	= z * (m_uCells + 1) + (x + 1);
			pIndices->_2	= (z + 1) * (m_uCells + 1) + x;
			++pIndices;
			pIndices->_0	= (z + 1) * (m_uCells + 1) + x;
			pIndices->_1	= z * (m_uCells + 1) + (x + 1);
			pIndices->_2	= (z + 1) * (m_uCells + 1) + (x + 1);
			++pIndices;
		}
	}

	m_pIB->Unlock ();

	return S_OK;
}

void CABrush::RestoreRenderState (bool bBegin)
{
	int i;
	static DWORD dwAlphaEnable;
	static DWORD dwSrcBlend;
	static DWORD dwDestBlend;

	static DWORD dwFogEnable;

	static DWORD dwMinFilter [1];
	static DWORD dwMagFilter [1];
	static DWORD dwMipFilter [1];

	static DWORD dwAlphaOP [2];
	static DWORD dwAlphaARG1 [2];
	static DWORD dwAlphaARG2 [2];
	static DWORD dwColorOP [2];
	static DWORD dwColorARG1 [2];
	static DWORD dwColorARG2 [2];
	static DWORD dwResultARG [2];
	static DWORD dwTextureTransfromFlags [2];

	if (bBegin)
	{
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_FOGENABLE, &dwFogEnable);
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_SRCBLEND, &dwSrcBlend);
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_DESTBLEND, &dwDestBlend);

		for (i = 0; i < 1; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MINFILTER, &dwMinFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MAGFILTER, &dwMagFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MIPFILTER, &dwMipFilter [i]);
		}
		
		for (i = 0; i < 2; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_COLOROP, &dwColorOP [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_COLORARG1, &dwColorARG1 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_COLORARG2, &dwColorARG2 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_ALPHAOP, &dwAlphaOP [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_ALPHAARG1, &dwAlphaARG1 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_ALPHAARG2, &dwAlphaARG2 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_RESULTARG, &dwResultARG [i]);

			APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (i, D3DTSS_TEXTURETRANSFORMFLAGS, &dwTextureTransfromFlags [i]);
		}
	}
	else
	{
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_FOGENABLE, dwFogEnable);
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, dwSrcBlend);
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, dwDestBlend);
		
		for (i = 0; i < 1; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MINFILTER, dwMinFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MAGFILTER, dwMagFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MIPFILTER, dwMipFilter [i]);
		}
		
		for (i = 0; i < 2; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_COLOROP, dwColorOP [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_COLORARG1, dwColorARG1 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_COLORARG2, dwColorARG2 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_ALPHAOP, dwAlphaOP [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_ALPHAARG1, dwAlphaARG1 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_ALPHAARG2, dwAlphaARG2 [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_RESULTARG, dwResultARG [i]);

			APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (i, D3DTSS_TEXTURETRANSFORMFLAGS, dwTextureTransfromFlags [i]);
		}
	}
}

CABrush::CABrush ()
{
	m_pVB = NULL;
	m_pIB = NULL;
	m_pTexture = NULL;
	m_pTerrain = NULL;
	m_uVertices = 0;
	m_uIndices = 0;
	m_uCells = 0;
	m_vSize = AVector3 (0.0f);
	m_fIntensity = 1.0f;

	D3DXMatrixIdentity (&m_matWorld);
}

CABrush::~CABrush ()
{
	Release ();
}

CABrush::CABrush (const CABrush& brush)
{
	DeepCopy (brush);
}

CABrush& CABrush::operator=(const CABrush& brush)
{
	DeepCopy (brush);

	return (*this);
}

HRESULT	CABrush::Init (char* szDiffuse, AUInt32 uCells, CAMap* pTerrain)
{
	m_uCells	= uCells;
	m_uVertices	= (m_uCells + 1) * (m_uCells + 1);
	m_uIndices	= m_uCells * m_uCells * 2;

	m_pTerrain = pTerrain;

	if (FAILED (InitVertexBuffer () ) )
	{
		return E_FAIL;
	}

	if (FAILED (InitIndexBuffer () ) )
	{
		return E_FAIL;
	}

	strcpy (m_szDiffuse, szDiffuse);

	if (FAILED (D3DXCreateTextureFromFileA (
			APROJECT_WINDOW->GetD3DDevice (),
			szDiffuse,
			&m_pTexture) ) )
	{
		return E_FAIL;
	}

	return S_OK;
}

void CABrush::Reset (void)
{
}

void CABrush::Update (void)
{
	if (m_vSize.x < 2.0f)
		m_vSize.x = 2.0f;
	if (m_vSize.x > 128.0f)
		m_vSize.x = 128.0f;
}

void CABrush::Render (void)
{
	DWORD state = 0;

	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 1.0f, 1.0f, 1.0f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 1.0f);
	CAMaterial::GetSingleton ()->SetMaterial ();

	D3DXMATRIX matOriWorld;
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;

	APROJECT_WINDOW->GetD3DDevice ()->GetTransform (D3DTS_WORLD, &matOriWorld);
	D3DXMatrixTranslation (&matPosition, m_vPosition.x, 0.0f, m_vPosition.z);
	D3DXMatrixScaling (&matSize, (m_vSize.x / m_uCells), 1.0f, (m_vSize.x / m_uCells) );
	m_matWorld = matOriWorld * matSize * matPosition;
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &m_matWorld);
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("View", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () );
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("Projection", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () );

	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("Obj_OriDif");

		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pVB, 0, sizeof (BRUSH_VERTEX) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (BRUSH_VERTEX::FVF);
		APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIB);

		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture1", m_pTexture);
		if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
		{
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
					{
						APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (
							D3DPT_TRIANGLELIST,
							0,
							0,
							m_uVertices,
							0,
							m_uIndices);

						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End ();
			}
		}
	}
}

/*
void CABrush::Render (void)
{

	RestoreRenderState (true);

	D3DXMATRIX matOriWorld;
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;

	APROJECT_WINDOW->GetD3DDevice ()->GetTransform (D3DTS_WORLD, &matOriWorld);

	D3DXMatrixTranslation (&matPosition, m_vPosition.x, 0.0f, m_vPosition.z);
	D3DXMatrixScaling (&matSize, (m_vSize.x / m_uCells), 1.0f, (m_vSize.x / m_uCells) );
	m_matWorld = matOriWorld * matSize * matPosition;

	//m_matWorld._11	= (m_vSize.x / m_uCells);
	//m_matWorld._33	= (m_vSize.x / m_uCells);
	//m_matWorld._41	= fX;
	//m_matWorld._43	= fZ;
	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &m_matWorld);

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_FOGENABLE, FALSE);

	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 1.0f, 1.0f, 1.0f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 1.0f);
	CAMaterial::GetSingleton ()->SetMaterial ();

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ONE);

	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_TEXCOORDINDEX, 0);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	APROJECT_WINDOW->GetD3DDevice ()->SetTexture (0, m_pTexture);

	APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pVB, 0, sizeof (BRUSH_VERTEX) );
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (BRUSH_VERTEX::FVF);
	APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIB);
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (
			D3DPT_TRIANGLELIST,
			0,
			0,
			m_uVertices,
			0,
			m_uIndices);

	APROJECT_WINDOW->GetD3DDevice ()->SetTexture (0, NULL);

	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &matOriWorld);

	RestoreRenderState (false);
}*/

void CABrush::SetDecal (AReal32 fX, AReal32 fZ)
{
	BRUSH_VERTEX* pVertices = NULL;
	if (SUCCEEDED (m_pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
	{
		for (AUInt32 z = 0; z < m_uCells + 1; ++z )
		{
			for (AUInt32 x = 0; x < m_uCells + 1; ++x )
			{
				AReal32 fDistX = ( (AReal32)x - m_uCells / 2) * (m_vSize.x / m_uCells);
				AReal32 fDistZ = -( (AReal32)z - m_uCells / 2) * (m_vSize.x / m_uCells);
				pVertices->p.y = m_pTerrain->GetHeight (fX + fDistX, fZ + fDistZ) + 0.1f;
				++pVertices;
			}
		}

		m_pVB->Unlock ();
	}

	m_vPosition.x = fX;
	m_vPosition.z = fZ;
}

void CABrush::SetIntensity (AReal32 fIntensity)
{
	m_fIntensity += fIntensity;

	if (m_fIntensity < 0.0f)
		m_fIntensity = 0.0f;

	if (m_fIntensity > 1.0f)
		m_fIntensity = 1.0f;
}

AReal32 CABrush::GetIntensity ()
{
	return m_fIntensity;
}