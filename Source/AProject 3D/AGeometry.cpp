#include "AWindow.h"

CAGeometry::CAGeometry (DWORD dwFVF, CustomVertex* pVertices, CustomIndex* pIndices, int nVC, int nIC)
{
	m_pTexture = NULL;

	m_bFrustumCulling = true;
	m_bFrustumState = false;

	m_nVertexCount = nVC;
	m_nIndexCount = nIC;

	m_pVertices = new CustomVertex [nVC * sizeof (CustomVertex)];
	memcpy (m_pVertices, pVertices, nVC * sizeof (CustomVertex) );

	if (pIndices != NULL)
	{
		m_pIndices = new CustomIndex[nIC * sizeof (CustomIndex)];
		memcpy(m_pIndices, pIndices, nIC * sizeof (CustomIndex));
	}
	else
		m_pIndices = NULL;

	m_nPrimitiveType = D3DPT_TRIANGLELIST;

#ifdef AUsingTangent
	m_FVF = 0;
#else
	#if AUsingRHW == 1
		m_FVF = D3DFVF_XYZRHW;
	#elif AUsingRHW == 2
		m_FVF = D3DFVF_XYZW;
	#else
		m_FVF = D3DFVF_XYZ;
	#endif
	#if AUsingColor == 1
		m_FVF |= D3DFVF_DIFFUSE;
	#elif AUsingColor == 2
		m_FVF |= D3DFVF_SPECULAR;
	#endif
	#if ATextureCount > 0
		m_FVF |= (D3DFVF_TEX1 * ATextureCount);
	#endif
	#ifdef AUsingVectorNormal
		m_FVF |= D3DFVF_NORMAL;
	#endif
#endif
	m_FVF |= dwFVF;

	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
}

CAGeometry::~CAGeometry (void)
{
	// 연결된 텍스쳐 제거
	SAFE_DELETE (m_pTexture);

	// 인덱스 버퍼 제거
	DebugConsoleLog (DebugClear, "CAGeometry: 1) Index buffer Destroy");
	SAFE_DELETE (m_pIndices);
	SAFE_RELEASE (m_pIndexBuffer);

	// 버텍스 버퍼 제거
	DebugConsoleLog (DebugClear, "CAGeometry: 2) Vertex buffer Destroy");
	SAFE_DELETE (m_pVertices);
	SAFE_RELEASE (m_pVertexBuffer);
}

bool CAGeometry::GetPicking (RAY Ray)
{
	float t,u,v;

	for (int i = 0; i < m_nIndexCount; ++i)
	{
		D3DXVECTOR3 v0;
		D3DXVECTOR3 v1;
		D3DXVECTOR3 v2;
		D3DXVec3TransformCoord (&v0, &D3DXVECTOR3 (m_pVertices [m_pIndices [i]._0].x, m_pVertices [m_pIndices [i]._0].y, m_pVertices [m_pIndices [i]._0].z), &m_matComplate);
		D3DXVec3TransformCoord (&v1, &D3DXVECTOR3 (m_pVertices [m_pIndices [i]._1].x, m_pVertices [m_pIndices [i]._1].y, m_pVertices [m_pIndices [i]._1].z), &m_matComplate);
		D3DXVec3TransformCoord (&v2, &D3DXVECTOR3 (m_pVertices [m_pIndices [i]._2].x, m_pVertices [m_pIndices [i]._2].y, m_pVertices [m_pIndices [i]._2].z), &m_matComplate);

		if (IntersectTriangle(Ray, v0, v1, v2, t, u, v) )
		{
			//g_PickVer[0] = g_Vertices[i];
			//g_PickVer[1] = g_Vertices[i+1];
			//g_PickVer[2] = g_Vertices[i+2];

			//g_pd3dDevice->CreateVertexBuffer(3*sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_PickVB,NULL);

			/*VOID* pVertices;
			if( FAILED( g_PickVB->Lock( 0, sizeof(g_PickVer), (void**)&pVertices, 0 ) ) )
				return E_FAIL;
			memcpy( pVertices, g_PickVer, sizeof(g_PickVer) );
			g_PickVB->Unlock();	*/
			return true;
		}
	}
	return false;
}

void CAGeometry::SetFVF (DWORD dwFVF)
{
	m_FVF = dwFVF;
}

DWORD CAGeometry::GetFVF (void)
{
	return m_FVF;
}

void CAGeometry::SetTexture (CATexture* pTexture)
{
	m_pTexture = pTexture;
}

CATexture* CAGeometry::GetTexture (void)
{
	return m_pTexture;
}

void CAGeometry::SetVertex (CustomVertex* pVertices, int nVC)
{
	SAFE_DELETE_ARRAY (m_pVertices);
	m_nVertexCount = nVC;
	m_pVertices = new CustomVertex [nVC * sizeof (CustomVertex)];
	memcpy (m_pVertices, pVertices, nVC * sizeof (CustomVertex) );
}

