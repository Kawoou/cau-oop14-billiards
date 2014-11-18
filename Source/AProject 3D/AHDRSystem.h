#ifndef __APROJECT_HDRLIGHT_SYSTEM__
#define __APROJECT_HDRLIGHT_SYSTEM__

#pragma once

class CAHDRLighting
{
private:
	static const AUInt8 MAX_SAMPLES				= 16;					// Maximum number of texture grabs
	static const AUInt8 MAX_LIGHTS				= 8;					// Number of max light count
	static const AUInt8 NUM_TONEMAP_TEXTURES	= 4;					// Number of stages in the 4x4 down-scaling of average luminance textures
	static const AUInt8 NUM_STAR_TEXTURES		= 12;					// Number of textures used for the star post-processing effect
	static const AUInt8 NUM_BLOOM_TEXTURES		= 3;					// Number of textures used for the bloom post-processing effect

private:
	// Texture coordinate rectangle
	struct CoordRect
	{
		AReal32 fLeftU, fTopV;
		AReal32 fRightU, fBottomV;
	};

	// World vertex format
	struct WorldVertex
	{
		D3DXVECTOR3 p;		// position
		D3DXVECTOR3 n;		// normal
		D3DXVECTOR2 t;		// texture coordinate

		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	};

	// Screen quad vertex format
	struct ScreenVertex
	{
		D3DXVECTOR4 p;		// position
		D3DXVECTOR2 t;		// texture coordinate

		static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_TEX1;
	};

private:
	AUInt32 NUM_LIGHTS;
	AReal32 EMISSIVE_COEFFICIENT;

public:
	//LPDIRECT3DDEVICE9			m_pD3DDevice;
	LPD3DXEFFECT				m_pEffect;
	D3DFORMAT					m_LuminanceFormat;							// Format to use for luminance map

protected:
	LPDIRECT3DSURFACE9			m_pFloatMSRT;								// Multi-Sample float render target
	LPDIRECT3DSURFACE9			m_pFloatMSDS;								// Depth Stencil surface for the float RT
	LPDIRECT3DTEXTURE9			m_pTexScene;								// HDR render target containing the scene
	LPDIRECT3DTEXTURE9			m_pTexSceneScaled;							// Scaled copy of the HDR scene
	LPDIRECT3DTEXTURE9			m_pTexBrightPass;							// Bright-pass filtered copy of the scene
	LPDIRECT3DTEXTURE9			m_pTexAdaptedLuminanceCur;					// The luminance that the user is currenly adapted to
	LPDIRECT3DTEXTURE9			m_pTexAdaptedLuminanceLast;					// The luminance that the user is currenly adapted to
	LPDIRECT3DTEXTURE9			m_pTexStarSource;							// Star effect source texture
	LPDIRECT3DTEXTURE9			m_pTexBloomSource;							// Bloom effect source texture
	
	//LPDIRECT3DTEXTURE9			m_pTexWall;									// Stone texture for the room walls
	//LPDIRECT3DTEXTURE9			m_pTexFloor;								// Concrete texture for the room floor
	//LPDIRECT3DTEXTURE9			m_pTexCeiling;								// Plaster texture for the room ceiling
	//LPDIRECT3DTEXTURE9			m_pTexPainting;								// Texture for the paintings on the wall

	LPDIRECT3DTEXTURE9			m_apTexBloom [NUM_BLOOM_TEXTURES];			// Blooming effect working textures
	LPDIRECT3DTEXTURE9			m_apTexStar [NUM_STAR_TEXTURES];			// Star effect working textures
	LPDIRECT3DTEXTURE9			m_apTexToneMap [NUM_TONEMAP_TEXTURES];		// Log average luminance samples from the HDR render target

protected:
	//LPD3DXMESH					m_pWorldMesh;								// Mesh to contain world objects
	LPD3DXMESH					m_pmeshSphere;								// Representation of point light
	
public:
	CAGlareDefine				m_GlareDef;									// Glare defintion
	EGLARELIBTYPE				m_eGlareType;								// Enumerated glare type

protected:
	D3DXVECTOR4					m_avLightPosition [MAX_LIGHTS];				// Light positions in world space
	D3DXVECTOR4					m_avLightIntensity [MAX_LIGHTS];			// Light floating point intensities
	AInt32						m_nLightLogIntensity [MAX_LIGHTS];			// Light intensities on a log scale
	AInt32						m_nLightMantissa [MAX_LIGHTS];				// Mantissa of the light intensity

	DWORD						m_dwCropWidth;								// Width of the cropped scene texture
	DWORD						m_dwCropHeight;								// Height of the cropped scene texture
	
	AReal32						m_fKeyValue;								// Middle gray key value for tone mapping
	
