#include "AModelInclude.h"

CAModelBone::CAModelBone (CAModel* pModel)
	: CAModelObject (pModel)
{
	D3DXMatrixIdentity (&m_matBone);
}

CAModelBone::~CAModelBone (void)
{
}

void CAModelBone::Update (float fFrame, D3DXMATRIX* pParentTM)
{
	CAModelObject::Update (fFrame, pParentTM);

	D3DXMATRIX	mWI; // world inverse	
	D3DXMatrixInverse (&mWI, NULL, &m_matWorld);
	m_matBone = mWI * m_matTM;
}
