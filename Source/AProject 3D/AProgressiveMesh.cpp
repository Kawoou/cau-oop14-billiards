#include "AModelInclude.h"

Triangle::Triangle(CAProgressiveMesh* parent, Vertex *v0, Vertex *v1, Vertex *v2)
{
	mParent = parent;

	vertex[0] = v0;
	vertex[1] = v1;
	vertex[2] = v2;

	ComputeNormal ();
	mParent->m_vTriangles.Add (this);
	for(int i = 0; i < 3; i ++)
	{
		vertex[i]->face.Add(this);
		for(int j = 0; j < 3; j ++)
			if(i != j)
				vertex[i]->neighbor.AddUnique(vertex[j]);
	}
}

Triangle::~Triangle()
{
	int i;
	mParent->m_vTriangles.Remove(this);
	for(i = 0; i < 3; i ++)
	{
		if(vertex[i]) vertex[i]->face.Remove(this);
	}
	for(i = 0; i < 3; i ++)
	{
		int i2 = (i + 1) % 3;
		if(!vertex[i] || !vertex[i2]) continue;
		vertex[i ]->RemoveIfNonNeighbor(vertex[i2]);
		vertex[i2]->RemoveIfNonNeighbor(vertex[i ]);
	}
}

int Triangle::HasVertex (Vertex *v)
{
	return (v == vertex [0] || v == vertex [1] || v == vertex [2]);
}

void Triangle::ComputeNormal ()
{
	AVector3 v0 = vertex [0]->position;
	AVector3 v1 = vertex [1]->position;
	AVector3 v2 = vertex [2]->position;

	normal = (v1 - v0) * (v2 - v1);
	if (normal.Length () == 0)
		return;
	normal = normal.Normalize ();
}

void Triangle::ReplaceVertex (Vertex *vold, Vertex *vnew)
{
	if (vold == vertex [0])
		vertex [0] = vnew;
	else if (vold == vertex[1])
		vertex [1] = vnew;
	else
		vertex [2] = vnew;

	int i;
	vold->face.Remove (this);
	vnew->face.Add(this);
	for(i = 0; i < 3; i ++)
	{
		vold->RemoveIfNonNeighbor (vertex [i]);
		vertex [i]->RemoveIfNonNeighbor (vold);
	}
	for (i = 0; i < 3; i ++)
	{
		for (int j = 0; j < 3; j ++)
			if (i != j)
				vertex [i]->neighbor.AddUnique (vertex [j]);
	}
	ComputeNormal ();
}

Vertex::Vertex (CAProgressiveMesh* parent, AVector3 v, int _id)
{
	position = v;
	id = _id;

	mParent = parent;

	mParent->m_vVertices.Add (this);
}

Vertex::~Vertex ()
{
	while(neighbor.num)
	{
		neighbor [0]->neighbor.Remove (this);
		neighbor.Remove (neighbor [0]);
	}
	mParent->m_vVertices.Remove (this);
}

void Vertex::RemoveIfNonNeighbor (Vertex *n)
{
	if (!neighbor.Contains (n) ) return;

	for (int i = 0; i < face.num; i ++)
	{
		if(face [i]->HasVertex (n) )
			return;
	}
	neighbor.Remove (n);
}

float CAProgressiveMesh::ComputeEdgeCollapseCost(Vertex *u, Vertex *v)
{
	int i;
	float edgelength = (v->position - u->position).Length ();
	float curvature=0;

	AList<Triangle*> sides;
	for (i = 0; i < u->face.num; i ++)
	{
		if (u->face [i]->HasVertex (v) )
			sides.Add (u->face [i]);
	}

	for (i = 0; i < u->face.num; i ++)
	{
		float mincurv = 1;
		for (int j = 0; j < sides.num; j ++)
		{
			float dotprod = (AReal32)AVector3::Dot (u->face [i]->normal, sides [j]->normal);
			mincurv = min (mincurv, (1 - dotprod) / 2.0f);
		}
		curvature = max (curvature, mincurv);
	}
	return edgelength * curvature;
}

