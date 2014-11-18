#ifndef __APROJECT_HEIGHTMAP_BLUSH__
#define __APROJECT_HEIGHTMAP_BLUSH__

#pragma once

struct BRUSH_VERTEX
{
	enum
	{
		FVF	= (D3DFVF_XYZ | D3DFVF_TEX1),	//!< ���� ����
	};

	D3DXVECTOR3	p;							//!< ���� ��ġ
	D3DXVECTOR2	t;							//!< �ؽ��� u, v ��ǥ
};

struct BRUSH_INDEX
{
	DWORD	_0;								//!< �ﰢ���� �����ϴ� ù ��° ���� �ε���
	DWORD	_1;								//!< �ﰢ���� �����ϴ� �� ��° ���� �ε���
	DWORD	_2;								//!< �ﰢ���� �����ϴ� �� ��° ���� �ε���
};

class CABrush : public CAObject
{
private:
	CAMap*					m_pTerrain;

	LPDIRECT3DVERTEXBUFFER9	m_pVB;				//!< ���� ����
	LPDIRECT3DINDEXBUFFER9	m_pIB;				//!< �ε��� ����
	
	LPDIRECT3DTEXTURE9		m_pTexture;			//!< Diffuse �ؽ���
	char					m_szDiffuse [256];	//!< Diffuse �ؽ��� �̸�
	
	D3DXMATRIXA16			m_matWorld;			//!< ���� ��ȯ ���
	
	AUInt32					m_uVertices;		//!< ���� ����
	AUInt32					m_uIndices;			//!< ������ ����
	AUInt32					m_uCells;			//!< �𼭸� ���� �� ����
	//AReal32					m_fScale;			//!< ũ�� ��ȭ��			// m_vSize�� ��ü
	AReal32					m_fIntensity;		//!< �귯�� ����

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

	//virtual VOID	Scale (FLOAT fScale);		// SetSize�� ��ü
	//virtual AReal32	GetRadius ();			// GetSize�� ��ü

	virtual	void	SetIntensity (AReal32 fIntensity);
	virtual AReal32	GetIntensity ();
};

#endif