#ifndef __APROJECT_GEOMETRY__
#define __APROJECT_GEOMETRY__

#pragma once

class CAGeometry : public CAObject
{
private:
	//LPDIRECT3DVERTEXDECLARATION9	m_pVertexDecl;
	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9	m_pIndexBuffer;
	AInt32					m_nVertexCount;
	AInt32					m_nIndexCount;

	D3DXMATRIXA16			m_matComplate;

private:
	bool					m_bFrustumCulling;
	bool					m_bFrustumState;

private:
	CATexture*				m_pTexture;
	CustomVertex*			m_pVertices;
	CustomIndex*			m_pIndices;
	DWORD					m_FVF;
	D3DPRIMITIVETYPE		m_nPrimitiveType;

public:
	CAGeometry (DWORD dwFVF, CustomVertex* pVertices, CustomIndex* pIndices, int nVC, int nIC);
	virtual ~CAGeometry (void);

public:
	bool GetPicking (RAY Ray);

	DWORD GetFVF (void);
	int GetIndexCount (void);
	int GetVertexCount (void);

	CATexture* GetTexture (void);
	CustomIndex* GetIndex (void);
	CustomVertex* GetVertex (void);
	D3DPRIMITIVETYPE GetPrimitiveType(void);

	void SetFVF (DWORD dwFVF);
	void SetTexture (CATexture* pTexture);
	void SetVertex (CustomVertex* pVertices, int nVC);
	void SetIndex (CustomIndex* pIndices, int nIC);
	void SetPrimitiveType (D3DPRIMITIVETYPE type);

public:
	HRESULT Create (void);
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);
};

#endif