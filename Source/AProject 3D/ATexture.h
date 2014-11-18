#ifndef __APROJECT_OBJECT_TEXTURE__
#define __APROJECT_OBJECT_TEXTURE__

#pragma once

typedef struct _MultiTextureFlag
{
	DWORD	m_dwTexcoordIndex;
	DWORD	m_dwColorOP;
	DWORD	m_dwColorArg1;
	DWORD	m_dwColorArg2;
	DWORD	m_dwAlphaOP;
	DWORD	m_dwAlphaArg1;
	DWORD	m_dwAlphaArg2;

	_MultiTextureFlag (void)
	{
		m_dwTexcoordIndex = 0;
		m_dwColorOP = D3DTOP_MODULATE;
		m_dwColorArg1 = D3DTA_TEXTURE;
		m_dwColorArg2 = D3DTA_TFACTOR;
		m_dwAlphaOP = D3DTOP_MODULATE;
		m_dwAlphaArg1 = D3DTA_TEXTURE;
		m_dwAlphaArg2 = D3DTA_TFACTOR;
	};
	_MultiTextureFlag (DWORD dwTexcoordIndex, DWORD dwColorOP, DWORD dwColorArg1, DWORD dwColorArg2, DWORD dwAlphaOP, DWORD dwAlphaArg1, DWORD dwAlphaArg2)
	{
		m_dwTexcoordIndex = dwTexcoordIndex;
		m_dwColorOP = dwColorOP;
		m_dwColorArg1 = dwColorArg1;
		m_dwColorArg2 = dwColorArg2;
		m_dwAlphaOP = dwAlphaOP;
		m_dwAlphaArg1 = dwAlphaArg1;
		m_dwAlphaArg2 = dwAlphaArg2;
	};
} MultiTextureFlag;

typedef struct
{
	AReal32 x, y, z, rhw;
	AReal32 u, v;
}Texture2DVertex;

class CATexture : public CAObject
{
private:
	LPDIRECT3DTEXTURE9			m_pTexture;
	D3DPOOL						m_dwPoolingType;
	D3DSURFACE_DESC				m_descSurface;
	MultiTextureFlag			m_mtFlag;

private:
	AColor						m_cColor;
	bool						m_bBlend;

	AInt						m_nTextureWidthSize;
	AInt						m_nTextureHeightSize;
	AUInt32						m_dwColorkey;
	char*						m_pTextureFilepath;

private:
	LPDIRECT3DVERTEXBUFFER9		m_pVertexBuffer;

public:
	CATexture (D3DPOOL dwPoolingType);
	virtual ~CATexture ();

public:
	void SetTexture (LPDIRECT3DTEXTURE9 pTexture, AUInt Width = -1, AUInt Height = -1);		//!< 텍스쳐를 지워주지 않는 함수 (처음 한번만 실행 권장)
	void SetTexture (const char* pTextureFilepath, AUInt Width = -1, AUInt Height = -1, AUInt32 dwColorKey = 0x00000000);
	void SetDrawFlag (MultiTextureFlag mtFlag);
	void SetColor (AColor cColor);
	void SetBlend (bool bState);

	LPDIRECT3DTEXTURE9 GetTexture (void);
	MultiTextureFlag GetDrawFlag (void);
	AColor GetColor (void);
	bool GetBlend (void);
	AInt GetTextureWidthSize (void);
	AInt GetTextureWidthHalfSize (void);
	AInt GetTextureHeightSize (void);
	AInt GetTextureHeightHalfSize (void);

	D3DPOOL GetPoolingType (void);

public:
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);
};

#endif