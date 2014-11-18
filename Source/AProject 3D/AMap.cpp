#include "AMapInclude.h"

CAMaterial* CAMaterial::GetSingleton ()
{
	static CAMaterial material;

	return &material;
}

void CAMaterial::SetDiffuse (AReal32 a, AReal32	r, AReal32 g, AReal32 b)
{
	SetColorValue (m_d3dMaterial.Diffuse, a, r, g, b);
}

void CAMaterial::SetAmbient (AReal32 a, AReal32 r, AReal32 g, AReal32 b)
{
	SetColorValue (m_d3dMaterial.Ambient, a, r, g, b);
}

void CAMaterial::SetSpecular (AReal32 a, AReal32 r, AReal32 g, AReal32 b)
{
	SetColorValue (m_d3dMaterial.Specular, a, r, g, b);
}

void CAMaterial::SetEmissive (AReal32 a, AReal32 r, AReal32 g, AReal32 b)
{
	SetColorValue (m_d3dMaterial.Emissive, a, r, g, b);
}

void CAMaterial::SetPower (AReal32 fPower)
{
	m_d3dMaterial.Power = fPower;
}

void CAMaterial::SetMaterial (void)
{
	APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&m_d3dMaterial);
}

CAMaterial::CAMaterial ()
{
	ZeroMemory (&m_d3dMaterial, sizeof (D3DMATERIAL9) );
}

void CAMaterial::SetColorValue (D3DCOLORVALUE& d3dColorValue, AReal32 a, AReal32 r, AReal32 g, AReal32 b)
{
	d3dColorValue.a	= a;
	d3dColorValue.r	= r;
	d3dColorValue.g	= g;
	d3dColorValue.b	= b;
}

void CAMap::Release ()
{
	SAFE_RELEASE (m_pTexHeight);

	if (m_pTileInfo)
	{
		for (AUInt32 uTileIndex = 0; uTileIndex < m_uTiles * m_uTiles; ++uTileIndex)
		{
			SAFE_RELEASE (m_pTileInfo [uTileIndex].pVB);
			SAFE_RELEASE (m_pTileInfo [uTileIndex].pNormalVB);
		}
		SAFE_DELETE_ARRAY (m_pTileInfo);
	}

	for (AUInt32 uLevel = 0; uLevel < m_uMaxLevel; ++uLevel)
		for (AUInt32 uCracks = 0; uCracks < TILE_INFO::MAX_CRACK; ++uCracks)
			SAFE_RELEASE (m_ppTileIB [uLevel][uCracks]);

	SAFE_DELETE_ARRAY (m_ppTileIB);
	SAFE_RELEASE (m_pTileNormalIB);

	SAFE_DELETE (m_pQuadTree);
	
	m_uSize			= 0;
	m_uTiles		= 0;
	m_uCellsPerTile	= 0;
	m_uMaxLevel		= 0;

	m_vecVertices.clear ();
	m_vecVisibleIndex.clear ();
}

HRESULT CAMap::InitVertexBuffer (void)
{
	D3DLOCKED_RECT	d3drc;

	//D3DX_FILTER_NONE

	m_pTexHeight->LockRect (0, &d3drc, NULL, 0);
	WORD* pByte = (WORD*)d3drc.pBits;

	for (AUInt32 z = 0; z < m_uSize; ++z)
	{
		for (AUInt32 x = 0; x < m_uSize; ++x)
		{
			TERRAIN_VERTEX	v;
			UINT			uPixelIndex	= z * m_uSize + x;

			v.p.x	= (float)x - (m_uSize - 1) / 2;
			v.p.z	= -( (float)z - (m_uSize - 1) / 2);
			v.p.y	= (float)pByte [uPixelIndex] * 0.1f;

			v.n.x	= 0.0f;
			v.n.y	= 1.0f;
			v.n.z	= 0.0f;

			v.t1.x	= (float)x / (m_uSize - 1);
			v.t1.y	= (float)z / (m_uSize - 1);

			v.t2.x	= (float)x / (m_uSize / m_uTiles - 1);
			v.t2.y	= (float)z / (m_uSize / m_uTiles - 1);

			m_vecVertices.push_back (v);
		}
	}
	m_pTexHeight->UnlockRect (0);

	m_pTileInfo	= new TILE_INFO [m_uTiles * m_uTiles];

	for (AUInt32 uCols = 0; uCols < m_uTiles; ++uCols)
	{
		for (AUInt32 uRows = 0; uRows < m_uTiles; ++uRows)
		{
			AUInt32 uTileIndex = uCols * m_uTiles + uRows;

			if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (
					(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1) * sizeof (TERRAIN_VERTEX),
					0,
					TERRAIN_VERTEX::FVF,
					D3DPOOL_MANAGED,
					&m_pTileInfo[ uTileIndex ].pVB,
					NULL) ) )
			{
				return E_FAIL;
			}

			TERRAIN_VERTEX*	pVertices = NULL;
			if (FAILED (m_pTileInfo [uTileIndex].pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
			{
				return E_FAIL;
			}

			for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
			{
				for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
				{
					AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

					pVertices->p		= m_vecVertices [uVertexIndex].p;
					pVertices->n		= m_vecVertices [uVertexIndex].n;
					pVertices->t1	= m_vecVertices [uVertexIndex].t1;
					pVertices->t2	= m_vecVertices [uVertexIndex].t2;
					++pVertices;
				}
			}

			m_pTileInfo [uTileIndex].pVB->Unlock ();
		}
	}

	return S_OK;
}

HRESULT CAMap::InitIndexBuffer (void)
{
	AUInt32 uCells = m_uCellsPerTile;

	while (uCells > 2)
	{
		uCells /= 2;
		++m_uMaxLevel;
	}

	m_ppTileIB = new LPDIRECT3DINDEXBUFFER9 [m_uMaxLevel][TILE_INFO::MAX_CRACK];

	for (AUInt32 uLevel = 0; uLevel < m_uMaxLevel; ++uLevel)
	{
		for (AUInt32 uCracks = 0; uCracks < TILE_INFO::MAX_CRACK; ++uCracks)
		{
			if ( (uLevel == 0) && ( (uCracks > 0) && (uCracks < TILE_INFO::MAX_CRACK) ) )
			{
				m_ppTileIB [uLevel][uCracks] = NULL;
				continue;
			}

			AUInt32 uCellsPerTile	= m_uCellsPerTile / (1 << uLevel);
			AUInt32 uIndices		= uCellsPerTile * uCellsPerTile * 2;

			if ( (uLevel > 0) && ( (uCracks > 0) && (uCracks < TILE_INFO::MAX_CRACK) ) )
				uIndices = uCellsPerTile * uCellsPerTile * 2 + uCellsPerTile * 3;

			if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateIndexBuffer (
					uIndices * sizeof( TERRAIN_INDEX ),
					0,
					D3DFMT_INDEX32,
					D3DPOOL_MANAGED,
					&m_ppTileIB [uLevel][uCracks],
					NULL) ) )
			{
				return E_FAIL;
			}

			TERRAIN_INDEX* pIndices = NULL;
			if (FAILED (m_ppTileIB [uLevel][uCracks]->Lock (0, 0, (void**)&pIndices, 0) ) )
			{
				return E_FAIL;
			}

			for (AUInt32 z = 0; z < m_uCellsPerTile; z += (1 << uLevel) )
			{
				for (AUInt32 x = 0; x < m_uCellsPerTile; x += (1 << uLevel) )
				{
					//	0 ─ 1 ─ 2
					//	│ ＼│／ │
					//	3 ─ 4 ─ 5
					//	│ ／│＼ │
					//	6 ─ 7 ─ 8
					AUInt32 uIndex [9] = {0,};

					uIndex [0] = ( z * ( m_uCellsPerTile + 1 ) + x );
					uIndex [1] = ( z * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) / 2 ) );
					uIndex [2] = ( z * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) ) );
					uIndex [3] = ( ( z + ( 1 << uLevel ) / 2 ) * ( m_uCellsPerTile + 1 ) + x );
					uIndex [4] = ( ( z + ( 1 << uLevel ) / 2 ) * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) / 2 ) );
					uIndex [5] = ( ( z + ( 1 << uLevel ) / 2 ) * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) ) );
					uIndex [6] = ( ( z + ( 1 << uLevel ) ) * ( m_uCellsPerTile + 1 ) + x );
					uIndex [7] = ( ( z + ( 1 << uLevel ) ) * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) / 2 ) );
					uIndex [8] = ( ( z + ( 1 << uLevel ) ) * ( m_uCellsPerTile + 1 ) + ( x + ( 1 << uLevel ) ) );

					switch (uCracks)
					{
					case TILE_INFO::LEFT:
						if (x == 0)
						{
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [3];		pIndices->_2 = uIndex [0];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [0];		pIndices->_2 = uIndex [2];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [8];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [8];		pIndices->_2 = uIndex [6];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [6];		pIndices->_2 = uIndex [3];		++pIndices;
							continue;
						}
						break;

					case TILE_INFO::TOP:
						if (z == 0)
						{
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [0];		pIndices->_2 = uIndex [1];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [1];		pIndices->_2 = uIndex [2];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [8];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [8];		pIndices->_2 = uIndex [6];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [6];		pIndices->_2 = uIndex [0];		++pIndices;
							continue;
						}
						break;

					case TILE_INFO::RIGHT:
						if (x == m_uCellsPerTile - (1 << uLevel) )
						{
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [5];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [5];		pIndices->_2 = uIndex [8];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [8];		pIndices->_2 = uIndex [6];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [6];		pIndices->_2 = uIndex [0];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [0];		pIndices->_2 = uIndex [2];		++pIndices;
							continue;
						}
						break;

					case TILE_INFO::BOTTOM:
						if (z == m_uCellsPerTile - (1 << uLevel) )
						{
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [8];		pIndices->_2 = uIndex [7];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [7];		pIndices->_2 = uIndex [6];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [6];		pIndices->_2 = uIndex [0];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [0];		pIndices->_2 = uIndex [2];		++pIndices;
							pIndices->_0 = uIndex [4];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [8];		++pIndices;
							continue;
						}
						break;
					}

					pIndices->_0 = uIndex [0];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [6];		++pIndices;
					pIndices->_0 = uIndex [6];		pIndices->_1 = uIndex [2];		pIndices->_2 = uIndex [8];		++pIndices;
				}
			}
			m_ppTileIB [uLevel][uCracks]->Unlock ();
		}
	}

	return S_OK;
}

