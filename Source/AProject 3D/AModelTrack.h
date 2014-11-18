#ifndef __APROJECT_OBJECT_MODEL_TRACK__
#define __APROJECT_OBJECT_MODEL_TRACK__

#pragma once

#include "AWindow.h"

class CAModelTrack
{
private:
	map<float, D3DXVECTOR3>		m_listPos;
	map<float, D3DXQUATERNION>	m_listRot;
	map<float, D3DXVECTOR3>		m_listScl;

	D3DXMATRIX					m_matAni;

private:
	float GetAlpha (float f1, float f2, float frame)
	{
		return ( (frame - f1) / (f2 - f1) );
	}

	void Get2KeysPos (float fFrame, D3DXVECTOR3& vecKey1, D3DXVECTOR3& vecKey2, float& fAlpha);
	void Get2KeysRot (float fFrame, D3DXQUATERNION& vecKey1, D3DXQUATERNION& vecKey2, float& fAlpha);
	void Get2KeysScl (float fFrame, D3DXVECTOR3& vecKey1, D3DXVECTOR3& vecKey2, float& fAlpha);

	void GetPosKey (float fFrame, D3DXVECTOR3* pPos);
	void GetRotKey (float fFrame, D3DXQUATERNION* pRot);
	void GetSclKey (float fFrame, D3DXVECTOR3* pScl);

public:
	CAModelTrack ();
	~CAModelTrack ();

	void AddPosKeyFrame (float fFrame, const D3DXVECTOR3& pos);
	void AddRotKeyFrame (float fFrame, const D3DXVECTOR4& rot, float fStartFrame);
	void AddSclKeyFrame (float fFrame, const D3DXVECTOR3& scl);

	D3DXMATRIX*	Update (float fFrame);
};

#endif