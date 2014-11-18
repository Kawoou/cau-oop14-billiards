#ifndef __APROJECT_HEIGHTMAP__
#define __APROJECT_HEIGHTMAP__

#pragma once

struct TERRAIN_VERTEX
{
	enum
	{
		FVF	= (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2),	//!< 정점 포맷
	};

	D3DXVECTOR3	p;			//!< 정점 위치
	D3DXVECTOR3	n;			//!< 법선 벡터
	D3DXVECTOR2	t1;			//!< Alpha 텍스쳐 u, v 좌표
	D3DXVECTOR2	t2;			//!< Diffuse 텍스쳐 u, v 좌표
};

struct TERRAIN_INDEX
{
	DWORD _0;				//!< 삼각형을 정의하는 첫 번째 정점의 인덱스
	DWORD _1;				//!< 삼각형을 정의하는 두 번째 정점의 인덱스
	DWORD _2;				//!< 삼각형을 정의하는 세 번째 정점의 인덱스
};

struct NORMAL_VERTEX
{
	enum
	{
		FVF	= D3DFVF_XYZ,	//!< 정점 포맷
	};

	D3DXVECTOR3	p;			//!< 정점 위치
};

struct NORMAL_INDEX
{
	DWORD _0;				//!< 선분을 정의하는 첫 번째 점의 인덱스
	DWORD _1;				//!< 선분을 정의하는 두 번째 점의 인덱스
};

struct TILE_INFO
{
	enum
	{
		BASE,				//!< 기본 크랙 인덱스
		LEFT,				//!< 왼쪽 방향 크랙 인덱스
		TOP,				//!< 위쪽 방향 크랙 인덱스
		RIGHT,				//!< 오른쪽 방향 크랙 인덱스
		BOTTOM,				//!< 아랫쪽 방향 크랙 인덱스

		MAX_CRACK,			//!< 크랙 갯수
	};

	LPDIRECT3DVERTEXBUFFER9	pVB;		//!< 정점 버퍼
	LPDIRECT3DVERTEXBUFFER9	pNormalVB;	//!< 법선 정점 버퍼
	D3DXVECTOR3				vCenter;	//!< 타일 중심 좌표
	FLOAT					fRadius;	//!< 경계구 반지름
	UINT					uLevel;		//!< 타일 레벨
	UINT					uCracks;	//!< 크랙 인덱스
	UINT					uIndices;	//!< 폴리곤 갯수

	TILE_INFO ()
		:pVB (NULL), pNormalVB (NULL), vCenter (0.0f, 0.0f, 0.0f), fRadius (0.0f), uLevel (0), uCracks (0), uIndices (0)
	{
	}
};

class CAMaterial
{
public:
	static	CAMaterial*	GetSingleton ();

	void	SetDiffuse (AReal32 a, AReal32 r, AReal32 g, AReal32 b);
	void	SetAmbient (AReal32 a, AReal32 r, AReal32 g, AReal32 b);
	void	SetSpecular (AReal32 a, AReal32 r, AReal32 g, AReal32 b);
	void	SetEmissive (AReal32 a, AReal32 r, AReal32 g, AReal32 b);
	void	SetPower (AReal32 fPower);

	void	SetMaterial (void);

private:
	CAMaterial ();
	~CAMaterial (){}
	CAMaterial (const CAMaterial& material) { }
	CAMaterial& operator=(const CAMaterial& material) { return (*this); }

	void SetColorValue (D3DCOLORVALUE& d3dColorValue, AReal32 a, AReal32 r, AReal32 g, AReal32 b);

private:
	D3DMATERIAL9	m_d3dMaterial;		//!< 재질 객체
};

class CAMap : public CAObject
{
public:
	struct SPLATTING_MAP
	{
		char* szSplatting;

		LPDIRECT3DTEXTURE9 pTexDiffuse;
		LPDIRECT3DTEXTURE9 pTexAlpha;
	};

private:
	LPDIRECT3DTEXTURE9			m_pTexHeight;
	vector<SPLATTING_MAP>		m_pTexSplatting;