HRESULT	CAMap::InitTexture (void)
{
	for (vector<SPLATTING_MAP>::iterator i = m_pTexSplatting.begin (); i != m_pTexSplatting.end (); ++i)
	{
		TCHAR	szBuf [255] = {NULL,};

		if (FAILED (D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), i->szSplatting, &i->pTexDiffuse) ) )
			return E_FAIL;
	}

	return S_OK;
}

HRESULT	CAMap::InitAlphaTexture (void)
{
	for (vector<SPLATTING_MAP>::iterator i = m_pTexSplatting.begin (); i != m_pTexSplatting.end (); ++i)
	{
		if (FAILED (D3DXCreateTexture (APROJECT_WINDOW->GetD3DDevice (), m_uTiles * m_uCellsPerTile * 2, m_uTiles * m_uCellsPerTile * 2,
				D3DX_DEFAULT, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &i->pTexAlpha) ) )
		{
			return E_FAIL;
		}

		D3DLOCKED_RECT d3drc;
		i->pTexAlpha->LockRect (0, &d3drc, NULL, 0);

		BYTE*	pBits		= (BYTE*)d3drc.pBits;
		BYTE	byteValue	= (i == m_pTexSplatting.begin () ) ? 0xFF : 0x00;
		for (AUInt32 z = 0; z < m_uTiles * m_uCellsPerTile * 2; ++z)
		{
			for (AUInt32 x = 0; x < m_uTiles * m_uCellsPerTile * 2; ++x)
			{
				AUInt32 uPixelIndex = z * d3drc.Pitch + x * 4;

				pBits [uPixelIndex + 0] = byteValue;	// B
				pBits [uPixelIndex + 1] = byteValue;	// G
				pBits [uPixelIndex + 2] = byteValue;	// R
				pBits [uPixelIndex + 3] = byteValue;	// A
			}
		}
		i->pTexAlpha->UnlockRect (0);
	}

	return S_OK;
}

void CAMap::BeginRenderState (void)
{
	//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	//												  
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
}

void CAMap::RestoreRenderState (bool bBegin)
{
	int i;
	static DWORD dwAlphaEnable;
	static DWORD dwSrcBlend;
	static DWORD dwDestBlend;

	static DWORD dwMinFilter [5];
	static DWORD dwMagFilter [5];
	static DWORD dwMipFilter [5];

	static DWORD dwAddressU [2];
	static DWORD dwAddressV [2];

	static DWORD dwAlphaOP [5];
	static DWORD dwAlphaARG1 [5];
	static DWORD dwAlphaARG2 [5];
	static DWORD dwColorOP [5];
	static DWORD dwColorARG1 [5];
	static DWORD dwColorARG2 [5];
	static DWORD dwResultARG [5];
	static DWORD dwTextureTransfromFlags [5];

	if (bBegin)
	{
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_SRCBLEND, &dwSrcBlend);
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_DESTBLEND, &dwDestBlend);
		
		APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (0, D3DSAMP_ADDRESSU, &dwAddressU [0]);
		APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (0, D3DSAMP_ADDRESSV, &dwAddressV [0]);
		APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (1, D3DSAMP_ADDRESSU, &dwAddressU [1]);
		APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (1, D3DSAMP_ADDRESSV, &dwAddressV [1]);

		for (i = 0; i < 5; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MINFILTER, &dwMinFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MAGFILTER, &dwMagFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (i, D3DSAMP_MIPFILTER, &dwMipFilter [i]);
		}
		
		for (i = 0; i < 5; i ++)
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
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, dwSrcBlend);
		APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, dwDestBlend);
		
		APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_ADDRESSU, dwAddressU [0]);
		APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_ADDRESSV, dwAddressV [0]);
		APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_ADDRESSU, dwAddressU [1]);
		APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (1, D3DSAMP_ADDRESSV, dwAddressV [1]);
		
		for (i = 0; i < 5; i ++)
		{
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MINFILTER, dwMinFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MAGFILTER, dwMagFilter [i]);
			APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (i, D3DSAMP_MIPFILTER, dwMipFilter [i]);
		}
		
		for (i = 0; i < 5; i ++)
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

HRESULT	CAMap::InitQuadTree ()
{
	m_pQuadTree = new CAQuadTree ();

	if (!m_pQuadTree->Create (m_uSize - 1, m_uSize - 1, m_uCellsPerTile) )
		return E_FAIL;

	m_pQuadTree->SetBoundingSphere (m_vecVertices);

	return S_OK;
}

HRESULT	CAMap::SetTileCenter ()
{
	for (AUInt32 z = 0; z < m_uTiles; ++z)
	{
		for (AUInt32 x = 0; x < m_uTiles; ++x)
		{
			AUInt32 uTileIndex = z * m_uTiles + x;

			TERRAIN_VERTEX*	pVertices = NULL;
			if (FAILED (m_pTileInfo [uTileIndex].pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
			{
				return E_FAIL;
			}

			m_pTileInfo [uTileIndex].vCenter	= pVertices [ (m_uCellsPerTile / 2) * (m_uCellsPerTile + 1) + (m_uCellsPerTile / 2)].p;
			m_pTileInfo [uTileIndex].vCenter.y	= 0.0f;

			D3DXVECTOR3	vLeftTop				= pVertices [0].p;
			vLeftTop.y							= 0.0f;

			m_pTileInfo [uTileIndex].fRadius	= D3DXVec3Length (&(vLeftTop - m_pTileInfo [uTileIndex].vCenter) );

			m_pTileInfo [uTileIndex].pVB->Unlock ();
		}
	}

	return S_OK;
}

void CAMap::SetCrackIndex ()
{
	for (vector<AUInt32>::iterator i = m_vecVisibleIndex.begin (); i != m_vecVisibleIndex.end (); ++i)
	{
		AUInt32 uLevel			= m_pTileInfo [*i].uLevel;
		AUInt32 uLevels [4]		= {0,};
		AUInt32 uCellsPerTile	= m_uCellsPerTile / (1 << uLevel);
		AUInt32 uIndices		= uCellsPerTile * uCellsPerTile * 2;

		uLevels [0] = ( (*i) % m_uTiles > 0 )				? m_pTileInfo [ (*i) - 1 ].uLevel			: m_pTileInfo [*i].uLevel;
		uLevels [1] = ( (*i) / m_uTiles > 0 )				? m_pTileInfo [ (*i) - m_uTiles ].uLevel		: m_pTileInfo [*i].uLevel;
		uLevels [2] = ( (*i) % m_uTiles < m_uTiles - 1 )		? m_pTileInfo [ (*i) + 1 ].uLevel			: m_pTileInfo [*i].uLevel;
		uLevels [3] = ( (*i) / m_uTiles < m_uTiles - 1 )		? m_pTileInfo [ (*i) + m_uTiles ].uLevel	: m_pTileInfo [*i].uLevel;

		AUInt32 uCount = 0;
		for (AUInt32 j = 0; j < 4; ++j)
		{
			if (uLevel > uLevels [j])
				uCount += (1 << j);
		}

		if (uCount % 3 == 0)
			uCount = 0;

		switch (uCount)
		{
		case 0:	m_pTileInfo [*i].uCracks = TILE_INFO::BASE;		break;
		case 1:	m_pTileInfo [*i].uCracks = TILE_INFO::LEFT;		break;
		case 2:	m_pTileInfo [*i].uCracks = TILE_INFO::TOP;		break;
		case 4:	m_pTileInfo [*i].uCracks = TILE_INFO::RIGHT;	break;
		case 8:	m_pTileInfo [*i].uCracks = TILE_INFO::BOTTOM;	break;
		}

		(m_pTileInfo [*i].uCracks == TILE_INFO::BASE) ?
			(m_pTileInfo [*i].uIndices = uIndices) :
			(m_pTileInfo [*i].uIndices = uIndices + uCellsPerTile * 3);
	}
}

HRESULT	CAMap::InitNormalVertexBuffer (void)
{
	for (AUInt32 uCols = 0; uCols < m_uTiles; ++uCols)
	{
		for (AUInt32 uRows = 0; uRows < m_uTiles; ++uRows)
		{
			AUInt32 uTileIndex = uCols * m_uTiles + uRows;

			if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (
					(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1) * 2 * sizeof (NORMAL_VERTEX),
					0,
					NORMAL_VERTEX::FVF,
					D3DPOOL_MANAGED,
					&m_pTileInfo [uTileIndex].pNormalVB,
					NULL) ) )
			{
				return E_FAIL;
			}

			NORMAL_VERTEX* pVertices = NULL;
			if (FAILED (m_pTileInfo [uTileIndex].pNormalVB->Lock (0, 0, (void**)&pVertices, 0) ) )
				return E_FAIL;

			for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
			{
				for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
				{
					AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

					pVertices->p	= m_vecVertices [uVertexIndex].p;
					++pVertices;
					pVertices->p	= m_vecVertices [uVertexIndex].p + m_vecVertices [uVertexIndex].n * 0.5f;
					++pVertices;
				}
			}
			m_pTileInfo [uTileIndex].pNormalVB->Unlock ();
		}
	}

	return S_OK;
}