void CAGeometry::SetIndex (CustomIndex* pIndices, int nIC)
{
	SAFE_DELETE_ARRAY (m_pIndices);
	m_nIndexCount = nIC;
	m_pIndices = new CustomIndex [nIC * sizeof (CustomIndex)];
	memcpy (m_pIndices, pIndices, nIC * sizeof (CustomIndex) );
}

void CAGeometry::SetPrimitiveType (D3DPRIMITIVETYPE type)
{
	m_nPrimitiveType = type;
}

CustomVertex* CAGeometry::GetVertex (void)
{
	return m_pVertices;
}

CustomIndex* CAGeometry::GetIndex (void)
{
	return m_pIndices;
}

int CAGeometry::GetVertexCount (void)
{
	return m_nVertexCount;
}

int CAGeometry::GetIndexCount (void)
{
	return m_nIndexCount;
}

D3DPRIMITIVETYPE CAGeometry::GetPrimitiveType (void)
{
	return m_nPrimitiveType;
}

HRESULT CAGeometry::Create (void)
{
	SAFE_RELEASE (m_pVertexBuffer);
	SAFE_RELEASE (m_pIndexBuffer);

	// Vertex Buffer
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (m_nVertexCount * sizeof (CustomVertex), 0, m_FVF,
	D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL) ) )
	{
		DebugLog ("Create(Create vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CAGeometry: Create(Create vertex buffer Error) 실패!");

		return AE_ERROR_CREATE_VERTEXBUFFER;
	}

	VOID* pVertices;
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof(*m_pVertices) * m_nVertexCount, (void**)&pVertices, 0) ) )
	{
		DebugLog ("Create(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CAGeometry: Create(Lock vertex buffer Error) 실패!");

		return AE_ERROR_LOCK_VERTEXBUFFER;
	}
	memcpy (pVertices, m_pVertices, sizeof (*m_pVertices) * m_nVertexCount);
	m_pVertexBuffer->Unlock ();

	// Index Buffer
	if (m_pIndices != NULL)
	{
		if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateIndexBuffer (m_nIndexCount * sizeof (CustomIndex), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL) ) )
		{
			DebugLog ("Create(Create index buffer Error) 실패!");
			DebugConsoleLog (DebugError, "CAGeometry: Create(Create index buffer Error) 실패!");

			return AE_ERROR_CREATE_INDEXBUFFER;
		}
		
		VOID* pIndices;
		if (FAILED (m_pIndexBuffer->Lock (0, sizeof (*m_pIndices) * m_nIndexCount, (void**) &pIndices, 0) ) )
		{
			DebugLog ("Create(Lock index buffer Error) 실패!");
			DebugConsoleLog (DebugError, "CAGeometry: Create(Lock index buffer Error) 실패!");

			return AE_ERROR_LOCK_INDEX_BUFFER;
		}
		memcpy (pIndices, m_pIndices, sizeof (*m_pIndices) * m_nIndexCount);
		m_pIndexBuffer->Unlock ();
	}

	DebugConsoleLog (DebugClear, "CAGeometry: Initialize Geometry, the success");

	return AE_SUCCESS;
}

void CAGeometry::Reset (void)
{
	// 연결된 텍스쳐를 초기화한다.
	m_pTexture->Reset ();

	Create ();
}

void CAGeometry::Update (void)
{
	int i, j;

	// 연결된 텍스쳐를 업데이트한다.
	if (m_pTexture && m_pTexture->GetTexture())
		m_pTexture->Update ();

	CustomVertex* pVertices;
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof(*m_pVertices) * m_nVertexCount, (void**)&pVertices, 0) ) )
	{
		DebugLog ("Create(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CAGeometry: Create(Lock vertex buffer Error) 실패!");

		return;
	}

	D3DXMATRIXA16 matSize;
	D3DXMATRIXA16 matNewPosition;
	D3DXMATRIXA16 matRotX, matRotY, matRotZ;
	D3DXMatrixScaling (&matSize, m_vSize.x, m_vSize.y, m_vSize.z);
	D3DXMatrixTranslation (&matNewPosition, m_vPosition.x, m_vPosition.y, m_vPosition.z);
	D3DXMatrixRotationX (&matRotX, m_fRotX);
	D3DXMatrixRotationY (&matRotY, m_fRotY);
	D3DXMatrixRotationZ (&matRotZ, m_fRotZ);
	m_matComplate = matSize * matNewPosition * matRotX * matRotY * matRotZ;

	int nCurlingCount = 0;
	for (i = 0; i < m_nVertexCount; i ++)
	{
		pVertices [i].x = m_pVertices [i].x + m_vOffset.x;
		pVertices [i].y = m_pVertices [i].y + m_vOffset.y;
		pVertices [i].z = m_pVertices [i].z + m_vOffset.z;

#if AUsingRHW == 1
		pVertices [i].rhw = m_pVertices [i].rhw;
#elif AUsingRHW == 2
		pVertices [i].w = m_pVertices [i].w;
#endif

#ifdef AUsingVectorNormal
		pVertices [i].nx = m_pVertices [i].nx;
		pVertices [i].ny = m_pVertices [i].ny;
		pVertices [i].nz = m_pVertices [i].nz;
#endif
#if AUsingColor == 1
		pVertices [i].diffColor = m_pVertices [i].diffColor;
#elif AUsingColor == 2
		pVertices [i].specColor = m_pVertices [i].specColor;
#endif

#if ATextureCount != 0
		for (j = 0; j < ATextureCount; j ++)
		{
			pVertices [i].tUV [j].tU = m_pVertices [i].tUV [j].tU;
			pVertices [i].tUV [j].tV = m_pVertices [i].tUV [j].tV;
		}
#endif
		
		D3DXMATRIXA16 matPosition;
		D3DXMatrixTranslation (&matPosition, pVertices [i].x, pVertices [i].y, pVertices [i].z);
		matPosition = matPosition * m_matComplate;
		
		nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );
	}
	m_pVertexBuffer->Unlock ();

	if (nCurlingCount != 0)
		m_bFrustumState = true;
	else
		m_bFrustumState = false;
}

