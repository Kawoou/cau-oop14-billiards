#ifndef __APROJECT_OBJECT_MODEL_DEFINE__
#define __APROJECT_OBJECT_MODEL_DEFINE__

#pragma once

#include "AWindow.h"

#define MAX_BONE	50

// Skinning Mode
enum
{
	SKINNING_SW = 0,	// Software Simul
	SKINNING_FF,		// Fixed Func
	SKINNING_SH,		// Vertex & Pixel Shader
};

// Model Material
class CAModelMaterial
{
public:
	D3DMATERIAL9		m_Material;	
	float				m_fOpacity;	
	LPDIRECT3DTEXTURE9	m_pDiffuseTexture;		// Texture List (diffuse, normal, bump, etc...)
	LPDIRECT3DTEXTURE9	m_pBumpTexture;
	LPDIRECT3DTEXTURE9	m_pHeightTexture;
	LPDIRECT3DTEXTURE9	m_pLuminanceTexture;
	LPDIRECT3DTEXTURE9	m_pSpecularTexture;
	LPDIRECT3DTEXTURE9	m_pOpacityTexture;

public:
	CAModelMaterial ()
		:m_fOpacity (0.0f)
	{
		m_pDiffuseTexture = nullptr;
		m_pBumpTexture = nullptr;
		m_pHeightTexture = nullptr;
		m_pLuminanceTexture = nullptr;
		m_pSpecularTexture = nullptr;
		m_pOpacityTexture = nullptr;
	}
	~CAModelMaterial ()
	{
		SAFE_RELEASE (m_pDiffuseTexture);
		SAFE_RELEASE (m_pBumpTexture);
		SAFE_RELEASE (m_pHeightTexture);
		SAFE_RELEASE (m_pLuminanceTexture);
		SAFE_RELEASE (m_pSpecularTexture);
		SAFE_RELEASE (m_pOpacityTexture);
	}
};

// Vertex Index
struct CAVertexIndex
{
	int	m_nMaterialID;
	int	m_nIndex [3];
};

// UV Index
struct CAUVIndex
{
	int	m_nIndex [3];
};

// Vectex Weight
struct CAVertexWeight
{
	vector<int>		m_listBoneID;
	vector<float>	m_listWeight;

	DWORD			m_dwWeightIndex;	// [8bit : W3] [8bit : W2] [8bit : W1] [8bit : W0]
};

// Model Vertex-Index Buffer
class CAModelVIB
{
public:
	CAModelVIB ()
		: m_nNumMaterial(0)
	{
	}
	~CAModelVIB () {}

public:
	vector<D3DXVECTOR3>		m_listVertex;
	vector<D3DXVECTOR3>		m_listNormal;
	vector<D3DXVECTOR2>		m_listUV;
	vector<CAVertexIndex>	m_listIndex;
	vector<CAUVIndex>		m_listUVIndex;
	vector<CAVertexWeight>	m_listWeight;

	int						m_nNumMaterial;

	map<int, int>			m_mapIndex;		// key : Vertex Index, value : UV Index
};

#endif