HRESULT CAMap::InitNormalIndexBuffer (void)
{
	if (FAILED(APROJECT_WINDOW->GetD3DDevice ()->CreateIndexBuffer (
			(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1) * sizeof (NORMAL_INDEX),
			0,
			D3DFMT_INDEX32,
			D3DPOOL_MANAGED,
			&m_pTileNormalIB,
			NULL) ) )
	{
		return E_FAIL;
	}

	NORMAL_INDEX* pIndices = NULL;
	if (FAILED (m_pTileNormalIB->Lock (0, 0, (void**)&pIndices, 0) ) )
		return E_FAIL;

	for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
	{
		for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
		{
			pIndices->_0 = (z * (m_uCellsPerTile + 1) + x) * 2;
			pIndices->_1 = (z * (m_uCellsPerTile + 1) + x) * 2 + 1;
			++pIndices;
		}
	}
	m_pTileNormalIB->Unlock ();

	return S_OK;
}

HRESULT	CAMap::SaveHeightMap (char* szHeightMap)
{
	D3DLOCKED_RECT d3drc;
	m_pTexHeight->LockRect (0, &d3drc, NULL, 0);
	WORD* pByte = (WORD*)d3drc.pBits;

	for (AUInt32 z = 0; z < m_uSize; ++z)
	{
		for (AUInt32 x = 0; x < m_uSize; ++x)
		{
			AUInt32 uVertexIndex	= z * m_uSize + x;

			pByte [uVertexIndex] = (WORD)(m_vecVertices [uVertexIndex].p.y * 10.0f);
		}
	}
	m_pTexHeight->UnlockRect (0);
	if (FAILED (D3DXSaveTextureToFileA (szHeightMap, D3DXIFF_BMP, m_pTexHeight, NULL) ) )
		return E_FAIL;

	return S_OK;
}

