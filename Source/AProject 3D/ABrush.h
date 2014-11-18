#ifndef __APROJECT_HEIGHTMAP_BLUSH__
#define __APROJECT_HEIGHTMAP_BLUSH__

#pragma once

struct BRUSH_VERTEX
{
	enum
	{
		FVF	= (D3DFVF_XYZ | D3DFVF_TEX1),	//!< 정점 포맷
	};

	D3DXVECTOR3	p;							//!< 정점 위치
	D3DXVECTOR2	t;							//!< 텍스쳐 u, v 좌표
};

struct BRUSH_INDEX
{
	DWORD	_0;								//!< 삼각형을 정의하는 첫 번째 점의 인덱스
	DWORD	_1;								//!< 삼각형을 정의하는 두 번째 점의 인덱스
	DWORD	_2;								//!< 삼각형을 정의하는 세 번째 점의 인덱스
};

class CABrush : public CAObject
{
private:
	CAMap*					m_pTerrain;

	LPDIRECT3DVERTEXBUFFER9	m_pVB;				//!< 정점 버퍼
	LPDIRECT3DINDEXBUFFER9	m_pIB;				//!< 인덱스 버퍼
	
	LPDIRECT3DTEXTURE9		m_pTexture;			//!< Diffuse 텍스쳐
	char					m_szDiffuse [256];	//!< Diffuse 텍스쳐 이름
	
	D3DXMATRIXA16			m_matWorld;			//!< 월드 변환 행렬
	
	AUInt32					m_uVertices;		//!< 정점 갯수
	AUInt32					m_uIndices;			//!< 폴리곤 갯수
	AUInt32					m_uCells;			//!< 모서리 기준 셀 갯수
	//AReal32					m_fScale;			//!< 크기 변화량			// m_vSize로 대체
	AReal32					m_fIntensity;		//!< 브러쉬 강도

private:
	void	DeepCopy (const CABrush& brush);
	void	Release ();

	HRESULT	InitVertexBuffer ();
	HRESULT	InitIndexBuffer ();

	void	RestoreRenderState (bool bBegin);

public:
	CABrush ();
	virtual ~CABrush ();
	CABrush (const CABrush& brush);
	CABrush& operator=(const CABrush& brush);

public:
	virtual HRESULT	Init (char* szDiffuse, AUInt32 uCells, CAMap* pTerrain);
	virtual void	Reset (void);
	virtual void	Update (void);
	virtual void	Render (void);

public:
	virtual void	SetDecal (AReal32 fX, AReal32 fZ);

	//virtual VOID	Scale (FLOAT fScale);		// SetSize로 대체
	//virtual AReal32	GetRadius ();			// GetSize로 대체

	virtual	void	SetIntensity (AReal32 fIntensity);
	virtual AReal32	GetIntensity ();
};

#endif