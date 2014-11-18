#include "AMapInclude.h"

CAQuadTree::CAQuadTree ()
	: m_vCenter (0.0f, 0.0f, 0.0f), m_fRadius (0.0f), m_uCellsPerTile (0)
{
	for (int i = 0; i < MAX_CORNER; ++i)
		m_uCorner [i] = 0;

	for (int i = 0; i < MAX_CHILD; ++i)
		m_pChild [i] = NULL;
}

CAQuadTree::~CAQuadTree ()
{
	Release ();
}

CAQuadTree::CAQuadTree (const CAQuadTree& quadTree)
{
	DeepCopy (quadTree);
}

CAQuadTree& CAQuadTree::operator=(const CAQuadTree&	quadTree)
{
	DeepCopy (quadTree);

	return (*this);
}

bool CAQuadTree::Create (AUInt32 uWidth, AUInt32 uHeight, AUInt32 uCellsPerTile)
{
	if ( (uWidth % 2) + (uHeight % 2) > 0)
		return false;

	SetChild (0, uWidth, (uWidth + 1) * uHeight, (uWidth + 1) * (uHeight + 1) - 1, uCellsPerTile);

	return true;
}

void CAQuadTree::SetBoundingSphere (TERRAIN_VERTEX*	pV)
{
	AUInt32 uCenter	= (m_uCorner [LEFT_TOP] + m_uCorner [RIGHT_TOP] + m_uCorner [LEFT_BOTTOM] + m_uCorner [RIGHT_BOTTOM]) / 4;

	m_vCenter = pV [uCenter].p;
	m_vCenter.y	= 0.0f;

	D3DXVECTOR3	vLeftTop = pV [m_uCorner [LEFT_TOP] ].p;
	vLeftTop.y = 0.0f;

	m_fRadius = D3DXVec3Length (&(m_vCenter - vLeftTop) );

	if (m_uCorner [RIGHT_TOP] - m_uCorner [LEFT_TOP] < m_uCellsPerTile + 1)
		return;

	for (int i = 0; i < MAX_CHILD; ++i)
		m_pChild [i]->SetBoundingSphere (pV);
}

void CAQuadTree::SetBoundingSphere (std::vector<TERRAIN_VERTEX>& vertices)
{
	AUInt32 uCenter	= (m_uCorner [LEFT_TOP] + m_uCorner [RIGHT_TOP] + m_uCorner [LEFT_BOTTOM] + m_uCorner [RIGHT_BOTTOM]) / 4;

	m_vCenter = vertices [uCenter].p;
	m_vCenter.y	= 0.0f;

	D3DXVECTOR3	vLeftTop = vertices [m_uCorner [LEFT_TOP] ].p;
	vLeftTop.y = 0.0f;

	m_fRadius = D3DXVec3Length (&(m_vCenter - vLeftTop) );

	if (m_uCorner [RIGHT_TOP] - m_uCorner [LEFT_TOP] < m_uCellsPerTile + 1)
		return;

	for (int i = 0; i < MAX_CHILD; ++i)
		m_pChild [i]->SetBoundingSphere (vertices);
}

void CAQuadTree::Culling (CAFrustum* pFrustum, vector<AUInt32>& vecVisibleIndex, AUInt32 uTiles, AVector3 vSize)
{
	D3DXVECTOR3 vCenter = D3DXVECTOR3(m_vCenter.x * vSize.x, m_vCenter.y * vSize.y, m_vCenter.z * vSize.z);
	if (pFrustum->IsInSphere (&vCenter, m_fRadius, false) )
	{
		if (m_uCorner [RIGHT_TOP] - m_uCorner [LEFT_TOP] < m_uCellsPerTile + 1)
		{
			AUInt32 uDist  = m_uCorner [RIGHT_TOP] - m_uCorner [LEFT_TOP];
			AUInt32 uSize  = uTiles * m_uCellsPerTile + 1;
			AUInt32 uRows  = (m_uCorner [LEFT_TOP] % uSize) / uDist;
			AUInt32 uCols  = (m_uCorner [RIGHT_TOP] / uSize) / uDist;
			AUInt32 uIndex = uCols * uTiles + uRows;

			vecVisibleIndex.push_back (uIndex);
		}
	}
	else
		return;

	if (m_uCorner [RIGHT_TOP] - m_uCorner [LEFT_TOP] < m_uCellsPerTile + 1)
		return;

	for (int i = 0; i < MAX_CHILD; ++i)
		m_pChild [i]->Culling (pFrustum, vecVisibleIndex, uTiles, vSize);
}

void CAQuadTree::DeepCopy (const CAQuadTree& quadTree)
{
	SetChild (quadTree.m_uCorner [LEFT_TOP], quadTree.m_uCorner [RIGHT_TOP], quadTree.m_uCorner [LEFT_BOTTOM], quadTree.m_uCorner [RIGHT_BOTTOM], quadTree.m_uCellsPerTile);
}

void CAQuadTree::Release ()
{
	for (int i = 0; i < MAX_CHILD; ++i)
		SAFE_DELETE (m_pChild [i]);
}

void CAQuadTree::SetChild (AUInt32 uLeftTop, AUInt32 uRightTop, AUInt32 uLeftBottom, AUInt32 uRightBottom, AUInt32 uCellsPerTile)
{
	m_uCorner [LEFT_TOP]		= uLeftTop;
	m_uCorner [RIGHT_TOP]		= uRightTop;
	m_uCorner [LEFT_BOTTOM]		= uLeftBottom;
	m_uCorner [RIGHT_BOTTOM]	= uRightBottom;
	m_uCellsPerTile				= uCellsPerTile;

	if (uRightTop - uLeftTop < m_uCellsPerTile + 1)
		return;

	for (int i = 0; i < MAX_CHILD; ++i)
		m_pChild [i] = new CAQuadTree ();

	AUInt32 uTopCenter		= (	uLeftTop		+ uRightTop		) / 2;
	AUInt32 uBottomCenter	= (	uLeftBottom		+ uRightBottom	) / 2;
	AUInt32 uLeftCenter		= (	uLeftTop		+ uLeftBottom	) / 2;
	AUInt32 uRightCenter	= (	uRightTop		+ uRightBottom	) / 2;
	AUInt32 uCenter			= (uLeftTop + uRightTop + uLeftBottom + uRightBottom) / 4;

	m_pChild[ 0 ]->SetChild (uLeftTop,		uTopCenter,		uLeftCenter,	uCenter,		m_uCellsPerTile);
	m_pChild[ 1 ]->SetChild (uTopCenter,		uRightTop,		uCenter,		uRightCenter,	m_uCellsPerTile);
	m_pChild[ 2 ]->SetChild (uLeftCenter,	uCenter,		uLeftBottom,	uBottomCenter,	m_uCellsPerTile);
	m_pChild[ 3 ]->SetChild (uCenter,		uRightCenter,	uBottomCenter,	uRightBottom,	m_uCellsPerTile);
}