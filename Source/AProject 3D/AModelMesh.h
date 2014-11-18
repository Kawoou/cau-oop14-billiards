#ifndef __APROJECT_OBJECT_MODEL_MESH__
#define __APROJECT_OBJECT_MODEL_MESH__

#pragma once

class CAModelMesh : public CAModelObject
{
protected:
	LPDIRECT3DVERTEXBUFFER9	m_pVB;
	LPDIRECT3DINDEXBUFFER9	m_pIB;

	DWORD m_dwFVF;
	int m_nNumVertices;
	int m_nNumTriangles;

	int	m_nMaterialID;

public:
	CAModelMesh (CAModel* pModel);
	virtual ~CAModelMesh (void);
};

#endif