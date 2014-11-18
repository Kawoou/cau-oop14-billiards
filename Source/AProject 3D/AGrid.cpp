#include "AWindow.h"

CAGrid::CAGrid(AReal32 nSize, AReal32 nAreaSize)
	:m_nSize (nSize), m_nAreaSize (nAreaSize)
{
	m_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	m_pVertexBuffer = NULL;

	Reset ();

	m_bUpdateMatrix = true;
}

CAGrid::~CAGrid (void)
{
	DebugConsoleLog (DebugClear, "CAGrid: Vertex buffer Destroy");
	SAFE_RELEASE (m_pVertexBuffer);
}

void CAGrid::Reset (void)
{
	SAFE_RELEASE (m_pVertexBuffer);

	float lineSize = (m_nSize == 0) ? m_nAreaSize : m_nSize;
	GridVertex vertices [] = 
	{
		{-lineSize,     0.0f,      0.0f, 0xff00ff00},		// x축 라인을 위한 버텍스
		{ lineSize,     0.0f,      0.0f, 0xff00ff00},

		{    0.0f,      0.0f, -lineSize, 0xffffff00},		// z축 라인을 위한 버텍스
		{    0.0f,      0.0f,  lineSize, 0xffffff00},

		{    0.0f, -lineSize,      0.0f, 0xffff0000},		// y축 라인을 위한 버텍스
		{    0.0f,  lineSize,      0.0f, 0xffff0000},
	};
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (sizeof (vertices), 0,
							m_FVF, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL) ) )
	{
		DebugLog ("CAGrid(Create vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CAGrid: CAGrid(Create vertex buffer Error) 실패!");

		return;
	}
	VOID *pVertices;
	if (FAILED (m_pVertexBuffer->Lock (0, sizeof (vertices), (void**)&pVertices, 0) ) )
	{
		DebugLog ("Reset(Lock vertex buffer Error) 실패!");
		DebugConsoleLog (DebugError, "CAGrid: Reset(Lock vertex buffer Error) 실패!");

		return;
	}
	memcpy (pVertices, vertices, sizeof (vertices) );
	m_pVertexBuffer->Unlock ();

	DebugConsoleLog (DebugClear, "CAGrid: Initialize Grid, the success");
}

void CAGrid::MoveX(AReal32 fX)										{ CAObject::MoveX(fX); m_bUpdateMatrix = true; }
void CAGrid::MoveY(AReal32 fY)										{ CAObject::MoveY(fY); m_bUpdateMatrix = true; }
void CAGrid::MoveZ(AReal32 fZ)										{ CAObject::MoveZ(fZ); m_bUpdateMatrix = true; }
void CAGrid::MoveRotX(AReal32 fRotate)								{ CAObject::MoveRotX(fRotate); m_bUpdateMatrix = true; }
void CAGrid::MoveRotY(AReal32 fRotate)								{ CAObject::MoveRotY(fRotate); m_bUpdateMatrix = true; }
void CAGrid::MoveRotZ(AReal32 fRotate)								{ CAObject::MoveRotZ(fRotate); m_bUpdateMatrix = true; }
void CAGrid::SetPos(AReal32 fX, AReal32 fY)							{ CAObject::SetPos(fX, fY); m_bUpdateMatrix = true; }
void CAGrid::SetPos(AReal32 fX, AReal32 fY, AReal32 fZ)				{ CAObject::SetPos(fX, fY, fZ); m_bUpdateMatrix = true; }
void CAGrid::SetPos(AVector3 vPos)									{ CAObject::SetPos(vPos); m_bUpdateMatrix = true; }
void CAGrid::SetSize(AReal32 fSizeX, AReal32 fSizeY)					{ CAObject::SetSize(fSizeX, fSizeY); m_bUpdateMatrix = true; }
void CAGrid::SetSize(AReal32 fSizeX, AReal32 fSizeY, AReal32 fSizeZ)	{ CAObject::SetSize(fSizeX, fSizeY, fSizeZ); m_bUpdateMatrix = true; }
void CAGrid::SetSize(AVector3 vSize)									{ CAObject::SetSize(vSize); m_bUpdateMatrix = true; }
void CAGrid::SetOffset(AReal32 fX, AReal32 fY)						{ CAObject::SetOffset(fX, fY);  m_bUpdateMatrix = true; }
void CAGrid::SetOffset(AReal32 fX, AReal32 fY, AReal32 fZ)			{ CAObject::SetOffset(fX, fY, fZ); m_bUpdateMatrix = true; }
void CAGrid::SetOffset(AVector3 vPos)									{ CAObject::SetOffset(vPos); m_bUpdateMatrix = true; }
void CAGrid::SetRotX(AReal32 fRotate)									{ CAObject::SetRotX(fRotate); m_bUpdateMatrix = true; }
void CAGrid::SetRotY(AReal32 fRotate)									{ CAObject::SetRotY(fRotate); m_bUpdateMatrix = true; }
void CAGrid::SetRotZ(AReal32 fRotate)									{ CAObject::SetRotZ(fRotate); m_bUpdateMatrix = true; }

