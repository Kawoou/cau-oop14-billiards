#include "AModelInclude.h"

CAModel::CAModel (void)
{
	D3DXMatrixIdentity (&m_matTM);

	m_nObjCount		= 0;
	m_nMeshCount	= 0;
	m_fAniStart		= 0.0f;
	m_fAniEnd		= 0.0f;
	m_nKeyType		= 0;
	m_nSkinType		= 0;

	m_strFile.clear ();

	m_nSkinningMode = 0;
}

CAModel::~CAModel (void)
{
	Unload ();
}

bool CAModel::Create (const char* strFileName, int nSkinningMode)
{
	return Load (strFileName, nSkinningMode);
}

bool CAModel::Load (const char* strFileName, int nSkinningMode)
{
	Unload ();

	m_strFile = strFileName;
	m_nSkinningMode = nSkinningMode;

	return CAModelLoader::GetSingleton ()->Load (this, strFileName, nSkinningMode);
}

void CAModel::Unload ()
{
	for (int i = 0; i != (int)m_listMaterial.size (); ++i)
		SAFE_DELETE (m_listMaterial [i]);
	m_listMaterial.clear ();

	for (int i = 0; i != (int)m_listObject.size (); ++i)
		SAFE_DELETE (m_listObject[i]);
	m_listObject.clear ();

	m_listMesh.clear ();
	m_listBone.clear ();
}

void CAModel::SetSkinngMode (int nSkinningMode)
{
	if (m_strFile.empty () )
		return;

	Load (m_strFile.data (), nSkinningMode);
}

void CAModel::AddMaterial (CAModelMaterial* pMaterial)
{	
	m_listMaterial.push_back (pMaterial);
}

void CAModel::AddObject (CAModelObject* pObject, char* strClass)
{
	m_listObject.push_back (pObject);

	if (strcmp (strClass, "Editable_mesh") == 0)			m_listMesh.push_back( (CAModelMesh*)pObject);
	else if (strcmp (strClass, "Bone") == 0)				m_listBone.push_back( (CAModelBone*)pObject);
	else if (strcmp (strClass, "Biped_Object") == 0)		m_listBone.push_back( (CAModelBone*)pObject);
	else if (strcmp (strClass, "BoneGeometry") == 0)		m_listBone.push_back( (CAModelBone*)pObject);
}

D3DXMATRIX* CAModel::GetObjectTM (int nObjectID)
{
	for (int i = 0; i != (int)m_listObject.size (); ++i)
	{
		if (m_listObject [i]->GetObjectID () == nObjectID)
			return m_listObject [i]->GetTM ();
	}

	return NULL;
}

D3DXMATRIX* CAModel::GetBoneTM (int nBoneID)
{
	return m_listBone [nBoneID-1]->GetBoneTM ();
}

void CAModel::Reset (void)
{
	Load (m_strFile.data (), m_nSkinningMode);
}

// 수정 필요 (예정: 애니메이션 관련 정보를 클래스 내부에 포함)
void CAModel::Update (float fFrame)
{
	int i;
	D3DXMATRIX* pTM = NULL;

	for (i = 0; i != (int)m_listBone.size (); ++i)
	{
		pTM = GetObjectTM (m_listBone [i]->GetParentID () );
		m_listBone [i]->Update (fFrame, pTM);
	}

	for (i = 0; i != (int)m_listMesh.size (); ++i)
	{
		pTM = GetObjectTM (m_listMesh [i]->GetParentID () );
		m_listMesh [i]->Update (fFrame, pTM);
	}
}

void CAModel::Update (void)
{
	static float fCurFrame = 0.0f;
	fCurFrame += 33.33333f * 0.03f;
	if(fCurFrame > this->GetEndFrame () )
		fCurFrame = 0.0f;

	this->Update (fCurFrame);

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
}

