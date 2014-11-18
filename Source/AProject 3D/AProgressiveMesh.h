#ifndef __APROJECT_OBJECT_MODEL_PROGRESSIVE_MESH__
#define __APROJECT_OBJECT_MODEL_PROGRESSIVE_MESH__

#pragma once

/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 *  Permission to use any of this code wherever you want is granted..
 *  Although, please do acknowledge authorship if appropriate.
 *
 *  See the header file progmesh.h for a description of this module
 */

class Vertex;
class Triangle;
class CAProgressiveMesh;

struct CAVertex
{
	enum { FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) };
	D3DXVECTOR3	pos;
	D3DXVECTOR3	normal;
	D3DXVECTOR2	uv;
};
struct CASkin
{
	D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	vector<int>		listBoneID;
	vector<float>	listWeight;
};

struct triData
{
	int v1;
	int v2;
	int v3;		// indices to vertex list
};

class Vertex
{
public:
	CAProgressiveMesh*	mParent;
	AVector3			position;
	int					id;
	AList<Vertex*>		neighbor;
	AList<Triangle*>	face;
	float				objdist;
	Vertex *			collapse;

public:
	Vertex(CAProgressiveMesh* parent, AVector3 v, int _id);
	~Vertex();

public:
	void RemoveIfNonNeighbor(Vertex* n);
};

class Triangle
{
public:
	CAProgressiveMesh*	mParent;
	Vertex*				vertex[3];
	AVector3			normal;

public:
	Triangle(CAProgressiveMesh* parent, Vertex* v0, Vertex* v1, Vertex* v2);
	~Triangle();

public:
	void ComputeNormal();
	void ReplaceVertex(Vertex *vold, Vertex *vnew);
	int HasVertex(Vertex *v);
};

class CAProgressiveMesh
{
private:
	

public:
	CAProgressiveMesh();
	~CAProgressiveMesh();

public:
	void ProgressiveMesh(vector<CAVertex>& vert, vector<DWORD>& tri);

	float ComputeEdgeCollapseCost(Vertex *u, Vertex *v);
	void ComputeEdgeCostAtVertex (Vertex *v);
	void ComputeAllEdgeCollapseCosts ();
	void Collapse (Vertex *u, Vertex *v);
	void AddVertex (vector<CAVertex>& vert);
	void AddFaces (vector<DWORD>& tri);
	Vertex* MinimumCostEdge ();

	int Map(int a, int max);

public:
	AList<Vertex*>		m_vVertices;
	AList<Triangle*>	m_vTriangles;
	AList<int>			m_lCollapseMap;
	AList<int>			m_lPermutation;
};

#endif