void CAGeometry::Render (void)
{
	if (!m_bFrustumState)
		return;

	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &m_matComplate);
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("View", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () );
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("Projection", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () );
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		if (m_pTexture)
		{
			if (m_pTexture->GetTexture())
			{
				hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("Obj_Dif");

				APROJECT_WINDOW->GetLightEffect()->SetTexture("Texture1", m_pTexture->GetTexture());
			}
			else
			{
				hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("Obj_OriNone");

				stMaterial material = {
					D3DXCOLOR(1.0f),
					D3DXCOLOR(m_pTexture->GetColor().r, m_pTexture->GetColor().g, m_pTexture->GetColor().b, m_pTexture->GetColor().a),
					D3DXCOLOR(1.0f),
					D3DXCOLOR(1.0f),
					1.0f
				};
				APROJECT_WINDOW->GetLightEffect()->SetValue("g_mMaterial", &material, sizeof (material));
			}
		}
		else
		{
			hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("Obj_None");

			stMaterial material = {
				D3DXCOLOR(1.0f),
				D3DXCOLOR(1.0f),
				D3DXCOLOR(1.0f),
				D3DXCOLOR(1.0f),
				1.0f
			};
			APROJECT_WINDOW->GetLightEffect()->SetValue("g_mMaterial", &material, sizeof (material));
		}

		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (m_nLayer, m_pVertexBuffer, 0, sizeof (CustomVertex) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (m_FVF);
		if (m_pIndexBuffer)
			APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIndexBuffer);

		if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
		{
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
					{
						if (m_pIndexBuffer)
							APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (m_nPrimitiveType, 0, 0, m_nVertexCount, 0, m_nIndexCount);
						else
							APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (m_nPrimitiveType, 0, m_nVertexCount);

						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End ();
			}
		}
	}
}

/*
void CAGeometry::Render (void)
{
	D3DXMATRIX matOriWorld;
	D3DXMATRIX matComplate;

	if (!m_bFrustumState)
		return;

	// 연결된 텍스쳐를 그린다.
	m_pTexture->Render ();

	// 현재 월드 행렬을 얻어옴
	APROJECT_WINDOW->GetD3DDevice ()->GetTransform (D3DTS_WORLD, &matOriWorld);

	// 현재 월드 행렬에 객체에서 필요한 행렬들을 곱함
	matComplate = matOriWorld * m_matComplate;

	// 월드 행렬을 변경시킴
	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &matComplate);
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;

		APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldMatrix", &matComplate);
		APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldViewProjectionMatrix", &(matComplate * *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () * 
			*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () ) );

		D3DXMATRIX matInverseTranspose;
		D3DXMatrixInverse (&matInverseTranspose, 0, &matComplate);
		D3DXMatrixTranspose (&matInverseTranspose, &matInverseTranspose);

		APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("worldInverseTransposeMatrix", &matInverseTranspose);

		hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("NoMapping");

		HRESULT hr = APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (m_nLayer, m_pVertexBuffer, 0, sizeof (CustomVertex) );
		hr = APROJECT_WINDOW->GetD3DDevice ()->SetFVF (m_FVF);
		hr = APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIndexBuffer);
						
		if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
		{
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
			{
				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
					{
						hr = APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, m_nVertexCount, 0, m_nIndexCount);
						
						APROJECT_WINDOW->GetLightEffect ()->EndPass ();
					}
				}
				APROJECT_WINDOW->GetLightEffect ()->End();
			}
		}
		hr = APROJECT_WINDOW->GetD3DDevice ()->SetTexture (m_nLayer, NULL);
	}

	// 월드 행렬을 원래 행렬로 변환
	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &matOriWorld);
}*/