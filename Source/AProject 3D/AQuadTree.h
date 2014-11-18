#ifndef __APROJECT_HEIGHTMAP_QUADTREE__
#define __APROJECT_HEIGHTMAP_QUADTREE__

#pragma once

struct TERRAIN_VERTEX;
class CAQuadTree
{
public:
	enum
	{
		LEFT_TOP,			//!< �簢�� ���� ��� �𼭸�
		RIGHT_TOP,			//!< �簢�� ������ ��� �𼭸�
		LEFT_BOTTOM,		//!< �簢�� ���� �ϴ� �𼭸�
		RIGHT_BOTTOM,		//!< �簢�� ������ �ϴ� �𼭸�
		
		MAX_CORNER,			//!< �ִ� �𼭸� ��
	};

	enum
	{
		MAX_CHILD	= 4,	//!< �ڽ� ��� ��
	};

protected:
	AUInt32		m_uCorner [MAX_CORNER];		//!< �𼭸� �ε���
	D3DXVECTOR3	m_vCenter;					//!< �߽� ��ǥ
	AReal32		m_fRadius;					//!< ��豸 ������
	CAQuadTree*	m_pChild [MAX_CHILD];		//!< �ڽ� ���
	AUInt32		m_uCellsPerTile;			//!< �𼭸� ���� Ÿ�ϴ� �� ����

public:
	CAQuadTree ();
	virtual ~CAQuadTree ();
	CAQuadTree (const CAQuadTree& quadTree);
	CAQuadTree& operator=(const CAQuadTree& quadTree);

public:
	virtual bool Create (AUInt32 uWidth, AUInt32 uHeight, AUInt32 uCellsPerTile);
	virtual void SetBoundingSphere (TERRAIN_VERTEX* pV);
	virtual void SetBoundingSphere (vector<TERRAIN_VERTEX>& vertices);
	virtual void Culling (CAFrustum* pFrustum, vector<AUInt32>& vecVisibleIndex, AUInt32 uTiles, AVector3 vSize);

protected:
	void DeepCopy (const CAQuadTree& quadTree);

	void Release ();
	void SetChild (AUInt32 uLeftTop, AUInt32 uRightTop, AUInt32 uLeftBottom, AUInt32 uRightBottom, AUInt32 uCellsPerTile);
};

#endif