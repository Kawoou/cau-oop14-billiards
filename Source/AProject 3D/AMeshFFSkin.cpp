#include "AModelInclude.h"

void CAMeshFFSkin::CreateVB (const CAModelVIB& modelVIB)
{
	vector<CAVertex> listVertex;
	listVertex.resize (m_nNumVertices);

	for (int i = 0; i != (int)listVertex.size (); ++i)
	{
		// Pos
		listVertex [i].pos = modelVIB.m_listVertex [i];

		// Weight
		for (int j = 0; j != 3; ++j)
			listVertex [i].weight [j] = modelVIB.m_listWeight [i].m_listWeight [j];

		// Bone Index (ID)
		listVertex [i].index = modelVIB.m_listWeight [i].m_dwWeightIndex;
		for (int j = 0; j != (int)modelVIB.m_listWeight [i].m_listBoneID.size (); ++j)
			m_listBoneID.insert (modelVIB.m_listWeight [i].m_listBoneID [j]);

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
	}

	DWORD dwUsage = APROJECT_WINDOW->GetD3DDevice ()->GetSoftwareVertexProcessing () ? D3DUSAGE_SOFTWAREPROCESSING : 0;

	APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (m_nNumVertices * sizeof(CAVertex), dwUsage, m_dwFVF, D3DPOOL_DEFAULT, &m_pVB, NULL);

	void* pVertex = NULL;
	m_pVB->Lock (0, 0, (void**)&pVertex, 0);
	CopyMemory (pVertex, &listVertex [0], m_nNumVertices * sizeof(CAVertex) );
	m_pVB->Unlock ();
}

void CAMeshFFSkin::CreateIB (const CAModelVIB& modelVIB)
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

void CAMeshFFSkin::SetSkinngMatrix ()
{
	D3DXMATRIX TM;
	for (set<int>::iterator itor = m_listBoneID.begin (); itor != m_listBoneID.end (); ++itor)
	{
		if (*itor)
			TM = m_matTM * *(m_pModel->GetBoneTM (*itor) );
		else 
			TM = m_matTM;

		TM = TM * *(m_pModel->GetTM () );

		APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLDMATRIX (*itor), &TM);
	}
	//m_matWorld = TM;
}

CAMeshFFSkin::CAMeshFFSkin (CAModel* pModel)
	: CAModelMesh (pModel)
{
}

CAMeshFFSkin::~CAMeshFFSkin(void)
{
}

void CAMeshFFSkin::MakeVIB (const CAModelVIB& modelVIB)
{
	m_dwFVF = CAVertex::FVF;

	m_nNumVertices	= (int)modelVIB.m_listVertex.size ();
	m_nNumTriangles = (int)modelVIB.m_listIndex.size ();

	CreateVB (modelVIB);
	CreateIB (modelVIB);
}

bool CAMeshFFSkin::Render_Front ()
{
	SetSkinngMatrix ();

	APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pVB, 0, sizeof (CAVertex) );
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (m_dwFVF);
	APROJECT_WINDOW->GetD3DDevice ()->SetIndices (m_pIB);

	return true;
}

void CAMeshFFSkin::Render ()
{
	//DWORD dwIndexVertexBlend;
	//DWORD dwVertexBlend;

	// Get Original Data
	//APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_INDEXEDVERTEXBLENDENABLE, &dwIndexVertexBlend);
	//APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_VERTEXBLEND, &dwVertexBlend);

	// Set New Data
	//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
	//APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);

	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, m_nNumVertices, 0, m_nNumTriangles);

	// Set Original Data
    //APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_VERTEXBLEND, dwVertexBlend);
    //APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_INDEXEDVERTEXBLENDENABLE, dwIndexVertexBlend);
}