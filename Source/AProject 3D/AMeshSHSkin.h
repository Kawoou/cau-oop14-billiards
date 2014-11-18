#ifndef __APROJECT_OBJECT_MODEL_MESH_SHSKIN__
#define __APROJECT_OBJECT_MODEL_MESH_SHSKIN__

#pragma once

class CAModelVIB;
class CAMeshSHSkin : public CAModelMesh
{
private:
	struct CAVertex
	{
		enum { FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3 | 
			          D3DFVF_TEXCOORDSIZE2 (0) | D3DFVF_TEXCOORDSIZE4 (1) | D3DFVF_TEXCOORDSIZE4 (2) ) };	

		D3DXVECTOR3	pos;
		D3DXVECTOR3	normal;
		D3DXVECTOR2	uv;
		D3DXVECTOR4	bone_index;
		D3DXVECTOR4	weight;
	};

private:
	set<int> m_listBoneID;
	vector<D3DXMATRIX> m_listBone; 

	LPD3DXEFFECT m_pEffect;

private:
	void CreateEffect ();

	void CreateVB (const CAModelVIB& modelVIB);
	void CreateIB (const CAModelVIB& modelVIB);

	void SetEffectData ();

public:
	CAMeshSHSkin (CAModel* pModel);
	virtual ~CAMeshSHSkin (void);

public:
	virtual void MakeVIB (const CAModelVIB& modelVIB);
	virtual bool Render_Front ();
	virtual void Render ();
};

#endif