	bool						m_bToneMap;									// True when scene is to be tone mapped            
	bool						m_bAdaptationInvalid;						// True when adaptation level needs refreshing
	bool						m_bUseMultiSampleFloat16;					// True when using multisampling on a floating point back buffer
	D3DMULTISAMPLE_TYPE			m_MaxMultiSampleType;						// Non-Zero when g_bUseMultiSampleFloat16 is true
	DWORD						m_dwMultiSampleQuality;						// Non-Zero when we have multisampling on a float backbuffer
	bool						m_bSupportsD16;
	bool						m_bSupportsD32;
	bool						m_bSupportsD24X8;

private:
	AReal32						fRElapsedTime;
	LPDIRECT3DSURFACE9			pRSurfLDR;									// Low dynamic range surface for final output
	LPDIRECT3DSURFACE9			pRSurfDS;									// Low dynamic range depth stencil surface
	LPDIRECT3DSURFACE9			pRSurfHDR;									// High dynamic range surface to store 
	// intermediate floating point color values

public:
	CAHDRLighting ()
	{
		NUM_LIGHTS = 0;														// Number of lights in the scene
		EMISSIVE_COEFFICIENT = 39.78f;										// Emissive color multiplier for each lumen of light intensity

		m_bUseMultiSampleFloat16 = false;
		m_MaxMultiSampleType = D3DMULTISAMPLE_NONE;
		m_dwMultiSampleQuality = 0;
		m_bSupportsD16 = false;
		m_bSupportsD32 = false;
		m_bSupportsD24X8 = false;

		// Set light positions in world space
		m_avLightPosition [0] = D3DXVECTOR4 (4.0f, 100.0f, 18.0f, 1.0f);
		m_avLightPosition [1] = D3DXVECTOR4 (11.0f, 2.0f, 18.0f, 1.0f);

		m_eGlareType = GLT_CAMERA;
		m_GlareDef.Initialize (m_eGlareType);

		m_nLightMantissa [0] = 1 + (350 % 9);
		m_nLightLogIntensity [0] = -4 + (350 / 9);
		m_nLightMantissa [1] = 1 + (40 % 9);
		m_nLightLogIntensity [1] = -4 + (40 / 9);
		RefreshLights ();

		m_fKeyValue = 0.18f;
		m_bToneMap = true;
	}
	~CAHDRLighting ()
	{
	}

public:
	AUInt8 AddLight (D3DXVECTOR4 vPosition, AUInt32 nPower);
	void ClearLight ();
	void SetLightPosition (AUInt32 nIndex, D3DXVECTOR4 vPosition);
	void SetLightLuminosity (AUInt32 nIndex, AUInt32 nPower);

public:
	HRESULT LoadMesh (LPDIRECT3DDEVICE9 pD3DDevice, WCHAR* strFileName, LPD3DXMESH* ppMesh);
	
	// Scene geometry initialization routines
	//HRESULT BuildWorldMesh ();
	//void SetTextureCoords (WorldVertex* pVertex, float u, float v);
	//HRESULT BuildColumn (WorldVertex*& pV, float x, float y, float z, float width);

	// Post-processing source textures creation
	HRESULT Scene_To_SceneScaled (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT SceneScaled_To_BrightPass (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT BrightPass_To_StarSource (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT StarSource_To_BloomSource (LPDIRECT3DDEVICE9 pD3DDevice);

	// Post-processing helper functions
	HRESULT GetTextureRect (LPDIRECT3DTEXTURE9 pTexture, RECT* pRect);
	HRESULT GetTextureCoords (LPDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, LPDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords);

	// Sample offset calculation. These offsets are passed to corresponding pixel shaders.
	HRESULT GetSampleOffsets_GaussBlur5x5 (DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR2* avTexCoordOffset, D3DXVECTOR4* avSampleWeights, AReal32 fMultiplier = 1.0f);
	HRESULT GetSampleOffsets_Bloom (DWORD dwD3DTexSize, AReal32 afTexCoordOffset [15], D3DXVECTOR4* avColorWeight, AReal32 fDeviation, AReal32 fMultiplier = 1.0f);
	HRESULT GetSampleOffsets_Star (DWORD dwD3DTexSize, AReal32 afTexCoordOffset [15], D3DXVECTOR4* avColorWeight, AReal32 fDeviation);
	HRESULT GetSampleOffsets_DownScale4x4 (DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets []);
	HRESULT GetSampleOffsets_DownScale2x2 (DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets []);

	// Tone mapping and post-process lighting effects
	HRESULT MeasureLuminance (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT CalculateAdaptation (LPDIRECT3DDEVICE9 pD3DDevice, float fElapsedTime);
	HRESULT RenderStar (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT RenderBloom (LPDIRECT3DDEVICE9 pD3DDevice);

	// Methods to control scene lights
	HRESULT AdjustLight (UINT iLight, bool bIncrement);
	HRESULT RefreshLights ();

public:
	HRESULT RenderScene (LPDIRECT3DDEVICE9 pD3DDevice);
	HRESULT ClearTexture (LPDIRECT3DDEVICE9 pD3DDevice, LPDIRECT3DTEXTURE9 pTexture);

public:
	void DrawFullScreenQuad (LPDIRECT3DDEVICE9 pD3DDevice, float fLeftU, float fTopV, float fRightU, float fBottomV);
	void DrawFullScreenQuad (LPDIRECT3DDEVICE9 pD3DDevice, CoordRect c)
	{
		DrawFullScreenQuad (pD3DDevice, c.fLeftU, c.fTopV, c.fRightU, c.fBottomV);
	}

public:
	bool CheckDevice (D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed);

public:
	HRESULT Create (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT Reset (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	void Update (float fElapsedTime);
	bool PreRender (LPDIRECT3DDEVICE9 pD3DDevice);
	void PostRender (LPDIRECT3DDEVICE9 pD3DDevice);
	//void Render (LPDIRECT3DDEVICE9 pD3DDevice, float fElapsedTime);
	void Lost ();
	void Destroy ();
};

#endif