#include "AModelInclude.h"

void CAMeshSHSkin::CreateEffect ()
{
	std::string strEffect = "./Data/skin.fx";

	if (FAILED (D3DXCreateEffectFromFileA (APROJECT_WINDOW->GetD3DDevice (), strEffect.data (), NULL, NULL, 0, NULL, &m_pEffect, NULL) ) )
	{
		DebugLog ("CreateEffect(Could not find %s) 실패!", strEffect.data () );
		DebugConsoleLog (DebugError, "CAMeshSHSkin: CreateEffect(Could not find %s) 실패!", strEffect.data () );
		m_pEffect = NULL;
		return;
	}

	// only one technique
	D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName ("TShader");
	m_pEffect->SetTechnique (hTechnique);

	DebugConsoleLog (DebugClear, "CAMeshSHSkin: Create Effect Shader, the success");
}

void CAMeshSHSkin::CreateVB (const CAModelVIB& modelVIB)
{
	vector<CAVertex> listVertex;
	listVertex.resize (m_nNumVertices);

	for (int i = 0; i != (int)listVertex.size (); ++i)
	{
		// Pos
		listVertex [i].pos = modelVIB.m_listVertex [i];

		// Normal
		if (!modelVIB.m_listNormal.empty () )
			listVertex [i].normal = modelVIB.m_listNormal [i];
		else
			listVertex [i].normal = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);

		// UV Texture
		if (!modelVIB.m_listUV.empty () )
		{
			int nUVIndex = 0;

			if (!modelVIB.m_mapIndex.empty () )
			{
				map<int,int>::const_iterator itor = modelVIB.m_mapIndex.find (i);
				nUVIndex = itor->second;
			}
			else
				nUVIndex = i;

			listVertex [i].uv = modelVIB.m_listUV [nUVIndex];
		}
		else
			listVertex [i].uv = D3DXVECTOR2 (0.0f, 0.0f);

		// Bone Index (ID)
		DWORD dwIndex = modelVIB.m_listWeight [i].m_dwWeightIndex;
		listVertex [i].bone_index.x = (BYTE)dwIndex; dwIndex >>= 8;
		listVertex [i].bone_index.y = (BYTE)dwIndex; dwIndex >>= 8;
		listVertex [i].bone_index.z = (BYTE)dwIndex; dwIndex >>= 8;
		listVertex [i].bone_index.w = (BYTE)dwIndex;
		for (int j = 0; j != (int)modelVIB.m_listWeight [i].m_listBoneID.size (); ++j)
			m_listBoneID.insert (modelVIB.m_listWeight [i].m_listBoneID [j]);

		// Weight
		listVertex [i].weight.x = modelVIB.m_listWeight [i].m_listWeight [0];
		listVertex [i].weight.y = modelVIB.m_listWeight [i].m_listWeight [1];
		listVertex [i].weight.z = modelVIB.m_listWeight [i].m_listWeight [2];
		listVertex [i].weight.w = modelVIB.m_listWeight [i].m_listWeight [3];
	}

	DebugConsoleLog (DebugMsg, "CAMeshSHSkin: Bone Count / Mesh : %d", m_listBoneID.size () );

	DWORD dwUsage = APROJECT_WINDOW->GetD3DDevice ()->GetSoftwareVertexProcessing () ? D3DUSAGE_SOFTWAREPROCESSING : 0;
	APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (m_nNumVertices * sizeof (CAVertex), dwUsage, m_dwFVF, D3DPOOL_DEFAULT, &m_pVB, NULL);

	void* pVertex = NULL;
	m_pVB->Lock (0, 0, (void**)&pVertex, 0);
	CopyMemory (pVertex, &listVertex [0], m_nNumVertices * sizeof (CAVertex) );
	m_pVB->Unlock ();

	m_listBone.resize (MAX_BONE);
}

void CAMeshSHSkin::CreateIB (const CAModelVIB& modelVIB)
{
	vector<DWORD> listIndex;
	for (int i = 0; i != m_nNumTriangles; ++i)
	{
		listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [0]);
		listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [1]);
		listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [2]);
	}

	DWORD dwUsage = APROJECT_WINDOW->GetD3DDevice ()->GetSoftwareVertexProcessing () ? D3DUSAGE_SOFTWAREPROCESSING : 0;

	APROJECT_WINDOW->GetD3DDevice ()->CreateIndexBuffer (m_nNumTriangles * sizeof (DWORD) * 3, dwUsage, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIB, NULL);
	
	void* pIndex = NULL;
	m_pIB->Lock (0, 0, (void**)&pIndex, 0);
	CopyMemory (pIndex, &listIndex [0], m_nNumTriangles * sizeof (DWORD) * 3);
	m_pIB->Unlock ();
}

void CAMeshSHSkin::SetEffectData ()
{
	D3DXMATRIX TM;
	for (set<int>::iterator itor = m_listBoneID.begin (); itor != m_listBoneID.end (); ++itor)
	{
		if (*itor)
			TM = m_matTM * *(m_pModel->GetBoneTM (*itor) );
		else 
			TM = m_matTM;

		TM = TM * *(m_pModel->GetTM () );

		m_listBone [*itor] = TM;
	}

	m_pEffect->SetMatrixArray ("matPal", &m_listBone [0], MAX_BONE);

	static D3DXMATRIX matViewProj;
	matViewProj = *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix ();
	matViewProj *= *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix ();
	m_pEffect->SetMatrix ("matViewProj", &matViewProj);

	static LPDIRECT3DBASETEXTURE9 pTexture = NULL;
	APROJECT_WINDOW->GetD3DDevice ()->GetTexture (0, &pTexture);
	m_pEffect->SetTexture ("ColorMap", pTexture);
	
	m_pEffect->SetVector ("vecLightDir", &D3DXVECTOR4(0.0f, 0.0f, 1.0f, 0.0f) );
}

CAMeshSHSkin::CAMeshSHSkin (CAModel* pModel)
	: CAModelMesh (pModel), m_pEffect (NULL)
{
	CreateEffect ();
}

CAMeshSHSkin::~CAMeshSHSkin (void)
{
	SAFE_RELEASE (m_pEffect);
}

void CAMeshSHSkin::MakeVIB (const CAModelVIB& modelVIB)
{
	m_dwFVF = CAVertex::FVF;

	m_nNumVertices	= (int)modelVIB.m_listVertex.size ();
	m_nNumTriangles = (int)modelVIB.m_listIndex.size ();

	CreateVB (modelVIB);
	CreateIB (modelVIB);
}

bool CAMeshSHSkin::Render_Front ()
{
	APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pVB, 0, sizeof (CAVertex) );
	APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIB);
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (CAVertex::FVF);

	return true;
}

void CAMeshSHSkin::Render ()
{
	SetEffectData ();

	// only one pass
	m_pEffect->Begin (NULL,0);
	m_pEffect->BeginPass (0);

	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, m_nNumVertices, 0, m_nNumTriangles);

	m_pEffect->EndPass ();
	m_pEffect->End ();
}