void CAGrid::SetMatrix(D3DXMATRIX& mat)
{
	m_matComplate = mat;

	m_bUpdateMatrix = false;
}

void CAGrid::UpdateMatrix(void)
{
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;
	D3DXMATRIX matComplate;

	D3DXMATRIX mT;
	{
		AVector3 vUp(0.0f, 1.0f, 0.0f);
		AVector3 vLeft(1.0f, 0.0f, 0.0f);
		AVector3 vLook(0.0f, 0.0f, 1.0f);
		AVector3 vUpNew, vLeftNew, vLookNew;
		D3DXMATRIX mX, mY, mZ;

		D3DXMatrixRotationX(&mX, m_fRotX);
		D3DXMatrixRotationY(&mY, m_fRotY);
		D3DXMatrixRotationZ(&mZ, -m_fRotZ);

		D3DXMatrixTranslation(&mT, vUp.x, vUp.y, vUp.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vUpNew = AVector3(mT._41, mT._42, mT._43);

		D3DXMatrixTranslation(&mT, vLeft.x, vLeft.y, vLeft.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vLeftNew = AVector3(mT._41, mT._42, mT._43);

		D3DXMatrixTranslation(&mT, vLook.x, vLook.y, vLook.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vLookNew = AVector3(mT._41, mT._42, mT._43);

		mT._11 = vLeftNew.x;   mT._12 = vLeftNew.y;   mT._13 = vLeftNew.z;   mT._14 = 0.0f;		// Left Vector
		mT._21 = vUpNew.x;     mT._22 = vUpNew.y;     mT._23 = vUpNew.z;     mT._24 = 0.0f;		// Up Vector
		mT._31 = vLookNew.x;   mT._32 = vLookNew.y;   mT._33 = vLookNew.z;   mT._34 = 0.0f;		// Front Vector
		mT._41 = 0.0f;         mT._42 = 0.0f;         mT._43 = 0.0f;         mT._44 = 1.0f;		// Translation
	}

	D3DXMatrixScaling(&matSize, m_vSize.x, m_vSize.y, m_vSize.z);
	D3DXMatrixTranslation(&matPosition, m_vPosition.x, m_vPosition.y, m_vPosition.z);

	m_matComplate = matSize * mT * matPosition;

	m_bUpdateMatrix = false;
}

void CAGrid::Update (void)
{
	if (m_bUpdateMatrix)
		UpdateMatrix();
}

void CAGrid::Render (void)
{
	APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, m_pVertexBuffer, 0, sizeof (GridVertex) );
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (m_FVF);

	D3DXMATRIXA16 matWorld;
	for (float x = -m_nSize; x <= m_nSize; x += m_nAreaSize)
	{
		D3DXMatrixTranslation (&matWorld, x, 0.0, 0.0);
		APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &(m_matComplate * matWorld));
		APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_LINELIST, 2, 1);	
	}

	for (float z = -m_nSize; z <= m_nSize; z += m_nAreaSize)
	{
		D3DXMatrixTranslation (&matWorld, 0.0, 0.0, z);
		APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &(m_matComplate * matWorld));	// 변환 매트릭스 적용
		APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_LINELIST, 0, 1);  	// x축 라인 그리기
	}
	APROJECT_WINDOW->GetD3DDevice ()->SetTransform (D3DTS_WORLD, &m_matComplate);
	APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_LINELIST, 4, 1);
}