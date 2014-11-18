#ifndef __APROJECT_GRID__
#define __APROJECT_GRID__

#pragma once

typedef struct
{
	AReal32 x, y, z;
	DWORD color;
}GridVertex;

class CAGrid : public CAObject
{
private:
	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;
	DWORD					m_FVF;

private:
	AReal32					m_nSize;
	AReal32					m_nAreaSize;
	D3DXMATRIX				m_matComplate;
	bool					m_bUpdateMatrix;

private:
	void UpdateMatrix(void);

public:
	CAGrid(AReal32 nSize = 30.0f, AReal32 nAreaSize = 1.0f);
	virtual ~CAGrid (void);

public:
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);

public:
	void MoveX(AReal32 fX);
	void MoveY(AReal32 fY);
	void MoveZ(AReal32 fZ);
	void MoveRotX(AReal32 fRotate);
	void MoveRotY(AReal32 fRotate);
	void MoveRotZ(AReal32 fRotate);

	void SetPos(AReal32 fX, AReal32 fY);
	void SetPos(AReal32 fX, AReal32 fY, AReal32 fZ);
	void SetPos(AVector3 vPos);
	void SetSize(AReal32 fSizeX, AReal32 fSizeY);
	void SetSize(AReal32 fSizeX, AReal32 fSizeY, AReal32 fSizeZ);
	void SetSize(AVector3 vSize);
	void SetOffset(AReal32 fX, AReal32 fY);
	void SetOffset(AReal32 fX, AReal32 fY, AReal32 fZ);
	void SetOffset(AVector3 vPos);
	void SetRotX(AReal32 fRotate);
	void SetRotY(AReal32 fRotate);
	void SetRotZ(AReal32 fRotate);

	void SetMatrix(D3DXMATRIX& mat);
};

#endif