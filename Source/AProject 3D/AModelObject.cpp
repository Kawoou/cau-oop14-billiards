#include "AModelInclude.h"

CAModelObject::CAModelObject (CAModel* pModel)
	: m_pModel (pModel),
		m_nObjectID (0),
		m_nParentID (0),
		m_nMaterialID (0),
		m_vBBoxMax (0.0f, 0.0f, 0.0f),
		m_vBBoxMin (0.0f, 0.0f, 0.0f)
{
	D3DXMatrixIdentity (&m_matLocal);
	D3DXMatrixIdentity (&m_matWorld);
	D3DXMatrixIdentity (&m_matTM);
}

CAModelObject::~CAModelObject (void)
{
}

void CAModelObject::Update (float fFrame, D3DXMATRIX* pParentTM)
{
	D3DXMATRIX* pmatAni = m_Track.Update (fFrame);

	m_matTM = m_matLocal * *pmatAni;

	// pos °ª Á¶Á¤
	if (pmatAni->_41 == 0.0f && pmatAni->_42 == 0.0f && pmatAni->_43 == 0.0f)
	{
		m_matTM._41 = m_matLocal._41;
		m_matTM._42 = m_matLocal._42;
		m_matTM._43 = m_matLocal._43;
	}
	else	
	{
		m_matTM._41 = pmatAni->_41;
		m_matTM._42 = pmatAni->_42;
		m_matTM._43 = pmatAni->_43;
	}

	m_matTM = m_matTM * *pParentTM;
}