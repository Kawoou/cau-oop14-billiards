#include "AModelInclude.h"

CAModelMesh::CAModelMesh (CAModel* pModel)
	: CAModelObject (pModel),
	m_pVB (NULL),
	m_pIB (NULL),
	m_dwFVF (0),
	m_nNumVertices (0),
	m_nNumTriangles (0),
	m_nMaterialID (0)
{
}

CAModelMesh::~CAModelMesh (void)
{
	SAFE_RELEASE (m_pVB);
	SAFE_RELEASE (m_pIB);
}