	LPDIRECT3DTEXTURE9			m_pTexNormalMap;
	LPDIRECT3DTEXTURE9			m_pTexSpecularMap;
	LPDIRECT3DTEXTURE9			m_pTexLightMap;

	UINT						m_uCellsPerTile;						//!< 모서리 기준 타일당 셀 갯수
	UINT						m_uTiles;								//!< 모서리 기준 타일 갯수
	UINT						m_uSize;								//!< 모서리 기준 지형 크기
	
	vector<TERRAIN_VERTEX>		m_vecVertices;							//!< 지형 정점 정보
	
	TILE_INFO*					m_pTileInfo;							//!< 타일 정보
	LPDIRECT3DINDEXBUFFER9		(*m_ppTileIB) [TILE_INFO::MAX_CRACK];	//!< 타일 단위 인덱스 버퍼
	LPDIRECT3DINDEXBUFFER9		m_pTileNormalIB;						//!< 타일 단위 법선 벡터용 인덱스 버퍼

	UINT						m_uMaxLevel;							//!< 최대 레벨 수

	D3DXMATRIXA16				m_matWorld;								//!< 월드 변환 행렬
	D3DXMATRIXA16				m_matScale;								//!< 맵 크기 변환 행렬

	vector<AUInt32>				m_vecVisibleIndex;						//!< 화면에 보이는 타일 인덱스
	CAQuadTree*					m_pQuadTree;							//!< 쿼드 트리 객체

	UINT						m_uLODLevel;							//!< LOD 레벨

private:
	bool						m_bLOD;									//!< LOD 설정 플래그
	bool						m_bRenderNormal;						//!< Normal 렌더링 플래그

private:
	void	Release ();

	HRESULT	InitVertexBuffer (void);
	HRESULT	InitIndexBuffer (void);
	HRESULT	InitTexture (void);
	HRESULT	InitAlphaTexture (void);
	void	BeginRenderState (void);
	void	RestoreRenderState (bool bBegin);

	HRESULT	InitQuadTree ();
	HRESULT	SetTileCenter ();
	VOID	SetCrackIndex ();

	HRESULT	InitNormalVertexBuffer (void);
	HRESULT	InitNormalIndexBuffer (void);

	HRESULT	SaveHeightMap (char* szHeightMap);
	HRESULT SaveDiffuseMap (void);
	HRESULT	SaveAlphaMap (void);

	HRESULT	LoadHeightMap (void* pHeightMap, AUInt32 nSize);
	HRESULT LoadDiffuseMap (SHANDLE pHandle);
	HRESULT	LoadAlphaMap (SHANDLE pHandle);

	HRESULT	ComputNormal ();

	// Update
	void	Culling (CAFrustum* pFrustum);
	void	SetLODLevel (const D3DXVECTOR3& vLookAt);

	// Render
	void	SetTransform (void);
	void	RenderNormal (void);

private:
	CAMap (const CAMap& terrain) { }
	CAMap& operator=(const CAMap& terrain)
	{
		return (*this);
	}

public:
	CAMap ();
	virtual ~CAMap ();

public:
	void SetLODUse (bool bLOD);
	void SetNormalRender (bool bRender);
	
	bool GetLODUse (void);
	bool GetNormalRender (void);

public:
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);

	void AddSplattingMap (char* szSplatting);

	HRESULT	Init (AUInt32 uLODLevel, AUInt32 uTiles, AUInt32 uCellsPerTile = 0);

	AReal32	GetHeight (AReal32 fX, AReal32 fZ);

	D3DXVECTOR3	Intersect ();

	void	EditTerrainCircle (AReal32 fX, AReal32 fZ, AReal32 fHeightRate, AReal32 fRadius);
	void	SplattingTerrain (AUInt32 uIndex, AReal32 fX, AReal32 fZ, AReal32 fRadius, AReal32 fIntensity, bool bErase = false);

	HRESULT	SaveTerrain (char* szMapFile, char* szPassword);
	HRESULT	LoadTerrain (char* szMapFile, char* szPassword);

	D3DXVECTOR3	GetNormal (AReal32 fX, AReal32 fZ);
};

#endif