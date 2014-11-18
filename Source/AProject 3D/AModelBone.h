#ifndef __APROJECT_OBJECT_MODEL_BONE__
#define __APROJECT_OBJECT_MODEL_BONE__

#pragma once

class CAModelBone : public CAModelObject
{
private:
	D3DXMATRIX m_matBone;

public:
	CAModelBone (CAModel* pModel);
	virtual ~CAModelBone (void);

public:
	virtual void Update (float fFrame, D3DXMATRIX* pParentTM);

	D3DXMATRIX* GetBoneTM () { return &m_matBone; }
};

#endif