#ifndef __APROJECT_HEIGHTMAP__
#define __APROJECT_HEIGHTMAP__

#pragma once

struct TERRAIN_VERTEX
{
	enum
	{
		FVF	= (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2),	//!< ���� ����
	};

	D3DXVECTOR3	p;			//!< ���� ��ġ
	D3DXVECTOR3	n;			//!< ���� ����
	D3DXVECTOR2	t1;			//!< Alpha �ؽ��� u, v ��ǥ
	D3DXVECTOR2	t2;			//!< Diffuse �ؽ��� u, v ��ǥ
};

struct TERRAIN_INDEX
{
	DWORD _0;				//!< �ﰢ���� �����ϴ� ù ��° ������ �ε���
	DWORD _1;				//!< �ﰢ���� �����ϴ� �� ��° ������ �ε���
	DWORD _2;				//!< �ﰢ���� �����ϴ� �� ��° ������ �ε���
};

struct NORMAL_VERTEX
{
	enum
	{
		FVF	= D3DFVF_XYZ,	//!< ���� ����
	};

	D3DXVECTOR3	p;			//!< ���� ��ġ
};

struct NORMAL_INDEX
{
	DWORD _0;				//!< ������ �����ϴ� ù ��° ���� �ε���
	DWORD _1;				//!< ������ �����ϴ� �� ��° ���� �ε���
};

struct TILE_INFO
{
	enum
	{
		BASE,				//!< �⺻ ũ�� �ε���
		LEFT,				//!< ���� ���� ũ�� �ε���
		TOP,				//!< ���� ���� ũ�� �ε���
		RIGHT,				//!< ������ ���� ũ�� �ε���
		BOTTOM,				//!< �Ʒ��� ���� ũ�� �ε���

		MAX_CRACK,			//!< ũ�� ����
	};

	LPDIRECT3DVERTEXBUFFER9	pVB;		//!< ���� ����
	LPDIRECT3DVERTEXBUFFER9	pNormalVB;	//!< ���� ���� ����
	D3DXVECTOR3				vCenter;	//!< Ÿ�� �߽� ��ǥ
	FLOAT					fRadius;	//!< ��豸 ������
	UINT					uLevel;		//!< Ÿ�� ����
	UINT					uCracks;	//!< ũ�� �ε���
	UINT					uIndices;	//!< ������ ����

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
	D3DMATERIAL9	m_d3dMaterial;		//!< ���� ��ü
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

	UINT						m_uCellsPerTile;						//!< �𼭸� ���� Ÿ�ϴ� �� ����
	UINT						m_uTiles;								//!< �𼭸� ���� Ÿ�� ����
	UINT						m_uSize;								//!< �𼭸� ���� ���� ũ��
	
	vector<TERRAIN_VERTEX>		m_vecVertices;							//!< ���� ���� ����
	
	TILE_INFO*					m_pTileInfo;							//!< Ÿ�� ����
	LPDIRECT3DINDEXBUFFER9		(*m_ppTileIB) [TILE_INFO::MAX_CRACK];	//!< Ÿ�� ���� �ε��� ����
	LPDIRECT3DINDEXBUFFER9		m_pTileNormalIB;						//!< Ÿ�� ���� ���� ���Ϳ� �ε��� ����

	UINT						m_uMaxLevel;							//!< �ִ� ���� ��

	D3DXMATRIXA16				m_matWorld;								//!< ���� ��ȯ ���
	D3DXMATRIXA16				m_matScale;								//!< �� ũ�� ��ȯ ���

	vector<AUInt32>				m_vecVisibleIndex;						//!< ȭ�鿡 ���̴� Ÿ�� �ε���
	CAQuadTree*					m_pQuadTree;							//!< ���� Ʈ�� ��ü

	UINT						m_uLODLevel;							//!< LOD ����

private:
	bool						m_bLOD;									//!< LOD ���� �÷���
	bool						m_bRenderNormal;						//!< Normal ������ �÷���

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