HRESULT CAMap::SaveDiffuseMap (void)
{
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); ++i)
	{
		char szBuf [256];

		sprintf (szBuf, "./Temp/Diffuse%u.png", i);
		if (FAILED (D3DXSaveTextureToFileA (szBuf, D3DXIFF_PNG, m_pTexSplatting [i].pTexDiffuse, NULL) ) )
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT	CAMap::SaveAlphaMap (void)
{
	for (AUInt32 i = 1; i < m_pTexSplatting.size (); ++i)
	{
		char szBuf [256];

		sprintf (szBuf, "./Temp/Alpha%u.png", i);
		if (FAILED (D3DXSaveTextureToFileA (szBuf, D3DXIFF_PNG, m_pTexSplatting [i].pTexAlpha, NULL) ) )
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT	CAMap::LoadHeightMap (void* pHeightMap, AUInt32 nSize)
{
	if (FAILED (D3DXCreateTextureFromFileInMemoryEx (
			APROJECT_WINDOW->GetD3DDevice (),
			pHeightMap,
			nSize,
			D3DX_FROM_FILE,
			D3DX_FROM_FILE,
			D3DX_DEFAULT,
			0,
			D3DFMT_A4R4G4B4,
			D3DPOOL_MANAGED,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			NULL,
			NULL,
			NULL,
			&m_pTexHeight) ) )
	{
		return E_FAIL;
	}

	D3DSURFACE_DESC	ddsd;
	ZeroMemory (&ddsd, sizeof (D3DSURFACE_DESC) );

	m_pTexHeight->GetLevelDesc (0, &ddsd);

	m_uSize		= ddsd.Height;
	m_uTiles	= (m_uSize - 1) / m_uCellsPerTile;

	D3DXMatrixScaling (&m_matScale, (float)m_uTiles * 4.0f, (float)m_uTiles * 4.0f, 1.0f);

	return S_OK;
}

HRESULT CAMap::LoadDiffuseMap (SHANDLE pHandle)
{
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); ++i)
	{
		char szBuf [256];
		void* pData;
		AUInt32 nSize;

		sprintf (szBuf, "/TileMap/Diffuse%u.png", i);

		AHANDLE a = CASecuFS::GetSingleton ()->OpenFile (pHandle, szBuf);
		CASecuFS::GetSingleton ()->SeekFile (a, 0, FileSeekEnd);
		nSize = CASecuFS::GetSingleton ()->GetFileOffset (a).ConvertToInt32 ();
		CASecuFS::GetSingleton ()->SeekFile (a, 0, FileSeekSet);
		pData = new char [nSize];
		CASecuFS::GetSingleton ()->ReadFile (pData, nSize, a);
		CASecuFS::GetSingleton ()->CloseFile (&a);

		if (FAILED (D3DXCreateTextureFromFileInMemory (
			APROJECT_WINDOW->GetD3DDevice (),
			pData,
			nSize,
			&m_pTexSplatting [i].pTexDiffuse) ) )
		{
			return E_FAIL;
		}
		SAFE_DELETE (pData);
	}

	return S_OK;
}

HRESULT CAMap::LoadAlphaMap (SHANDLE pHandle)
{
	if (FAILED (D3DXCreateTexture (
			APROJECT_WINDOW->GetD3DDevice (),
			m_uTiles * m_uCellsPerTile * 2,
			m_uTiles * m_uCellsPerTile * 2,
			1,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED,
			&m_pTexSplatting [0].pTexAlpha) ) )
	{
		return E_FAIL;
	}

	D3DLOCKED_RECT	d3drc;
	m_pTexSplatting [0].pTexAlpha->LockRect (0, &d3drc, NULL, 0);

	BYTE*	pBits		= (BYTE*)d3drc.pBits;
	BYTE	byteValue	= 0xFF;
	for (AUInt32 z = 0; z < m_uTiles * m_uCellsPerTile * 2; ++z)
	{
		for (AUInt32 x = 0; x < m_uTiles * m_uCellsPerTile * 2; ++x)
		{
			AUInt32 uPixelIndex = z * d3drc.Pitch + x * 4;

			pBits [uPixelIndex + 0] = byteValue;
			pBits [uPixelIndex + 1] = byteValue;
			pBits [uPixelIndex + 2] = byteValue;
			pBits [uPixelIndex + 3] = byteValue;
		}
	}

	for (AUInt32 i = 1; i < m_pTexSplatting.size (); ++i)
	{
		char szBuf [256];
		void* pData;
		AUInt32 nSize;

		sprintf (szBuf, "/AlphaMap/Alpha%u.png", i);

		AHANDLE a = CASecuFS::GetSingleton ()->OpenFile (pHandle, szBuf);
		CASecuFS::GetSingleton ()->SeekFile (a, 0, FileSeekEnd);
		nSize = CASecuFS::GetSingleton ()->GetFileOffset (a).ConvertToInt32 ();
		CASecuFS::GetSingleton ()->SeekFile (a, 0, FileSeekSet);
		pData = new char [nSize];
		CASecuFS::GetSingleton ()->ReadFile (pData, nSize, a);
		CASecuFS::GetSingleton ()->CloseFile (&a);

		if (FAILED (D3DXCreateTextureFromFileInMemory (
			APROJECT_WINDOW->GetD3DDevice (),
			pData,
			nSize,
			&m_pTexSplatting [i].pTexAlpha) ) )
		{
			return E_FAIL;
		}
		SAFE_DELETE (pData);
	}

	return S_OK;
}

HRESULT	CAMap::ComputNormal ()
{
	for (AUInt32 z = 1; z < m_uSize - 1; ++z)
	{
		for (AUInt32 x = 1; x < m_uSize - 1; ++x)
		{
			float fLeft		= m_vecVertices [ z * m_uSize + ( x - 1 ) ].p.y;
			float fRight	= m_vecVertices [ z * m_uSize + ( x + 1 ) ].p.y;
			float fUp		= m_vecVertices [ ( z - 1 ) * m_uSize + x ].p.y;
			float fDown		= m_vecVertices [ ( z + 1 ) * m_uSize + x ].p.y;

			D3DXVECTOR3	vNormal	= D3DXVECTOR3 ( (fLeft - fRight), 2.0f, (fDown - fUp) );
			D3DXVec3Normalize (&vNormal, &vNormal);

			m_vecVertices [z * m_uSize + x].n = vNormal;
		}
	}

	for (AUInt32 uCols = 0; uCols < m_uTiles; ++uCols)
	{
		for (AUInt32 uRows = 0; uRows < m_uTiles; ++uRows)
		{
			AUInt32 uTileIndex = uCols * m_uTiles + uRows;

			TERRAIN_VERTEX* pVertices = NULL;
			if (FAILED (m_pTileInfo [uTileIndex].pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
				return E_FAIL;

			for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
			{
				for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
				{
					AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

					pVertices->n	= m_vecVertices [uVertexIndex].n;
					++pVertices;
				}
			}

			m_pTileInfo [uTileIndex].pVB->Unlock ();

			NORMAL_VERTEX* pNormals = NULL;
			if (FAILED (m_pTileInfo [uTileIndex].pNormalVB->Lock (0, 0, (void**)&pNormals, 0) ) )
				return E_FAIL;

			for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
			{
				for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
				{
					AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

					pNormals->p	= m_vecVertices [uVertexIndex].p;
					++pNormals;
					pNormals->p	= m_vecVertices [uVertexIndex].p + m_vecVertices [uVertexIndex].n * 0.5f;
					++pNormals;
				}
			}
			m_pTileInfo [uTileIndex].pNormalVB->Unlock ();
		}
	}

	return S_OK;
}

void CAMap::Culling (CAFrustum* pFrustum)
{
	m_vecVisibleIndex.clear ();
	m_pQuadTree->Culling (pFrustum, m_vecVisibleIndex, m_uTiles, m_vSize);
}

void CAMap::SetLODLevel (const D3DXVECTOR3& vLookAt)
{
	if (m_bLOD)
	{
		for (AUInt32 z = 0; z < m_uTiles; ++z)
		{
			for (AUInt32 x = 0; x < m_uTiles; ++x)
			{
				AUInt32 uTileIndex = z * m_uTiles + x;

				AReal32 fDistX	= abs (vLookAt.x - m_pTileInfo [uTileIndex].vCenter.x);
				AReal32 fDistZ	= abs (vLookAt.z - m_pTileInfo [uTileIndex].vCenter.z);
				AReal32 fDist	= (fDistX < fDistZ) ? fDistZ : fDistX;

				for (AUInt32 uLevel = 0; uLevel < m_uMaxLevel; ++uLevel)
				{
					if ( (AReal32)(uLevel * m_uLODLevel) < fDist)
						m_pTileInfo [uTileIndex].uLevel = uLevel;
					else
						break;
				}
			}
		}

		SetCrackIndex ();
	}
}

void CAMap::SetTransform (void)
{
	D3DXMATRIX matOriWorld;
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;
	D3DXMATRIX matRotX;
	D3DXMATRIX matRotY;
	D3DXMATRIX matRotZ;
	D3DXMATRIX matComplate;

	D3DXMatrixIdentity (&matOriWorld);
	D3DXMatrixScaling (&matSize, m_vSize.x, m_vSize.y, m_vSize.z);
	D3DXMatrixTranslation (&matPosition, m_vPosition.x, m_vPosition.y, m_vPosition.z);
	D3DXMatrixRotationX (&matRotX, m_fRotX);
	D3DXMatrixRotationY (&matRotY, m_fRotY);
	D3DXMatrixRotationZ (&matRotZ, m_fRotZ);
	m_matWorld = matOriWorld * matSize * matRotX * matRotY * matRotZ * matPosition;

	//APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &m_matWorld);
	//APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_TEXTURE1, &m_matScale);
}

void CAMap::RenderNormal (void)
{
	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 0.6f, 0.6f, 0.6f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 0.0f);
	CAMaterial::GetSingleton ()->SetMaterial ();

	for (vector<AUInt32>::iterator p = m_vecVisibleIndex.begin (); p != m_vecVisibleIndex.end (); ++p)
	{
		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pTileInfo [*p].pNormalVB, 0, sizeof (NORMAL_VERTEX) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (NORMAL_VERTEX::FVF);
		APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pTileNormalIB);
		APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (
				D3DPT_LINELIST,
				0,
				0,
				(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1) * 2,
				0,
				(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1) );
	}
}

CAMap::CAMap ()
{
	m_pTexHeight = NULL;

	m_pTexNormalMap = NULL;
	m_pTexSpecularMap = NULL;
	m_pTexLightMap = NULL;
	m_uCellsPerTile = 0;
	m_uTiles = 0;
	m_uSize = 0;
	m_pTileInfo = NULL;
	m_ppTileIB = NULL;
	m_pTileNormalIB = NULL;
	m_uMaxLevel = 0;
	m_pQuadTree = 0;
	m_bLOD = false;
	m_bRenderNormal = false;
	m_uLODLevel = 0;

	m_pTexSplatting.clear ();

	D3DXMatrixIdentity (&m_matWorld);
}

CAMap::~CAMap ()
{
	for (vector<SPLATTING_MAP>::iterator i = m_pTexSplatting.begin (); i != m_pTexSplatting.end (); ++i)
	{
		SAFE_RELEASE (i->pTexAlpha);
		SAFE_RELEASE (i->pTexDiffuse);
		SAFE_DELETE (i->szSplatting);
	}
	m_pTexSplatting.clear ();

	Release ();
}

void CAMap::SetLODUse (bool bLOD)
{
	m_bLOD = bLOD;
}

void CAMap::SetNormalRender (bool bRender)
{
	m_bRenderNormal = bRender;
}
	
bool CAMap::GetLODUse (void)
{
	return m_bLOD;
}

bool CAMap::GetNormalRender (void)
{
	return m_bRenderNormal;
}

void CAMap::Reset (void)
{
}

void CAMap::Update (void)
{
	//LPDIRECT3DSURFACE9 pSurface;
	//LPDIRECT3DSURFACE9 pBackbuffer;

	APROJECT_WINDOW->GetCurScene ()->GetCamera ()->UpdateFrustum ();
	Culling (APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum () );
	SetLODLevel (*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetLookat () );

	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 0.6f, 0.6f, 0.6f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 1.0f);
	CAMaterial::GetSingleton ()->SetMaterial ();

	SetTransform ();

	/*
	RestoreRenderState (true);
	BeginRenderState ();

	// Tile Map Draw
	m_pTexTileMap->GetSurfaceLevel (0, &pSurface);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderTarget (0, &pBackbuffer);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderTarget (0, pSurface);

	//// Create Vertex Buffer
	//AUInt32 size = m_uTiles * m_uCellsPerTile;
	//LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
	//APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (4 * sizeof (Texture2DVertex), D3DUSAGE_WRITEONLY,
	//											D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pVertexBuffer, NULL);
	//Texture2DVertex* pVertices;
	//Texture2DVertex vertices [] = {
	//	{size,		size,		0.0f, 1.0f, 0.0f, 0.0f},
	//	{size * 3,	size,		0.0f, 1.0f, 1.0f, 0.0f},
	//	{size,		size * 3,	0.0f, 1.0f, 0.0f, 1.0f},
	//	{size * 3,	size * 3,	0.0f, 1.0f, 1.0f, 1.0f},
	//};
	//pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0);
	//memcpy (pVertices, vertices, sizeof (*vertices) * 4);
	//pVertexBuffer->Unlock();

	// RTT
	APROJECT_WINDOW->GetD3DDevice ()->BeginScene ();
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); ++i)
	{
		APROJECT_WINDOW->GetD3DDevice ()->SetTexture (0, m_pTexSplatting [i].pTexAlpha);
		APROJECT_WINDOW->GetD3DDevice ()->SetTexture (1, m_pTexSplatting [i].pTexDiffuse);

		for (vector<AUInt32>::iterator p = m_vecVisibleIndex.begin (); p != m_vecVisibleIndex.end (); ++p)
		{
			APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pTileInfo [*p].pVB, 0, sizeof (TERRAIN_VERTEX) );
			APROJECT_WINDOW->GetD3DDevice ()->SetFVF (TERRAIN_VERTEX::FVF);
			(m_bLOD) ?
				APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [m_pTileInfo [*p].uLevel][m_pTileInfo[*p].uCracks]) :
				APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [0][0]);

			APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					0,
					0,
					(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1),
					0,
					(m_bLOD) ? m_pTileInfo [*p].uIndices : (m_uCellsPerTile * m_uCellsPerTile * 2) );
		}
		//APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, pVertexBuffer, 0, sizeof (Texture2DVertex) );
		//APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZRHW | D3DFVF_TEX1);
		//APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_TRIANGLESTRIP, 0, 2);
	}
	APROJECT_WINDOW->GetD3DDevice ()->EndScene ();

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderTarget (0, pBackbuffer);
	//SAFE_RELEASE (pVertexBuffer);
	SAFE_RELEASE (pBackbuffer);
	SAFE_RELEASE (pSurface);

	RestoreRenderState (false);*/

	/*
	/////////////1
	D3DXMATRIX matComplate;
	D3DXMATRIXA16 mLightView;
	SetTransform ();
	APROJECT_WINDOW->GetD3DDevice ()->GetTransform (D3DTS_WORLD, &matComplate);

	D3DXMatrixIdentity (&matComplate);

	APROJECT_WINDOW->GetLightEffect ()->SetInt ("SelectLightNumber", 0);
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldMatrix", &matComplate);
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldViewProjectionMatrix", &(*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () * 
		*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () ) );

	D3DXMATRIX matInverseTranspose;
	D3DXMatrixInverse (&matInverseTranspose, 0, &matComplate);
	D3DXMatrixTranspose (&matInverseTranspose, &matInverseTranspose);

	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldInverseTransposeMatrix", &matInverseTranspose);

	LPDIRECT3DSURFACE9 pSurface;
	LPDIRECT3DSURFACE9 pBackbuffer;
	LPDIRECT3DSURFACE9 pDSBackbuffer;
	m_pShadowTexture->GetSurfaceLevel (0, &pSurface);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderTarget (0, &pBackbuffer);
	APROJECT_WINDOW->GetD3DDevice ()->GetDepthStencilSurface (&pDSBackbuffer);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderTarget (0, pSurface);
	APROJECT_WINDOW->GetD3DDevice ()->SetDepthStencilSurface (m_pDSShadow);

	APROJECT_WINDOW->GetD3DDevice ()->Clear (0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0L);
	//APROJECT_WINDOW->GetD3DDevice ()->BeginScene ();
	/////////////1
	for (vector<AUInt32>::iterator p = m_vecVisibleIndex.begin (); p != m_vecVisibleIndex.end (); ++p)
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName ("ShadowMapping");

		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pTileInfo [*p].pVB, 0, sizeof (TERRAIN_VERTEX) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (TERRAIN_VERTEX::FVF);
		(m_bLOD) ?
			APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [m_pTileInfo [*p].uLevel][m_pTileInfo[*p].uCracks]) :
		APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [0][0]);

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
							(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1),
							0,
							(m_bLOD) ? m_pTileInfo [*p].uIndices : (m_uCellsPerTile * m_uCellsPerTile * 2) );

						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End();
			}
		}
	}
	/////////////////1
	D3DXSaveSurfaceToFileW (L"test1.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, pSurface, NULL, NULL);
	SAFE_RELEASE (pSurface);

	//APROJECT_WINDOW->GetD3DDevice ()->EndScene ();

	APROJECT_WINDOW->GetLightEffect ()->SetTexture ("shadowMapTexture", m_pShadowTexture);
	/////////////////1

	/////////////////2
	m_pShadowScreenMap->GetSurfaceLevel (0, &pSurface);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderTarget (0, pSurface);
	APROJECT_WINDOW->GetD3DDevice ()->SetDepthStencilSurface (m_pDSUnlit);

	APROJECT_WINDOW->GetD3DDevice ()->Clear (0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);
	//APROJECT_WINDOW->GetD3DDevice ()->BeginScene ();
	/////////////////2
	for (vector<AUInt32>::iterator p = m_vecVisibleIndex.begin (); p != m_vecVisibleIndex.end (); ++p)
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName ("UnlitMapping");

		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pTileInfo [*p].pVB, 0, sizeof (TERRAIN_VERTEX) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (TERRAIN_VERTEX::FVF);
		(m_bLOD) ?
			APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [m_pTileInfo [*p].uLevel][m_pTileInfo[*p].uCracks]) :
		APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [0][0]);

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
							(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1),
							0,
							(m_bLOD) ? m_pTileInfo [*p].uIndices : (m_uCellsPerTile * m_uCellsPerTile * 2) );

						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End();
			}
		}
	}
	/////////////////2
	//APROJECT_WINDOW->GetD3DDevice ()->EndScene ();

	D3DXSaveSurfaceToFileW (L"test2.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, pSurface, NULL, NULL);

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderTarget (0, pBackbuffer);
	APROJECT_WINDOW->GetD3DDevice ()->SetDepthStencilSurface (pDSBackbuffer);
	SAFE_RELEASE (pBackbuffer);
	SAFE_RELEASE (pDSBackbuffer);
	SAFE_RELEASE (pSurface);

	//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("shadowMapTexture", m_pShadowScreenMap);
	/////////////////2
	*/
}

