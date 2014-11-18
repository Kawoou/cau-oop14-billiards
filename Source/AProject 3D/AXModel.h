#ifndef __APROJECT_OBJECT_X_MODEL__
#define __APROJECT_OBJECT_X_MODEL__

#pragma once

class CAXModel : public CAObject
{
private:
	LPD3DXMESH						m_pMeshObject;
	DWORD							m_dwNumMaterials;

	D3DMATERIAL9*					m_pMaterials;
	CHAR(*m_strMaterials)[MAX_PATH];
	IDirect3DBaseTexture9**			m_pTextures;
	IDirect3DVertexBuffer9*			m_pVB;
	IDirect3DIndexBuffer9*			m_pIB;
	IDirect3DVertexDeclaration9*	m_pDecl;
	DWORD							m_dwNumVertices;
	DWORD							m_dwNumFaces;
	DWORD							m_dwBytesPerVertex;

	AVector3						m_vAnchorPoint;
	D3DXMATRIX						m_matComplate;

	bool							m_bUpdateMatrix;

	std::string						m_strFile;

private:
	AReal32							m_fOldRotX;
	AReal32							m_fOldRotY;
	AReal32							m_fOldRotZ;

private:
	void UpdateMatrix(void);

public:
	CAXModel(void);
	virtual ~CAXModel(void);

public:
	bool Create(const char* strFileName);

private:
	bool Load(const char* strFileName);
	void Unload(void);

public:
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Render(void);

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

	void SetAnchorPoint(AVector3 vector);
	AVector3 GetAnchorPoint();

	void SetMatrix(D3DXMATRIX& mat);
};

#endif