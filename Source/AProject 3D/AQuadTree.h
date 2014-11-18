#ifndef __APROJECT_HEIGHTMAP_QUADTREE__
#define __APROJECT_HEIGHTMAP_QUADTREE__

#pragma once

struct TERRAIN_VERTEX;
class CAQuadTree
{
public:
	enum
	{
		LEFT_TOP,			//!< 사각형 왼쪽 상단 모서리
		RIGHT_TOP,			//!< 사각형 오른쪽 상단 모서리
		LEFT_BOTTOM,		//!< 사각형 왼쪽 하단 모서리
		RIGHT_BOTTOM,		//!< 사각형 오른쪽 하단 모서리
		
		MAX_CORNER,			//!< 최대 모서리 수
	};

	enum
	{
		MAX_CHILD	= 4,	//!< 자식 노드 수
	};

protected:
	AUInt32		m_uCorner [MAX_CORNER];		//!< 모서리 인덱스
	D3DXVECTOR3	m_vCenter;					//!< 중심 좌표
	AReal32		m_fRadius;					//!< 경계구 반지름
	CAQuadTree*	m_pChild [MAX_CHILD];		//!< 자식 노드
	AUInt32		m_uCellsPerTile;			//!< 모서리 기준 타일당 셀 갯수

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