void CAMap::Render (void)
{
	DWORD state = 0;

	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 0.6f, 0.6f, 0.6f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 1.0f);
	//CAMaterial::GetSingleton ()->SetMaterial ();

	//RestoreRenderState (true);
	//BeginRenderState ();

	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &m_matWorld);
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("View", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () );
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("Projection", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () );
	
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); ++i)
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		if (AINPUT_KEYBOARD->IsButtonDown (DIK_Y) )
			hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("Obj_Dif");
		else
			hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("Obj_DifAlp");

		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture1", m_pTexSplatting [i].pTexDiffuse);
		APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture2", m_pTexSplatting [i].pTexAlpha);

		for (vector<AUInt32>::iterator p = m_vecVisibleIndex.begin (); p != m_vecVisibleIndex.end (); ++p)
		{
			APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pTileInfo [*p].pVB, 0, sizeof (TERRAIN_VERTEX) );
			APROJECT_WINDOW->GetD3DDevice ()->SetFVF (TERRAIN_VERTEX::FVF);
			(m_bLOD) ?
				APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [m_pTileInfo [*p].uLevel][m_pTileInfo[*p].uCracks]) :
				APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_ppTileIB [0][0]);

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
									(m_uCellsPerTile + 1) * (m_uCellsPerTile + 1),
									0,
									(m_bLOD) ? m_pTileInfo [*p].uIndices : (m_uCellsPerTile * m_uCellsPerTile * 2) );

							APROJECT_WINDOW->GetLightEffect ()->EndPass ();
						}
					}
					APROJECT_WINDOW->GetLightEffect ()->End ();
				}
			}
		}
	}
//	RestoreRenderState (false);

	if (m_bRenderNormal)
		RenderNormal ();
}

void CAMap::AddSplattingMap (char* szSplatting)
{
	SPLATTING_MAP pTmp;
	pTmp.pTexAlpha = NULL;
	pTmp.pTexDiffuse = NULL;
	pTmp.szSplatting = new char [strlen (szSplatting) + 1];
	strcpy (pTmp.szSplatting, szSplatting);

	m_pTexSplatting.push_back (pTmp);
}