void CAModel::Render (void)
{

	DWORD state = 0;

	CAMaterial::GetSingleton ()->SetAmbient (1.0f, 0.6f, 0.6f, 0.6f);
	CAMaterial::GetSingleton ()->SetDiffuse (1.0f, 1.0f, 1.0f, 1.0f);
	
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("View", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () );
	APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("Projection", APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () );

	for (AUInt32 i = 0; i < (int)m_listObject.size (); ++i)
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		//if (AINPUT_KEYBOARD->IsButtonDown (DIK_Y) )
		//else
		//	hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("Obj_DifAlp");

		//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture2", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pOpacityTexture);
		//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture3", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pHeightTexture);
		//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture6", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pSpecularTexture);
		//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture7", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pLuminanceTexture);
		//APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture8", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pBumpTexture);

		if(m_listObject[i]->Render_Front () )
		{
			int nCurlingCount = 0;
			D3DXMATRIXA16 matComplate = m_matWorld;// * *m_listObject[i]->GetTM ();
			D3DXMATRIXA16 matPosition;
			{
				//m_listObject[i]->
				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMin ()->x, m_listObject[i]->GetBBoxMin ()->y, m_listObject[i]->GetBBoxMin ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMin ()->x, m_listObject[i]->GetBBoxMin ()->y, m_listObject[i]->GetBBoxMax ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMin ()->x, m_listObject[i]->GetBBoxMax ()->y, m_listObject[i]->GetBBoxMin ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMin ()->x, m_listObject[i]->GetBBoxMax ()->y, m_listObject[i]->GetBBoxMax ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMax ()->x, m_listObject[i]->GetBBoxMin ()->y, m_listObject[i]->GetBBoxMin ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMax ()->x, m_listObject[i]->GetBBoxMin ()->y, m_listObject[i]->GetBBoxMax ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMax ()->x, m_listObject[i]->GetBBoxMax ()->y, m_listObject[i]->GetBBoxMin ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );

				D3DXMatrixTranslation (&matPosition, m_listObject[i]->GetBBoxMax ()->x, m_listObject[i]->GetBBoxMax ()->y, m_listObject[i]->GetBBoxMax ()->z);
				matPosition = matPosition * matComplate;
				nCurlingCount += APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetFrustum ()->IsIn (&D3DXVECTOR3 (matPosition._41, matPosition._42, matPosition._43) );
			}
			if(nCurlingCount == 0) continue;

			//DebugConsoleLog(DebugMsg, "%d", m_listObject[i]->GetMaterialID ());

			APROJECT_WINDOW->GetLightEffect ()->SetMatrix ("World", &matComplate);
			APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_Material);
			APROJECT_WINDOW->GetLightEffect ()->SetTexture ("Texture1", m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pDiffuseTexture);

			hTechnique = APROJECT_WINDOW->GetLightEffect ()->GetTechniqueByName("Obj_Dif");
			if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->SetTechnique (hTechnique) ) )
			{
				if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->Begin (&totalPasses, 0) ) )
				{
					for (UINT pass = 0; pass < totalPasses; ++pass)
					{
						if (SUCCEEDED (APROJECT_WINDOW->GetLightEffect ()->BeginPass (pass) ) )
						{
							m_listObject [i]->Render ();
							APROJECT_WINDOW->GetLightEffect ()->EndPass ();
						}
					}
					APROJECT_WINDOW->GetLightEffect ()->End ();
				}
			}
		}
	}

}
/*
void CAModel::Render (void)
{
	DWORD dwNormalize;
	DWORD dwMinfilter;
	DWORD dwMagfilter;
	DWORD dwMipfilter;

	DWORD dwAlphaBlend;
	DWORD dwSrcblend;
	DWORD dwDestblend;

	// Get Original Data
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_NORMALIZENORMALS, &dwNormalize);
	APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (0, D3DSAMP_MINFILTER, &dwMinfilter);
	APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (0, D3DSAMP_MAGFILTER, &dwMagfilter);
	APROJECT_WINDOW->GetD3DDevice ()->GetSamplerState (0, D3DSAMP_MIPFILTER, &dwMipfilter);

	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_SRCBLEND, &dwSrcblend);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_DESTBLEND, &dwDestblend);

	// Set New Data
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_NORMALIZENORMALS, TRUE);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ZERO);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ZERO);

	D3DXMATRIX matOriWorld;
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;
	D3DXMATRIX matRotX;
	D3DXMATRIX matRotY;
	D3DXMATRIX matRotZ;
	D3DXMATRIX matComplate;

	APROJECT_WINDOW->GetD3DDevice ()->GetTransform (D3DTS_WORLD, &matOriWorld);

	D3DXMatrixTranslation (&matPosition, m_vPosition.x, m_vPosition.y, m_vPosition.z);
	D3DXMatrixScaling (&matSize, m_vSize.x, m_vSize.y, m_vSize.z);
	D3DXMatrixRotationX (&matRotX, m_fRotX);
	D3DXMatrixRotationY (&matRotY, m_fRotY);
	D3DXMatrixRotationZ (&matRotZ, m_fRotZ);
	matComplate = matOriWorld * matSize * matRotX * matRotY * matRotZ * matPosition;
	for (int i = 0; i != (int)m_listObject.size (); ++i)
	{		
		APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_Material);
		APROJECT_WINDOW->GetD3DDevice ()->SetTexture (0, m_listMaterial [m_listObject [i]->GetMaterialID () ]->m_pDiffuseTexture);

		// 수정 필요 (예정, CAGeometry처럼 이동 및 크기 조정을 간편하게 하도록 수정)
		APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &matComplate);

		m_listObject [i]->Render ();
	}
	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &matOriWorld);

	// Set Original Data
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_NORMALIZENORMALS, dwNormalize);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MINFILTER, dwMinfilter);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MAGFILTER, dwMagfilter);
	APROJECT_WINDOW->GetD3DDevice ()->SetSamplerState (0, D3DSAMP_MIPFILTER, dwMipfilter);

	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, dwSrcblend);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, dwDestblend);
}
*/