void CAProgressiveMesh::ComputeEdgeCostAtVertex (Vertex *v)
{
	if (v->neighbor.num == 0)
	{
		v->collapse = NULL;
		v->objdist = -0.01f;
		return;
	}
	v->objdist = 1000000;
	v->collapse = NULL;

	for (int i = 0; i < v->neighbor.num; i ++)
	{
		float dist = ComputeEdgeCollapseCost (v, v->neighbor [i]);
		if (dist < v->objdist)
		{
			v->collapse = v->neighbor [i];
			v->objdist = dist;
		}
	}
}

void CAProgressiveMesh::ComputeAllEdgeCollapseCosts ()
{
	for (int i = 0; i < m_vVertices.num; i ++)
		ComputeEdgeCostAtVertex (m_vVertices [i]);
}

void CAProgressiveMesh::Collapse (Vertex *u, Vertex *v)
{
	if (!v)
	{
		delete u;
		return;
	}

	int i;
	AList<Vertex*> tmp;

	for (i = 0; i < u->neighbor.num; i ++)
		tmp.Add (u->neighbor [i]);

	for (i = u->face.num - 1; i >= 0; i --)
	{
		if (u->face [i]->HasVertex (v) )
			delete (u->face [i]);
	}

	for (i = u->face.num - 1; i >= 0; i --)
		u->face [i]->ReplaceVertex (u, v);
	delete u;

	for (i = 0; i < tmp.num; i ++)
		ComputeEdgeCostAtVertex (tmp [i]);
}

void CAProgressiveMesh::AddVertex (vector<CAVertex>& vert)
{
	for (unsigned int i = 0; i < vert.size (); i ++)
		Vertex *v = new Vertex(this, vert [i].pos, i);
}

void CAProgressiveMesh::AddFaces (vector<DWORD>& tri)
{
	for (unsigned int i = 0; i < tri.size () / 3; i ++)
	{
		Triangle *t = new Triangle(
			this,
			m_vVertices [tri [i * 3 + 0] ],
			m_vVertices [tri [i * 3 + 1] ],
			m_vVertices [tri [i * 3 + 2] ]
		);
	}
}

Vertex* CAProgressiveMesh::MinimumCostEdge ()
{
	Vertex *mn = m_vVertices [0];
	for (int i = 0; i < m_vVertices.num; i ++)
	{
		if(m_vVertices [i]->objdist < mn->objdist)
			mn = m_vVertices [i];
	}
	return mn;
}

CAProgressiveMesh::CAProgressiveMesh ()
{

}

CAProgressiveMesh::~CAProgressiveMesh()
{

}

void CAProgressiveMesh::ProgressiveMesh (vector<CAVertex>& vert, vector<DWORD>& tri)
{
	unsigned int i;

	AddVertex (vert);
	AddFaces (tri);
	ComputeAllEdgeCollapseCosts ();
	m_lPermutation.SetSize (m_vVertices.num);
	m_lCollapseMap.SetSize (m_vVertices.num);

	while (m_vVertices.num > 0)
	{
		Vertex* mn = MinimumCostEdge ();
		m_lPermutation [mn->id] = m_vVertices.num - 1;
		m_lCollapseMap [m_vVertices.num - 1] = (mn->collapse) ? mn->collapse->id : -1;
		Collapse (mn, mn->collapse);
	}
	for (i = 0; i < (unsigned int)m_lCollapseMap.num; i ++)
		m_lCollapseMap [i] = (m_lCollapseMap [i] == -1) ? 0 : m_lPermutation [m_lCollapseMap [i] ];

	// Re-ordering
	AList<CAVertex> temp_list;
	for (i = 0; i < vert.size (); i ++)
		temp_list.Add (vert [i]);

	for (i = 0; i < vert.size (); i ++)
		vert [m_lPermutation [i] ] = temp_list [i];

	for (i = 0; i < tri.size () / 3; i ++)
	{
		tri [i * 3 + 0] = m_lPermutation [tri [i * 3 + 0] ];
		tri [i * 3 + 1] = m_lPermutation [tri [i * 3 + 1] ];
		tri [i * 3 + 2] = m_lPermutation [tri [i * 3 + 2] ];
	}
}

int CAProgressiveMesh::Map(int a, int max)
{
	if (max <= 0) return 0;
	while (a >= max)
		a = m_lCollapseMap [a];
	return a;
}