HRESULT	CAMap::Init (AUInt32 uLODLevel, AUInt32 uTiles, AUInt32 uCellsPerTile)
{
	Release ();

	m_uCellsPerTile	= (uCellsPerTile == 0) ? 16 : uCellsPerTile;
	m_uTiles		= uTiles;
	m_uSize			= m_uCellsPerTile * m_uTiles + 1;
	m_uLODLevel		= uLODLevel;

	D3DXMatrixScaling (&m_matScale, (AReal32)m_uTiles * 4.0f, (AReal32)m_uTiles * 4.0f, 1.0f);

	if (FAILED (D3DXCreateTexture(
			APROJECT_WINDOW->GetD3DDevice (),
			m_uSize,
			m_uSize,
			D3DX_DEFAULT,
			0,
			D3DFMT_A4R4G4B4,
			D3DPOOL_MANAGED,
			&m_pTexHeight) ) )
	{
		return E_FAIL;
	}

	if (FAILED (InitTexture () ) )				return E_FAIL;
	if (FAILED (InitAlphaTexture () ) )			return E_FAIL;
	if (FAILED (InitVertexBuffer () ) )			return E_FAIL;
	if (FAILED (InitIndexBuffer () ) )			return E_FAIL;
	if (FAILED (SetTileCenter () ) )			return E_FAIL;
	if (FAILED (InitQuadTree () ) )				return E_FAIL;
	if (FAILED (InitNormalVertexBuffer () ) )	return E_FAIL;
	if (FAILED (InitNormalIndexBuffer () ) )	return E_FAIL;

	return S_OK;
}

AReal32	CAMap::GetHeight (AReal32 fX, AReal32 fZ)
{
	AReal32 fHeight = 0.0;

	if ( (fabs (fX) > (AReal32)(m_uSize - 1) / 2.0f) ||
		 (fabs (fZ) > (AReal32)(m_uSize - 1) / 2.0f) )
	{
		return fHeight;
	}

	fX = ( (AReal32)(m_uSize - 1) / 2.0f) + fX;
	fZ = ( (AReal32)(m_uSize - 1) / 2.0f) - fZ;
	AReal32 fRows = floor (fX);
	AReal32 fCols = floor (fZ);

	AReal32 fA = m_vecVertices [ (AUInt32)fCols * m_uSize + (AUInt32)fRows ].p.y;
	AReal32 fB = m_vecVertices [ (AUInt32)fCols * m_uSize + ( (AUInt32)fRows + 1 ) ].p.y;
	AReal32 fC = m_vecVertices [ ( (AUInt32)fCols + 1 ) * m_uSize + (AUInt32)fRows ].p.y;
	AReal32 fD = m_vecVertices [ ( (AUInt32)fCols + 1 ) * m_uSize + ( (AUInt32)fRows + 1 ) ].p.y;

	AReal32 fDx = fX - fRows;
	AReal32 fDz = fZ - fCols;

	if (fDz < 1.0f - fDx)
	{
		AReal32 fU = fB - fA;
		AReal32 fV = fC - fA;

		fHeight = fA + fU * fDx + fV * fDz;
	}
	else
	{
		AReal32 fU = fC - fD;
		AReal32 fV = fB - fD;

		fHeight = fD + fU * (1.0f - fDx) + fV * (1.0f - fDz);
	}

	return fHeight;
}

