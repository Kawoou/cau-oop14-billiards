#include "AModelInclude.h"

CAModelTrack::CAModelTrack()
{
}

CAModelTrack::~CAModelTrack ()
{
}

void CAModelTrack::Get2KeysPos (float fFrame, D3DXVECTOR3& vecKey1, D3DXVECTOR3& vecKey2, float& fAlpha)
{
	map<float, D3DXVECTOR3>::const_iterator itor = m_listPos.lower_bound (fFrame);

	if (itor == m_listPos.end () )
	{
		--itor;
		vecKey1 = itor->second;
		vecKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	if (itor == m_listPos.begin () || itor->first == fFrame)
	{
		vecKey1 = itor->second;
		vecKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	vecKey2 = itor->second; float f2 = itor->first; --itor;
	vecKey1 = itor->second; float f1 = itor->first;
	fAlpha = GetAlpha (f1, f2, fFrame);
}

void CAModelTrack::Get2KeysRot (float fFrame, D3DXQUATERNION& qKey1, D3DXQUATERNION& qKey2, float& fAlpha)
{
	map<float, D3DXQUATERNION>::const_iterator itor = m_listRot.lower_bound (fFrame);

	if (itor == m_listRot.end () )
	{
		--itor;
		qKey1 = itor->second;
		qKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	if (itor == m_listRot.begin () || itor->first == fFrame)
	{
		qKey1 = itor->second;
		qKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	qKey2 = itor->second; float f2 = itor->first; --itor;
	qKey1 = itor->second; float f1 = itor->first;
	fAlpha = GetAlpha (f1, f2, fFrame);
}

void CAModelTrack::Get2KeysScl (float fFrame, D3DXVECTOR3& vecKey1, D3DXVECTOR3& vecKey2, float& fAlpha)
{
	map<float, D3DXVECTOR3>::const_iterator itor = m_listScl.lower_bound (fFrame);

	if (itor == m_listScl.end () )
	{
		--itor;
		vecKey1 = itor->second;
		vecKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	if (itor == m_listScl.begin () || itor->first == fFrame)
	{
		vecKey1 = itor->second;
		vecKey2 = itor->second;
		fAlpha = 0;
		return;
	}

	vecKey2 = itor->second; float f2 = itor->first; --itor;
	vecKey1 = itor->second; float f1 = itor->first;
	fAlpha = GetAlpha (f1, f2, fFrame);
}

void CAModelTrack::GetPosKey (float fFrame, D3DXVECTOR3* pPos)
{
	float fAlpha;
	D3DXVECTOR3 vecKey1, vecKey2;

	Get2KeysPos (fFrame, vecKey1, vecKey2, fAlpha);

	if (fAlpha)
		D3DXVec3Lerp (pPos, &vecKey1, &vecKey2, fAlpha);
	else
		*pPos = vecKey1;
}

void CAModelTrack::GetRotKey (float fFrame, D3DXQUATERNION* pRot)
{
	float fAlpha;
	D3DXQUATERNION qKey1, qKey2;

	Get2KeysRot (fFrame, qKey1, qKey2, fAlpha);

	if (fAlpha)
		D3DXQuaternionSlerp (pRot, &qKey1, &qKey2, fAlpha);
	else
		*pRot = qKey1;
}

void CAModelTrack::GetSclKey (float fFrame, D3DXVECTOR3* pScl)
{
	float fAlpha;
	D3DXVECTOR3 vecKey1, vecKey2;

	Get2KeysScl (fFrame, vecKey1, vecKey2, fAlpha);

	if (fAlpha)
		D3DXVec3Lerp (pScl, &vecKey1, &vecKey2, fAlpha);
	else
		*pScl = vecKey1;
}



void CAModelTrack::AddPosKeyFrame (float fFrame, const D3DXVECTOR3& pos)
{
	m_listPos.insert (make_pair (fFrame, pos) );
}

void CAModelTrack::AddRotKeyFrame (float fFrame, const D3DXVECTOR4& rot, float fStartFrame)
{
	D3DXQUATERNION	q;
	
	if (fFrame <= fStartFrame)
		q = D3DXQUATERNION (0, 0, 0, 1);
	else
		q = D3DXQUATERNION (rot.x, rot.y, rot.z, rot.w);

	if (m_listRot.empty () )
		m_listRot.insert (make_pair (fFrame, q) );
	else
	{
		D3DXQUATERNION acc;
		map<float, D3DXQUATERNION>::reverse_iterator ritor = m_listRot.rbegin ();
		D3DXQuaternionMultiply (&acc, &(ritor->second), &q);
		m_listRot.insert (make_pair (fFrame, acc) );
	}
}

void CAModelTrack::AddSclKeyFrame (float fFrame, const D3DXVECTOR3& scl)
{
	m_listScl.insert (make_pair (fFrame, scl) );
}

D3DXMATRIX*	CAModelTrack::Update (float fFrame)
{
	D3DXVECTOR3		v;
	D3DXQUATERNION	q;

	D3DXMatrixIdentity (&m_matAni);

	if (!m_listRot.empty () )
	{
		GetRotKey (fFrame, &q);
		D3DXMatrixRotationQuaternion (&m_matAni, &q);
	}

	if (!m_listPos.empty () )
	{
		GetPosKey (fFrame, &v);

		m_matAni._41 = v.x;
		m_matAni._42 = v.y;
		m_matAni._43 = v.z;
	}

	if (!m_listScl.empty () )
	{
		GetSclKey (fFrame, &v);

		m_matAni._11 *= v.x;
		m_matAni._22 *= v.y;
		m_matAni._33 *= v.z;
	}

	return &m_matAni;
}