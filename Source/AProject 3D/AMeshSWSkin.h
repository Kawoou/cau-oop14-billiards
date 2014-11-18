#ifndef __APROJECT_OBJECT_MODEL_MESH_SWSKIN__
#define __APROJECT_OBJECT_MODEL_MESH_SWSKIN__

#pragma once

class CAModelVIB;
class CAMeshSWSkin : public CAModelMesh
{
private:
	vector<CAVertex>	m_listVertex;
	vector<DWORD>		m_listIndex;
	vector<CASkin>		m_listSkin;

private:
	void CreateVB (const CAModelVIB& modelVIB);
	void CreateIB (const CAModelVIB& modelVIB);
	void CreateSkin (const CAModelVIB& modelVIB);

	void CalculateSkinng (int verticesCount);

public:
	CAMeshSWSkin (CAModel* pModel);
	virtual ~CAMeshSWSkin (void);

public:
	virtual void MakeVIB (const CAModelVIB& modelVIB);
	virtual bool Render_Front ();
	virtual void Render ();
};

#endif