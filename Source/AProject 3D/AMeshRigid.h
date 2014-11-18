#ifndef __APROJECT_OBJECT_MODEL_MESH_RIGID__
#define __APROJECT_OBJECT_MODEL_MESH_RIGID__

#pragma once

class CAModelVIB;
class CAMeshRigid : CAModelMesh
{
private:
	struct CAVertex
	{
		enum { FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) };
		D3DXVECTOR3	pos;
		D3DXVECTOR3	normal;
		D3DXVECTOR2	uv;
	};

private:
	void CreateVB (const CAModelVIB& modelVIB);
	void CreateIB (const CAModelVIB& modelVIB);

public:
	CAMeshRigid (CAModel* pModel);
	virtual ~CAMeshRigid (void);

public:
	virtual void MakeVIB (const CAModelVIB& modelVIB);
	virtual bool Render_Front ();
	virtual void Render ();
};

#endif