D3DXVECTOR3	CAMap::Intersect ()
{
	RAY ray;

	{
		std::pair<D3DXVECTOR3, D3DXVECTOR3>	result;

		D3DXVECTOR3	vPickRayOrig;
		D3DXVECTOR3	vPickRayDir;

		D3DXMATRIXA16	matProj;
		matProj = *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix ();

		D3DXMATRIXA16	matView;
		matView = *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix ();

		POINT	pt;
		GetCursorPos (&pt);
		ScreenToClient (APROJECT_WINDOW->GetHWND (), &pt);

		D3DSURFACE_DESC pDescBackbuffer;
		LPDIRECT3DSURFACE9 pBackbuffer;
		APROJECT_WINDOW->GetD3DDevice ()->GetRenderTarget (0, &pBackbuffer);

		pBackbuffer->GetDesc (&pDescBackbuffer);

		D3DXVECTOR3	v;
		v.x	= ( ( ( 2.0f * pt.x ) / pDescBackbuffer.Width) - 1) / matProj._11;
		v.y	= -( ( ( 2.0f * pt.y ) / pDescBackbuffer.Height) - 1) / matProj._22;
		v.z	= 1.0f;

		SAFE_RELEASE (pBackbuffer);

		D3DXMATRIXA16	matViewInv;
		D3DXMatrixInverse( &matViewInv, NULL, &matView );

		D3DXVec3TransformNormal( &vPickRayDir, &v, &matViewInv );
		vPickRayOrig.x	= matViewInv._41;
		vPickRayOrig.y	= matViewInv._42;
		vPickRayOrig.z	= matViewInv._43;

		ray.Origin	= vPickRayOrig;
		ray.Dir	= vPickRayDir;
	}

	D3DXVECTOR3	v;

	bool bFind = false;
	std::vector<AReal32> vDist;
	std::multimap<AReal32, AUInt32, std::less<AReal32>> mapIntersect;

	DWORD* pIndices = NULL;
	if (SUCCEEDED (m_ppTileIB [0][0]->Lock (0, 0, (void**)&pIndices, 0) ) )
	{
		// Step 1 : 반직선 - 경계구 교차 검사
		//			반직선과 타일 경계구가 교차하는 타일만 얻어냅니다.
		for (vector<AUInt32>::iterator i = m_vecVisibleIndex.begin (); i != m_vecVisibleIndex.end (); ++i)
		{
			TERRAIN_VERTEX* pVertices = NULL;

			D3DXVECTOR3	vOrig	= D3DXVECTOR3 (ray.Origin.x, 0.0f, ray.Origin.z );
			D3DXVECTOR3	vDir	= D3DXVECTOR3 (ray.Dir.x, 0.0f, ray.Dir.z );
			D3DXVec3Normalize (&vDir, &vDir);

			AReal32 fDist = 0.0f;
			if (IntersectSphere (ray, m_pTileInfo [*i].vCenter, m_pTileInfo [*i].fRadius, &fDist, NULL) )
				mapIntersect.insert( std::pair<AReal32, AUInt32> (fDist, (*i) ) );
		}

		// Step 2 : 가까운 타일부터 반직선-삼각형 교차 검사
		//			반직선과 타일 경계구가 교차하는 타일들 중에서
		//			반직선과 교차하는 타일만 조사합니다.
		//			이 타일에서 반직선과 교차하는 가장 가까운 거리를 얻어냅니다.
		for (multimap<AReal32, AUInt32, std::less<AReal32>>::iterator i = mapIntersect.begin (); i != mapIntersect.end (); ++i )
		{
			TERRAIN_VERTEX* pVertices = NULL;
			if (SUCCEEDED (m_pTileInfo [i->second].pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
			{
				UINT	uNumFaces	= m_uCellsPerTile * m_uCellsPerTile * 2;
				FLOAT	fBary1		= 0.0f;
				FLOAT	fBary2		= 0.0f;
				FLOAT	fDist		= 0.0f;

				for (AUInt32 j = 0; j < uNumFaces; ++j)
				{
					D3DXVECTOR3		v0	= pVertices [pIndices [3 * j + 0] ].p;
					D3DXVECTOR3		v1	= pVertices [pIndices [3 * j + 1] ].p;
					D3DXVECTOR3		v2	= pVertices [pIndices [3 * j + 2] ].p;

					if (IntersectTriangle (ray, v0, v1, v2, fDist, fBary1, fBary2) )
					{
						vDist.push_back (fDist);
						bFind = true;
					}
				}
				m_pTileInfo [i->second].pVB->Unlock ();

				if (bFind)
					break;
			}
		}
		m_ppTileIB [0][0]->Unlock ();
	}

	// 교차한 거리를 가까운 거리에서 큰 거리 순서로 정렬합니다.
	sort (vDist.begin (), vDist.end (), std::less<AReal32>() );

	// 반직선과 타일이 교차한다면, 교차점을 계산합니다.
	// 그렇지 않으면, 교차점은 ( 0.0f, 0.0f, 0.0f ) 입니다.
	if (bFind)
		v = vDist [0] * ray.Dir + ray.Origin;
	else
		v = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);

	return v;
}

void CAMap::EditTerrainCircle (AReal32 fX, AReal32 fZ, AReal32 fHeightRate, AReal32 fRadius)
{
	if (fabs (fX) <= ( (AReal32)(m_uSize - 1) / 2.0f) &&
		fabs (fZ) <= ( (AReal32)(m_uSize - 1) / 2.0f) )
	{
		D3DXVECTOR3	v =	D3DXVECTOR3 (fX, 0.0f, fZ);

		fX = ( (AReal32)(m_uSize - 1) / 2.0f) + fX;
		fZ = ( (AReal32)(m_uSize - 1) / 2.0f) - fZ;
		AReal32 fRows = floor (fX);
		AReal32 fCols = floor (fZ);

		AUInt32 uStartX	= ( (fRows - fRadius ) < 0.0f ) ? 0 : (UINT)( fRows  - fRadius );
		AUInt32 uEndX	= ( (fRows + fRadius + 1 ) > ( m_uSize - 1.0f ) ) ? ( m_uSize ) : (UINT)( fRows + fRadius + 1 );
		AUInt32 uStartZ	= ( (fCols - fRadius ) < 0.0f ) ? 0 : (UINT)( fCols  - fRadius );
		AUInt32 uEndZ	= ( (fCols + fRadius + 1 ) > ( m_uSize - 1.0f ) ) ? ( m_uSize ) : (UINT)( fCols + fRadius + 1 );

		// Step 01 : 브러쉬 영역에 해당하는 지형의 높이를 편집합니다.
		for (AUInt32 z = uStartZ; z < uEndZ; ++z)
		{
			for (AUInt32 x = uStartX; x < uEndX; ++x)
			{
				D3DXVECTOR3	p		= D3DXVECTOR3 (m_vecVertices [z * m_uSize + x].p.x, 0.0f, m_vecVertices [z * m_uSize + x].p.z);
				AReal32		fDot	= D3DXVec3Dot (&(v - p), &(v - p) );

				if (fDot < fRadius * fRadius)
				{
					m_vecVertices [z * m_uSize + x].p.y += sqrt (fRadius * fRadius - fDot) * 0.01f * fHeightRate;

					if (m_vecVertices [z * m_uSize + x].p.y < 0.0f)
						m_vecVertices [z * m_uSize + x].p.y = 0.0f;
					else if (m_vecVertices [z* m_uSize + x].p.y > 3000.0f)
						m_vecVertices [z * m_uSize + x].p.y = 3000.0f;

					if ( (x > 0) && (x + 1 < m_uSize) && (z > 0) && (z + 1 < m_uSize) )
					{
						AReal32 fLeft	= m_vecVertices [ z * m_uSize + (x - 1) ].p.y;
						AReal32 fRight	= m_vecVertices [ z * m_uSize + (x + 1) ].p.y;
						AReal32 fUp		= m_vecVertices [ (z - 1) * m_uSize + x ].p.y;
						AReal32 fDown	= m_vecVertices [ (z + 1) * m_uSize + x ].p.y;

						D3DXVECTOR3	vNormal	= D3DXVECTOR3 ( (fLeft - fRight), 2.0f, (fDown - fUp) );
						D3DXVec3Normalize (&vNormal, &vNormal);

						m_vecVertices [z * m_uSize + x].n = vNormal;
					}
				}
			}
		}

		// Step 02 : 브러쉬 영역에 해당하는 지형 타일을 얻습니다.
		vector<AUInt32> vecUpdateIndex;

		for (AUInt32 uCols = 0; uCols < m_uTiles; ++uCols)
		{
			for (AUInt32 uRows = 0; uRows < m_uTiles; ++uRows)
			{
				AUInt32 uTileIndex = uCols * m_uTiles + uRows;

				if (SphereToSphereCollision (v, fRadius, m_pTileInfo [uTileIndex].vCenter, m_pTileInfo [uTileIndex].fRadius, NULL) )
					vecUpdateIndex.push_back (uTileIndex);
			}
		}

		// Step 03 : 브러쉬 영역에 해당하는 타일만 정점 데이터를 갱신합니다.
		for (vector<AUInt32>::iterator p = vecUpdateIndex.begin (); p != vecUpdateIndex.end (); ++p)
		{
			AUInt32 uRows = (*p) % m_uTiles;
			AUInt32 uCols = (*p) / m_uTiles;

			TERRAIN_VERTEX* pVertices = NULL;
			if (SUCCEEDED (m_pTileInfo [*p].pVB->Lock (0, 0, (void**)&pVertices, 0) ) )
			{
				for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
				{
					for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
					{
						AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

						pVertices->p = m_vecVertices [uVertexIndex].p;
						pVertices->n = m_vecVertices [uVertexIndex].n;
						++pVertices;
					}
				}

				m_pTileInfo [*p].pVB->Unlock ();
			}

			NORMAL_VERTEX* pNormals = NULL;
			if (SUCCEEDED (m_pTileInfo [*p].pNormalVB->Lock (0, 0, (void**)&pNormals, 0) ) )
			{
				for (AUInt32 z = 0; z < m_uCellsPerTile + 1; ++z)
				{
					for (AUInt32 x = 0; x < m_uCellsPerTile + 1; ++x)
					{
						AUInt32 uVertexIndex = (m_uCellsPerTile * uCols + z) * m_uSize + (m_uCellsPerTile * uRows + x);

						pNormals->p = m_vecVertices [uVertexIndex].p;
						++pNormals;
						pNormals->p = m_vecVertices [uVertexIndex].p + m_vecVertices [uVertexIndex].n * 0.5f;
						++pNormals;
					}
				}

				m_pTileInfo [*p].pNormalVB->Unlock ();
			}
		}
	}
}

void CAMap::SplattingTerrain (AUInt32 uIndex, AReal32 fX, AReal32 fZ, AReal32 fRadius, AReal32 fIntensity, bool bErase)
{
	if (uIndex == 0)
		return;

	if (fabs (fX) <= ( (AReal32)(m_uSize - 1) / 2.0f) &&
		fabs (fZ) <= ( (AReal32)(m_uSize - 1) / 2.0f) )
	{
		AUInt32 uAlphaMapSize	= m_uCellsPerTile * m_uTiles * 2;
		AReal32 fMapRate		= (AReal32)m_uSize / uAlphaMapSize;

		fX = ( (AReal32)(m_uSize - 1) / 2.0f) + fX;
		fZ = ( (AReal32)(m_uSize - 1) / 2.0f) - fZ;
		fX = fX / fMapRate;
		fZ = fZ / fMapRate;
		fRadius = fRadius / fMapRate;
		AReal32 fRows = floor (fX);
		AReal32 fCols = floor (fZ);

		D3DXVECTOR3	v = D3DXVECTOR3 (fX, 0.0f, fZ);

		D3DLOCKED_RECT*	d3drc;
		BYTE**			pByte;

		d3drc = new D3DLOCKED_RECT [m_pTexSplatting.size () - 1];
		pByte = new BYTE* [m_pTexSplatting.size () - 1];
		memset (pByte, NULL, sizeof (pByte) );

		for (AUInt32 i = 1; i < m_pTexSplatting.size (); ++i)
		{
			m_pTexSplatting [i].pTexAlpha->LockRect (0, &d3drc [i - 1], NULL, 0);
			pByte [i - 1] = (BYTE*)d3drc [i - 1].pBits;
		}

		AUInt32 uStartX	= ( (fRows - fRadius) < 0.0f) ? 0 : (UINT)(fRows  - fRadius);
		AUInt32 uEndX	= ( (fRows + fRadius + 1) > (uAlphaMapSize - 1) ) ? (uAlphaMapSize) : (AUInt32)(fRows + fRadius + 1);
		AUInt32 uStartZ	= ( (fCols - fRadius) < 0.0f) ? 0 : (UINT)(fCols  - fRadius);
		AUInt32 uEndZ	= ( (fCols + fRadius + 1) > (uAlphaMapSize - 1) ) ? (uAlphaMapSize) : (AUInt32)(fCols + fRadius + 1);

		for (AUInt32 z = uStartZ; z < uEndZ; ++z)
		{
			for (AUInt32 x = uStartX; x < uEndX; ++x)
			{
				D3DXVECTOR3	p		= D3DXVECTOR3 ( (AReal32)x, 0.0f, (AReal32)z);
				AReal32		fDot	= D3DXVec3Dot (&(v - p), &(v - p) );

				if (fDot < fRadius * fRadius)
				{
					AUInt32	uPixelIndex	= z * d3drc [uIndex - 1].Pitch + x * 4;
					BYTE	byteData	= 0x00;

					// Step 01 : 반영될 Alpha 값을 구합니다.
					//			 만약 이미 반영된 값이 현재 값보다 크다면,
					//			 Alpha 값을 반영하지 않습니다.
					if (bErase)
					{
						byteData = 0x00;
						if (pByte [uIndex - 1][uPixelIndex] < byteData)
							continue;
					}
					else
					{
						byteData = (BYTE)(0xFF * (sqrt (fRadius * fRadius - fDot) / fRadius) * fIntensity);
						if (pByte [uIndex - 1][uPixelIndex] > byteData)
							continue;
					}

					// Step 02 : Alpha 값을 적용합니다.
					//			 사용된 Splatting 텍스쳐가 자연스럽게 혼합되도록
					//			 각 Alpha 텍스쳐에 Alpha 값을 적용합니다.
					for (AUInt32 i = 0; i < m_pTexSplatting.size () - 1; ++ i)
					{
						BYTE byteTemp = byteData;

						if ( (i != (uIndex - 1) ) && (bErase == false) )
						{
							(pByte [i][uPixelIndex + 0] < 0xFF - byteData) ?
								byteTemp = pByte [i][uPixelIndex + 0] :
								byteTemp = 0xFF - byteData;
						}

						for (AUInt32 j = 0; j < 4; ++j)
							pByte [i][uPixelIndex + j] = byteTemp;
					}
				}
			}
		}

		for (AUInt32 i = 1; i < m_pTexSplatting.size (); ++i)
			m_pTexSplatting [i].pTexAlpha->UnlockRect (0);

		SAFE_DELETE (d3drc);
		SAFE_DELETE (pByte);
	}
}

bool ReturnFunction (A16byte,A16byte,bool)
{
	return true;
}

HRESULT	CAMap::SaveTerrain (char* szMapFile, char* szPassword)
{
	AU4byte size = m_pTexSplatting.size ();
	AU4byte totalSize = 0;
	CASecuFS::GetSingleton ()->DestoryVolume (szMapFile);

	FILE* fp = fopen ("./Temp/Information.map", "wb");
	fwrite (&size, sizeof (AU4byte), 1, fp);
	fwrite (&m_uCellsPerTile, sizeof (AU4byte), 1, fp);
	totalSize += ftell (fp);
	fclose (fp);

	if (FAILED (SaveHeightMap ("./Temp/HeightMap.bmp") ) )
		return E_FAIL;

	fp = fopen ("./Temp/HeightMap.bmp", "rb");
	fseek (fp, 0, SEEK_END);
	totalSize += ftell (fp);
	fclose (fp);

	if (FAILED (SaveDiffuseMap () ) )
		return E_FAIL;

	if (FAILED (SaveAlphaMap () ) )
		return E_FAIL;

	char pStr [256];
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); i ++)
	{
		sprintf (pStr, "./Temp/Diffuse%u.png", i);
		fp = fopen (pStr, "rb");
		fseek (fp, 0, SEEK_END);
		totalSize += ftell (fp);
		fclose (fp);

		if (i > 0)
		{
			sprintf (pStr, "./Temp/Alpha%u.png", i);
			fp = fopen (pStr, "rb");
			fseek (fp, 0, SEEK_END);
			totalSize += ftell (fp);
			fclose (fp);
		}
	}

	SHANDLE sVolume = CASecuFS::GetSingleton ()->CreateVolume (szMapFile, szPassword, (AU4byte)(totalSize * 1.2f), 4);
	CASecuFS::GetSingleton ()->ImportFile (sVolume, "/", "./Temp/Information.map", &ReturnFunction);
	CASecuFS::GetSingleton ()->ImportFile (sVolume, "/", "./Temp/HeightMap.bmp", &ReturnFunction);
	CASecuFS::GetSingleton ()->CreateDirectoryEx (sVolume, "/", "AlphaMap");
	CASecuFS::GetSingleton ()->CreateDirectoryEx (sVolume, "/", "TileMap");
	for (AUInt32 i = 0; i < m_pTexSplatting.size (); i ++)
	{
		sprintf (pStr, "./Temp/Diffuse%u.png", i);
		CASecuFS::GetSingleton ()->ImportFile (sVolume, "/TileMap/", pStr, &ReturnFunction);

		if (i > 0)
		{
			sprintf (pStr, "./Temp/Alpha%u.png", i);
			CASecuFS::GetSingleton ()->ImportFile (sVolume, "/AlphaMap/", pStr, &ReturnFunction);
		}
	}
	CASecuFS::GetSingleton ()->CloseVolume (&sVolume);

	return S_OK;
}

