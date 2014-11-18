#include "AModelInclude.h"

void CAMeshSWSkin::CreateVB (const CAModelVIB& modelVIB)
{
	m_listVertex.resize (m_nNumVertices);

	for (int i = 0; i != m_nNumVertices; ++i)
	{
		// Pos
		m_listVertex [i].pos = modelVIB.m_listVertex [i];

		// Normal
		if (!modelVIB.m_listNormal.empty () )
			m_listVertex [i].normal = modelVIB.m_listNormal [i];
		else
			m_listVertex [i].normal = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);

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

			m_listVertex [i].uv = modelVIB.m_listUV [nUVIndex];
		}
		else
			m_listVertex [i].uv = D3DXVECTOR2 (0.0f, 0.0f);
	}
}

void CAMeshSWSkin::CreateIB (const CAModelVIB& modelVIB)
{
	for (int i = 0; i != m_nNumTriangles; ++i)
	{
		m_listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [0]);
		m_listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [1]);
		m_listIndex.push_back ( (DWORD)modelVIB.m_listIndex [i].m_nIndex [2]);
	}
}

void CAMeshSWSkin::CreateSkin (const CAModelVIB& modelVIB)
{
	m_listSkin.resize (m_nNumVertices);

	for (int i = 0; i != m_nNumVertices; ++i)
	{
		// Pos
		m_listSkin [i].pos = modelVIB.m_listVertex [i];

		// Normal
		if (!modelVIB.m_listNormal.empty () )
			m_listSkin [i].normal = modelVIB.m_listNormal [i];
		else
			m_listSkin [i].normal = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);

		// BoneID
		m_listSkin [i].listBoneID = modelVIB.m_listWeight [i].m_listBoneID;

		// Weight
		m_listSkin [i].listWeight = modelVIB.m_listWeight [i].m_listWeight;
	}
}

void CAMeshSWSkin::CalculateSkinng (int verticesCount)
{
	D3DXMATRIX	TM;
	D3DXVECTOR3 pos, normal;
	for (int i = 0; i != m_nNumVertices; ++i)
	{
		m_listVertex [i].pos = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);
		m_listVertex [i].normal = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);

		for (int j = 0; j != (int)m_listSkin [i].listBoneID.size (); ++j)
		{
			if (m_listSkin [i].listBoneID [j])
				TM = m_matTM * *(m_pModel->GetBoneTM (m_listSkin [i].listBoneID [j]) );
			else
				TM = m_matTM;

			// calculate pos
			D3DXVec3TransformCoord (&pos, &m_listSkin [i].pos, &TM);
			m_listVertex [i].pos += pos * m_listSkin [i].listWeight [j];

			// calculate normal
			D3DXVec3TransformNormal (&normal, &m_listSkin [i].normal, &TM);
			m_listVertex [i].normal += normal * m_listSkin [i].listWeight [j];
		}
	}
}

CAMeshSWSkin::CAMeshSWSkin (CAModel* pModel)
	: CAModelMesh (pModel)
{
}

CAMeshSWSkin::~CAMeshSWSkin (void)
{
}

void CAMeshSWSkin::MakeVIB (const CAModelVIB& modelVIB)
{
	m_dwFVF = CAVertex::FVF;

	m_nNumVertices	= (int)modelVIB.m_listVertex.size ();
	m_nNumTriangles = (int)modelVIB.m_listIndex.size ();

	CreateVB (modelVIB);
	CreateIB (modelVIB);
	CreateSkin (modelVIB);
}

bool CAMeshSWSkin::Render_Front ()
{
	CalculateSkinng (0);

	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (CAVertex::FVF);

	return true;
}

void CAMeshSWSkin::Render ()
{
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, m_nNumVertices, m_nNumTriangles,
								      &m_listIndex [0], D3DFMT_INDEX32, &m_listVertex [0], sizeof (CAVertex) );
}