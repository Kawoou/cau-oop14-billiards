#ifndef __APROJECT_OBJECT_MODEL_MESH_FFSKIN__
#define __APROJECT_OBJECT_MODEL_MESH_FFSKIN__

#pragma once

class CAModelVIB;
class CAMeshFFSkin : public CAModelMesh
{
private:
	struct CAVertex
	{
		enum { FVF = (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1) };
		D3DXVECTOR3	pos;
		float		weight [3];
		DWORD		index;
		D3DXVECTOR3	normal;
		D3DXVECTOR2	uv;
	};

private:
	set<int> m_listBoneID;

private:
	void CreateVB (const CAModelVIB& modelVIB);
	void CreateIB (const CAModelVIB& modelVIB);

	void SetSkinngMatrix ();

public:
	CAMeshFFSkin (CAModel* pModel);
	virtual ~CAMeshFFSkin (void);

public:
	virtual void MakeVIB (const CAModelVIB& modelVIB);
	virtual bool Render_Front ();
	virtual void Render ();
};

#endif