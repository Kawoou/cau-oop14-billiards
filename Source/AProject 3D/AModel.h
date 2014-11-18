#ifndef __APROJECT_OBJECT_MODEL__
#define __APROJECT_OBJECT_MODEL__

#pragma once

// Model Class (Object = Mesh + Bone + Dummy<root>)
class CAModelMaterial;
class CAModelObject;
class CAModelMesh;
class CAModelBone;
class CAModel : public CAObject
{
public:
	CAModel (void);
	virtual ~CAModel (void);

public:
	bool Create (const char* strFileName, int nSkinningMode);

private:
	bool Load (const char* strFileName, int nSkinningMode);
	void Unload (void);

public:
	void SetSkinngMode (int nSkinningMode);

	void AddMaterial (CAModelMaterial* pMaterial);
	void AddObject (CAModelObject* pObject, char* strClass);

public:
	D3DXMATRIX* GetObjectTM (int nObjectID);
	D3DXMATRIX* GetBoneTM (int nBoneID);

public:
	virtual void Reset (void);
	void Update(float fFrame);
	virtual void Update (void);
	virtual void Render (void);

public:
	void SetTM (D3DXMATRIX* pTM) { m_matTM = *pTM; }
	D3DXMATRIX* GetTM () { return &m_matTM; }

	void SetObjCount (int nCount) { m_nObjCount = nCount; }
	void SetMeshCount (int nCount) { m_nMeshCount = nCount; }
	void SetAniStart (float fStart) { m_fAniStart = fStart; }
	void SetAniEnd (float fEnd) { m_fAniEnd = fEnd; }

	float GetStartFrame () { return m_fAniStart; }
	float GetEndFrame () { return m_fAniEnd; }

private:
	D3DXMATRIX m_matTM;

	int							m_nObjCount;
	int							m_nMeshCount;
	float						m_fAniStart;
	float						m_fAniEnd;
	int							m_nKeyType;
	int							m_nSkinType;
	D3DXMATRIXA16				m_matWorld;								//!< 월드 변환 행렬

	vector<CAModelMaterial*>	m_listMaterial;
	vector<CAModelObject*>		m_listObject;
	vector<CAModelMesh*>		m_listMesh;
	vector<CAModelBone*>		m_listBone;

	std::string m_strFile;

	int m_nSkinningMode;
};

#endif