HRESULT	CAMap::LoadTerrain (char* szMapFile, char* szPassword)
{
	AU4byte size;
	void* pMapData;
	AU4byte totalSize = 0;

	for (AUInt32 i = 0; i < m_pTexSplatting.size (); ++i)
	{
		SAFE_RELEASE (m_pTexSplatting [i].pTexAlpha);
		SAFE_RELEASE (m_pTexSplatting [i].pTexDiffuse);
		SAFE_DELETE (m_pTexSplatting [i].szSplatting);
	}
	m_pTexSplatting.clear ();

	Release();

	// Information Data Load
	SHANDLE sVolume = CASecuFS::GetSingleton ()->OpenVolume (szMapFile, szPassword);
	AHANDLE afile = CASecuFS::GetSingleton ()->OpenFile (sVolume, "/Information.map");
	CASecuFS::GetSingleton ()->ReadFile (&size, 4, afile);
	CASecuFS::GetSingleton ()->ReadFile (&m_uCellsPerTile, 4, afile);
	CASecuFS::GetSingleton ()->CloseFile (&afile);
	for (AUInt32 i = 0; i < size; i ++)
	{
		SPLATTING_MAP pTmp;
		pTmp.pTexAlpha = NULL;
		pTmp.pTexDiffuse = NULL;
		pTmp.szSplatting = NULL;
		m_pTexSplatting.push_back (pTmp);
	}

	// Height Map
	afile = CASecuFS::GetSingleton ()->OpenFile (sVolume, "/HeightMap.bmp");
	CASecuFS::GetSingleton ()->SeekFile (afile, 0, FileSeekEnd);
	size = CASecuFS::GetSingleton ()->GetFileOffset (afile).ConvertToInt32 ();
	pMapData = (void*)new char [size];
	CASecuFS::GetSingleton ()->SeekFile (afile, 0, FileSeekSet);
	CASecuFS::GetSingleton ()->ReadFile (pMapData, size, afile);
	CASecuFS::GetSingleton ()->CloseFile (&afile);

	if (FAILED (LoadHeightMap (pMapData, size) ) )
		return E_FAIL;

	SAFE_DELETE (pMapData);

	if (FAILED (LoadDiffuseMap (sVolume) ) )
	{
		CASecuFS::GetSingleton ()->CloseVolume (&sVolume);
		return E_FAIL;
	}

	if (FAILED (LoadAlphaMap (sVolume) ) )
	{
		CASecuFS::GetSingleton ()->CloseVolume (&sVolume);
		return E_FAIL;
	}

	CASecuFS::GetSingleton ()->CloseVolume (&sVolume);

	if (FAILED (InitVertexBuffer () ) )
		return E_FAIL;

	if (FAILED (InitIndexBuffer () ) )
		return E_FAIL;

	if (FAILED (SetTileCenter () ) )
		return E_FAIL;

	if (FAILED (InitQuadTree () ) )
		return E_FAIL;

	if (FAILED (InitNormalVertexBuffer () ) )
		return E_FAIL;

	if (FAILED (InitNormalIndexBuffer () ) )
		return E_FAIL;

	if (FAILED (ComputNormal () ) )
		return E_FAIL;

	return S_OK;
}

D3DXVECTOR3	CAMap::GetNormal (AReal32 fX, AReal32 fZ)
{
	D3DXVECTOR3 vNormal = D3DXVECTOR3 (0.0f, 1.0f, 0.0f);

	if ( (fabs (fX) > (AReal32)(m_uSize - 1) / 2.0f) ||
		 (fabs (fZ) > (AReal32)(m_uSize - 1) / 2.0f) )
	{
		return vNormal;
	}

	fX = ( (AReal32)(m_uSize - 1) / 2.0f) + fX;
	fZ = ( (AReal32)(m_uSize - 1) / 2.0f) + fZ;
	AReal32 fRows = floor (fX);
	AReal32 fCols = floor (fZ);

	D3DXVECTOR3 vA = m_vecVertices [ (AUInt32)fCols * m_uSize + (AUInt32)fRows].n;
	D3DXVECTOR3 vB = m_vecVertices [ (AUInt32)fCols * m_uSize + ( (AUInt32)fRows + 1)].n;
	D3DXVECTOR3 vC = m_vecVertices [ ( (AUInt32)fCols + 1 ) * m_uSize + (AUInt32)fRows].n;
	D3DXVECTOR3 vD = m_vecVertices [ ( (AUInt32)fCols + 1 ) * m_uSize + ( (AUInt32)fRows + 1)].n;

	AReal32 fDx = fX - fRows;
	AReal32 fDz = fZ - fCols;

	if (fDz < 1.0f - fDx)
	{
		vNormal = vA + vB + vC;
		D3DXVec3Normalize (&vNormal, &vNormal);
	}
	else
	{
		vNormal = vB + vC + vD;
		D3DXVec3Normalize (&vNormal, &vNormal);
	}

	return vNormal;
}