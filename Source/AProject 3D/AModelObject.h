#ifndef __APROJECT_OBJECT_MODEL_OBJECT__
#define __APROJECT_OBJECT_MODEL_OBJECT__

#pragma once

class CAModelVIB;
class CAModelObject
{
protected:
	CAModel* m_pModel;

	int	m_nObjectID;
	int	m_nParentID;

	int m_nMaterialID;

	D3DXMATRIX m_matLocal;
	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matTM;

	D3DXVECTOR3	m_vBBoxMax;
	D3DXVECTOR3	m_vBBoxMin;

	CAModelTrack m_Track;

public:
	CAModelObject (CAModel* pModel);
	virtual ~CAModelObject (void);

public:
	void SetObjectID (int nID) { m_nObjectID = nID; }
	void SetParentID (int nID) { m_nParentID = nID; }
	void SetBBoxMax (const D3DXVECTOR3& v) { m_vBBoxMax = v; }
	void SetBBoxMin (const D3DXVECTOR3& v) { m_vBBoxMin = v; }
	void SetLocalTM (const D3DXMATRIX& mat) { m_matLocal = mat; }
	void SetWorldTM (const D3DXMATRIX& mat) { m_matWorld = mat; }
	void SetMaterialID (int nID) { m_nMaterialID = nID; }

	int GetObjectID () { return m_nObjectID; }
	int GetParentID () { return m_nParentID; }
	int GetMaterialID () { return m_nMaterialID; }

	D3DXMATRIX* GetTM () { return &m_matTM; }
	D3DXMATRIX* GetWorld () { return &m_matWorld; }
	D3DXVECTOR3* GetBBoxMax () { return &m_vBBoxMax; }
	D3DXVECTOR3* GetBBoxMin () { return &m_vBBoxMin; }
	CAModelTrack* GetTrack () { return &m_Track; }

public:
	virtual void MakeVIB (const CAModelVIB& modelVIB) 
	{
		//UNREFERENCED_PARAMETER (modelVIB);
	}

	virtual void Update (float fFrame, D3DXMATRIX* pParentTM);
	virtual bool Render_Front () { return false; }
	virtual void Render () { }
};

#endif