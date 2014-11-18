#include "AWindow.h"

/**
	@brief			: AdjustLight
	@description	: Increment or decrement the light at the given index
*/
HRESULT CAHDRLighting::AdjustLight (UINT iLight, bool bIncrement)
{
	if (iLight >= NUM_LIGHTS)
		return E_INVALIDARG;

	if (bIncrement && m_nLightLogIntensity [iLight] < 7)
	{
		m_nLightMantissa [iLight] ++;
		if (m_nLightMantissa [iLight] > 9)
		{
			m_nLightMantissa [iLight] = 1;
			m_nLightLogIntensity [iLight] ++;
		}
	}

	if (!bIncrement && m_nLightLogIntensity [iLight] > -4)
	{
		m_nLightMantissa [iLight] --;
		if (m_nLightMantissa [iLight] < 1)
		{
			m_nLightMantissa [iLight] = 9;
			m_nLightLogIntensity [iLight] --;
		}
	}

	RefreshLights ();
	return S_OK;
}


/**
	@brief			: RefreshLights
	@description	: Set the light intensities to match the current log luminance
*/
HRESULT CAHDRLighting::RefreshLights ()
{
	for (UINT i = 0; i < NUM_LIGHTS; ++i)
	{
		m_avLightIntensity [i].x = m_nLightMantissa [i] * (float)pow (10.0f, m_nLightLogIntensity [i]);
		m_avLightIntensity [i].y = m_nLightMantissa [i] * (float)pow (10.0f, m_nLightLogIntensity [i]);
		m_avLightIntensity [i].z = m_nLightMantissa [i] * (float)pow (10.0f, m_nLightLogIntensity [i]);
		m_avLightIntensity [i].w = 1.0f;
	}

	return S_OK;
}

bool CAHDRLighting::CheckDevice (D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed)
{
	// Skip backbuffer formats that don't support alpha blending
	LPDIRECT3D9 pD3D = APROJECT_WINDOW->GetD3D ();
	if (FAILED (pD3D->CheckDeviceFormat (pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, BackBufferFormat) ))
		return false;

	if (pCaps->PixelShaderVersion < D3DPS_VERSION (2, 0) )
		return false;

	// No fallback yet, so need to support D3DFMT_A16B16G16R16F render target
	if (FAILED (pD3D->CheckDeviceFormat (pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F) ))
		return false;

	// No fallback yet, so need to support D3DFMT_R32F or D3DFMT_R16F render target
	if (FAILED (pD3D->CheckDeviceFormat (pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F) ))
		if (FAILED (pD3D->CheckDeviceFormat (pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R16F) ))
			return false;

	// Need to support post-pixel processing
	if (FAILED (pD3D->CheckDeviceFormat (pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_SURFACE, BackBufferFormat) ))
		return false;

	return true;
}

HRESULT CAHDRLighting::Create (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	// Determine which of D3DFMT_R16F or D3DFMT_R32F to use for luminance texture
	D3DCAPS9 Caps;
	LPDIRECT3D9 pD3D = APROJECT_WINDOW->GetD3D ();
	if (!pD3D)
		return E_FAIL;

	D3DDISPLAYMODE DisplayMode;
	pD3DDevice->GetDeviceCaps (&Caps);
	pD3DDevice->GetDisplayMode (0, &DisplayMode);
	// IsDeviceAcceptable already ensured that one of D3DFMT_R16F or D3DFMT_R32F is available.
	if (FAILED (pD3D->CheckDeviceFormat (Caps.AdapterOrdinal, Caps.DeviceType, DisplayMode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R16F) ))
		m_LuminanceFormat = D3DFMT_R32F;
	else
		m_LuminanceFormat = D3DFMT_R16F;

	// Determine whether we can support multisampling on a A16B16G16R16F render target
	m_bUseMultiSampleFloat16 = false;
	m_MaxMultiSampleType = D3DMULTISAMPLE_NONE;
	D3DPRESENT_PARAMETERS* settings = APROJECT_WINDOW->GetSetting ();
	for (D3DMULTISAMPLE_TYPE imst = D3DMULTISAMPLE_2_SAMPLES; imst <= D3DMULTISAMPLE_16_SAMPLES; imst = (D3DMULTISAMPLE_TYPE)(imst + 1) )
	{
		DWORD msQuality = 0;
		if (SUCCEEDED (pD3D->CheckDeviceMultiSampleType (Caps.AdapterOrdinal, Caps.DeviceType, D3DFMT_A16B16G16R16F, settings->Windowed, imst, &msQuality) ))
		{
			m_bUseMultiSampleFloat16 = true;
			m_MaxMultiSampleType = imst;
			if (msQuality > 0)
				m_dwMultiSampleQuality = msQuality - 1;
			else
				m_dwMultiSampleQuality = msQuality;
		}
	}

	m_bSupportsD16 = false;
	if (SUCCEEDED (pD3D->CheckDeviceFormat (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16) ))
		if (SUCCEEDED (pD3D->CheckDepthStencilMatch (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DFMT_A16B16G16R16F, D3DFMT_D16) ))
			m_bSupportsD16 = true;

	m_bSupportsD32 = false;
	if (SUCCEEDED (pD3D->CheckDeviceFormat (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32) ))
		if (SUCCEEDED (pD3D->CheckDepthStencilMatch (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DFMT_A16B16G16R16F, D3DFMT_D32) ))
			m_bSupportsD32 = true;

	m_bSupportsD24X8 = false;
	if (SUCCEEDED (pD3D->CheckDeviceFormat (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8) ))
		if (SUCCEEDED (pD3D->CheckDepthStencilMatch (Caps.AdapterOrdinal, Caps.DeviceType, settings->BackBufferFormat, D3DFMT_A16B16G16R16F, D3DFMT_D24X8) ))
			m_bSupportsD24X8 = true;

	// Shader Flags
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
#if defined (DEBUG) || defined (_DEBUG)
	// Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// Read the D3DX effect file
	// If this fails, there should be debug output as to 
	// they the .fx file failed to compile
	D3DXCreateEffectFromFile (pD3DDevice, L"./Data/HDRLighting.fx", nullptr, nullptr, dwShaderFlags, NULL, &m_pEffect, NULL);

	return S_OK;
}

AUInt8 CAHDRLighting::AddLight (D3DXVECTOR4 vPosition, AUInt32 nPower)
{
	m_avLightPosition [NUM_LIGHTS] = vPosition;

	m_nLightMantissa [NUM_LIGHTS] = 1 + (nPower % 9);
	m_nLightLogIntensity [NUM_LIGHTS ++] = -4 + (nPower / 9);

	RefreshLights ();

	return NUM_LIGHTS - 1;
}

void CAHDRLighting::ClearLight ()
{
	NUM_LIGHTS = 0;

	RefreshLights ();
}

void CAHDRLighting::SetLightPosition (AUInt32 nIndex, D3DXVECTOR4 vPosition)
{
	m_avLightPosition [nIndex] = vPosition;
}

void CAHDRLighting::SetLightLuminosity (AUInt32 nIndex, AUInt32 nPower)
{
	m_nLightMantissa [nIndex] = 1 + (nPower % 9);
	m_nLightLogIntensity [nIndex] = -4 + (nPower / 9);

	RefreshLights ();
}

/**
	@brief			: LoadMesh
	@description	: This function loads the mesh and ensures the mesh has normals
					  It also optimizes the mesh for the graphics card's vertex cache,
					  which improces performance by organizing the internal triangle list for less cache misses.
*/
HRESULT CAHDRLighting::LoadMesh (LPDIRECT3DDEVICE9 pD3DDevice, WCHAR* strFileName, ID3DXMesh** ppMesh)
{
	LPD3DXMESH pMesh = NULL;
	//HRESULT hr;

	// Load the mesh with D3DX and get back a ID3DXMesh*.  For this
	// sample we'll ignore the X file's embedded materials since we know 
	// exactly the model we're loading.  See the mesh samples such as
	// "OptimizedMesh" for a more generic mesh loading example.
	D3DXLoadMeshFromX (strFileName, D3DXMESH_MANAGED, pD3DDevice, nullptr, nullptr, nullptr, nullptr, &pMesh);

	DWORD* rgdwAdjacency = NULL;
	// Make sure there are normals which are required for lighting
	if (!(pMesh->GetFVF () & D3DFVF_NORMAL) )
	{
		LPD3DXMESH pTempMesh;
		pMesh->CloneMeshFVF (pMesh->GetOptions (), pMesh->GetFVF () | D3DFVF_NORMAL, pD3DDevice, &pTempMesh);
		D3DXComputeNormals (pTempMesh, nullptr);

		SAFE_RELEASE (pMesh);
		pMesh = pTempMesh;
	}

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     
	rgdwAdjacency = new DWORD [pMesh->GetNumFaces () * 3];
	if (rgdwAdjacency == NULL)
		return E_OUTOFMEMORY;

	pMesh->GenerateAdjacency (1e-6f, rgdwAdjacency);
	pMesh->OptimizeInplace (D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, nullptr, nullptr, nullptr);
	delete [] rgdwAdjacency;

	*ppMesh = pMesh;

	return S_OK;
}

HRESULT CAHDRLighting::Reset (LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	int i = 0; // loop variable

	if (m_pEffect)
		if (FAILED (hr = m_pEffect->OnResetDevice () ) )
		{
			return E_FAIL;
		}

	// Create a sprite to help batch calls when drawing many lines of text
	const D3DSURFACE_DESC* pBackBufferDesc = pBackBufferSurfaceDesc;

	// Create the Multi-Sample floating point render target
	D3DFORMAT dfmt = D3DFMT_UNKNOWN;
	if (m_bSupportsD16)
		dfmt = D3DFMT_D16;
	else if (m_bSupportsD32)
		dfmt = D3DFMT_D32;
	else if (m_bSupportsD24X8)
		dfmt = D3DFMT_D24X8;
	else
		m_bUseMultiSampleFloat16 = false;

	if (m_bUseMultiSampleFloat16)
	{
		hr = pD3DDevice->CreateRenderTarget (pBackBufferDesc->Width, pBackBufferDesc->Height, D3DFMT_A16B16G16R16F, m_MaxMultiSampleType, m_dwMultiSampleQuality, FALSE, &m_pFloatMSRT, NULL);
		if (FAILED (hr) )
			m_bUseMultiSampleFloat16 = false;
		else
		{
			hr = pD3DDevice->CreateDepthStencilSurface (pBackBufferDesc->Width, pBackBufferDesc->Height, dfmt, m_MaxMultiSampleType, m_dwMultiSampleQuality, TRUE, &m_pFloatMSDS, NULL);
			if (FAILED (hr) )
			{
				m_bUseMultiSampleFloat16 = false;
				SAFE_RELEASE (m_pFloatMSRT);
			}
		}
	}

	// Crop the scene texture so width and height are evenly divisible by 8.
	// This cropped version of the scene will be used for post processing effects,
	// and keeping everything evenly divisible allows precise control over
	// sampling points within the shaders.
	m_dwCropWidth = pBackBufferDesc->Width - pBackBufferDesc->Width % 8;
	m_dwCropHeight = pBackBufferDesc->Height - pBackBufferDesc->Height % 8;

	// Create the HDR scene texture
	hr = pD3DDevice->CreateTexture (pBackBufferDesc->Width, pBackBufferDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pTexScene, NULL);
	if (FAILED (hr) )
		return hr;

	// Scaled version of the HDR scene texture
	hr = pD3DDevice->CreateTexture (m_dwCropWidth / 4, m_dwCropHeight / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pTexSceneScaled, NULL);
	if (FAILED (hr) )
		return hr;

	// Create the bright-pass filter texture. 
	// Texture has a black border of single texel thickness to fake border 
	// addressing using clamp addressing
	hr = pD3DDevice->CreateTexture (m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexBrightPass, NULL);
	if (FAILED (hr) )
		return hr;

	// Create a texture to be used as the source for the star effect
	// Texture has a black border of single texel thickness to fake border 
	// addressing using clamp addressing
	hr = pD3DDevice->CreateTexture (m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexStarSource, NULL);
	if (FAILED (hr) )
		return hr;

	// Create a texture to be used as the source for the bloom effect
	// Texture has a black border of single texel thickness to fake border 
	// addressing using clamp addressing
	hr = pD3DDevice->CreateTexture (m_dwCropWidth / 8 + 2, m_dwCropHeight / 8 + 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexBloomSource, NULL);
	if (FAILED (hr) )
		return hr;

	// Create a 2 textures to hold the luminance that the user is currently adapted
	// to. This allows for a simple simulation of light adaptation.
	hr = pD3DDevice->CreateTexture (1, 1, 1, D3DUSAGE_RENDERTARGET, m_LuminanceFormat, D3DPOOL_DEFAULT, &m_pTexAdaptedLuminanceCur, NULL);
	if (FAILED (hr) )
		return hr;
	hr = pD3DDevice->CreateTexture (1, 1, 1, D3DUSAGE_RENDERTARGET, m_LuminanceFormat, D3DPOOL_DEFAULT, &m_pTexAdaptedLuminanceLast, NULL);
	if (FAILED (hr) )
		return hr;

	// For each scale stage, create a texture to hold the intermediate results
	// of the luminance calculation
	for (i = 0; i < NUM_TONEMAP_TEXTURES; ++i)
	{
		int iSampleLen = 1 << (2 * i);

		hr = pD3DDevice->CreateTexture (iSampleLen, iSampleLen, 1, D3DUSAGE_RENDERTARGET, m_LuminanceFormat, D3DPOOL_DEFAULT, &m_apTexToneMap [i], NULL);
		if (FAILED (hr) )
			return hr;
	}

	// Create the temporary blooming effect textures
	// Texture has a black border of single texel thickness to fake border 
	// addressing using clamp addressing
	for (i = 1; i < NUM_BLOOM_TEXTURES; ++i)
	{
		hr = pD3DDevice->CreateTexture (m_dwCropWidth / 8 + 2, m_dwCropHeight / 8 + 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_apTexBloom [i], NULL);
		if (FAILED (hr) )
			return hr;
	}

	// Create the final blooming effect texture
	hr = pD3DDevice->CreateTexture (m_dwCropWidth / 8, m_dwCropHeight / 8, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_apTexBloom [0], NULL);
	if (FAILED (hr) )
		return hr;

	// Create the star effect textures
	for (i = 0; i < NUM_STAR_TEXTURES; ++i)
	{
		hr = pD3DDevice->CreateTexture (m_dwCropWidth / 4, m_dwCropHeight / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_apTexStar [i], NULL);
		if (FAILED (hr) )
			return hr;
	}

	// Create a texture to paint the walls
	//TCHAR Path [MAX_PATH];
	//DXUTFindDXSDKMediaFileCch (Path, MAX_PATH, TEXT ("misc\\env2.bmp") );
	//hr = D3DXCreateTextureFromFile (pD3DDevice, Path, &m_pTexWall);
	//if (FAILED (hr) )
	//	return hr;

	// Create a texture to paint the floor
	//DXUTFindDXSDKMediaFileCch (Path, MAX_PATH, TEXT ("misc\\ground2.bmp") );
	//hr = D3DXCreateTextureFromFile (m_pD3DDevice, Path, &m_pTexFloor);
	//if (FAILED (hr) )
	//	return hr;

	// Create a texture to paint the ceiling
	//DXUTFindDXSDKMediaFileCch (Path, MAX_PATH, TEXT ("misc\\seafloor.bmp") );
	//hr = D3DXCreateTextureFromFile (m_pD3DDevice, Path, &m_pTexCeiling);
	//if (FAILED (hr) )
	//	return hr;


	// Create a texture for the paintings
	//DXUTFindDXSDKMediaFileCch (Path, MAX_PATH, TEXT ("misc\\env3.bmp") );
	//hr = D3DXCreateTextureFromFile (m_pD3DDevice, Path, &m_pTexPainting);
	//if (FAILED (hr) )
	//	return hr;

	// Textures with borders must be cleared since scissor rect testing will
	// be used to avoid rendering on top of the border
	ClearTexture (pD3DDevice, m_pTexAdaptedLuminanceCur);
	ClearTexture (pD3DDevice, m_pTexAdaptedLuminanceLast);
	ClearTexture (pD3DDevice, m_pTexBloomSource);
	ClearTexture (pD3DDevice, m_pTexBrightPass);
	ClearTexture (pD3DDevice, m_pTexStarSource);

	for (i = 0; i < NUM_BLOOM_TEXTURES; ++i)
		ClearTexture (pD3DDevice, m_apTexBloom [i]);

	// Build the world object
	//hr = BuildWorldMesh ();
	//if (FAILED (hr) )
	//	return hr;

	// Create sphere mesh to represent the light
	hr = LoadMesh (pD3DDevice, TEXT("./Data/sphere0.x"), &m_pmeshSphere);
	if (FAILED (hr) )
		return hr;

	// Setup the camera's projection parameters
	//float fAspectRatio =  ((FLOAT)m_dwCropWidth) / m_dwCropHeight;
	//g_Camera.SetProjParams (D3DX_PI / 4, fAspectRatio, 0.2f, 1000.0f);
	//D3DXMATRIX mProjection = *g_Camera.GetProjMatrix ();

	// Set effect file variables
	m_pEffect->SetMatrix ("g_mProjection", ACAMERA->GetProjectionMatrix () );
	m_pEffect->SetFloat ("g_fBloomScale", 1.0f);
	m_pEffect->SetFloat ("g_fStarScale", 0.5f);

	return S_OK;
}

/**
	@brief			: ClearTexture
	@description	: Helper function for RestoreDeviceObjects to clear a texture surface
*/
HRESULT CAHDRLighting::ClearTexture (LPDIRECT3DDEVICE9 pD3DDevice, LPDIRECT3DTEXTURE9 pTexture)
{
	HRESULT hr = S_OK;
	PDIRECT3DSURFACE9 pSurface = NULL;

	hr = pTexture->GetSurfaceLevel (0, &pSurface);
	if (SUCCEEDED(hr) )
		pD3DDevice->ColorFill (pSurface, NULL, D3DCOLOR_ARGB (0, 0, 0, 0) );

	SAFE_RELEASE (pSurface);
	return hr;
}

/**
	@brief			: BuildWorldMesh
	@description	: Creates the wall, floor, ceiling, columns, and painting mesh
*/
//HRESULT CAHDRLighting::BuildWorldMesh ()
//{
//	HRESULT hr;
//	UINT i;
//
//	const FLOAT fWidth = 15.0f;
//	const FLOAT fDepth = 20.0f;
//	const FLOAT fHeight = 3.0f;
//
//	// Create the room  
//	LPD3DXMESH pWorldMeshTemp = NULL;
//	hr = D3DXCreateMeshFVF (48, 96, 0, WorldVertex::FVF, m_pD3DDevice, &pWorldMeshTemp);
//	if (FAILED (hr) )
//		goto LCleanReturn;
//
//	WorldVertex* pVertex;
//	hr = pWorldMeshTemp->LockVertexBuffer (0,  (PVOID*)&pVertex);
//	if (FAILED (hr) )
//		goto LCleanReturn;
//
//	WorldVertex* pV;
//	pV = pVertex;
//
//	// Front wall
//	SetTextureCoords (pV, 7.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, fDepth);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, fDepth);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, fDepth);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, fDepth);
//
//	// Right wall
//	SetTextureCoords (pV, 10.5f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, fDepth);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, fDepth);
//
//	// Back wall
//	SetTextureCoords (pV, 7.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, 0.0f);
//
//	// Left wall
//	SetTextureCoords (pV, 10.5f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, fDepth);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, fDepth);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);
//
//	BuildColumn (pV, 4.0f, fHeight, 7.0f, 0.75f);
//	BuildColumn (pV, 4.0f, fHeight, 13.0f, 0.75f);
//	BuildColumn (pV, 11.0f, fHeight, 7.0f, 0.75f);
//	BuildColumn (pV, 11.0f, fHeight, 13.0f, 0.75f);
//
//	// Floor
//	SetTextureCoords (pV, 7.0f, 7.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, fDepth);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, fDepth);
//	(pV++)->p = D3DXVECTOR3 (fWidth, 0.0f, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, 0.0f, 0.0f);
//
//	// Ceiling
//	SetTextureCoords (pV, 7.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, 0.0f);
//	(pV++)->p = D3DXVECTOR3 (fWidth, fHeight, fDepth);
//	(pV++)->p = D3DXVECTOR3 (0.0f, fHeight, fDepth);
//
//	// Painting 1
//	SetTextureCoords (pV, 1.0f, 1.0f);
//	(pV++)->p = D3DXVECTOR3 (2.0f, fHeight - 0.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (6.0f, fHeight - 0.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (6.0f, fHeight - 2.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (2.0f, fHeight - 2.5f, fDepth - 0.01f);
//
//	// Painting 2
//	SetTextureCoords (pV, 1.0f, 1.0f);
//	(pV++)->p = D3DXVECTOR3 (9.0f, fHeight - 0.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (13.0f, fHeight - 0.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (13.0f, fHeight - 2.5f, fDepth - 0.01f);
//	(pV++)->p = D3DXVECTOR3 (9.0f, fHeight - 2.5f, fDepth - 0.01f);
//
//	pWorldMeshTemp->UnlockVertexBuffer();
//
//	// Retrieve the indices
//	WORD* pIndex;
//	hr = pWorldMeshTemp->LockIndexBuffer (0,  (PVOID*)&pIndex);
//	if (FAILED (hr) )
//		goto LCleanReturn;
//
//	for (i = 0; i < pWorldMeshTemp->GetNumFaces() / 2; i++)
//	{
//		*pIndex++ = (WORD) ( (i * 4) + 0);
//		*pIndex++ = (WORD) ( (i * 4) + 1);
//		*pIndex++ = (WORD) ( (i * 4) + 2);
//		*pIndex++ = (WORD) ( (i * 4) + 0);
//		*pIndex++ = (WORD) ( (i * 4) + 2);
//		*pIndex++ = (WORD) ( (i * 4) + 3);
//	}
//
//	pWorldMeshTemp->UnlockIndexBuffer();
//
//	// Set attribute groups to draw floor, ceiling, walls, and paintings
//	// separately, with different shader constants. These group numbers
//	// will be used during the calls to DrawSubset().
//	DWORD* pAttribs;
//	hr = pWorldMeshTemp->LockAttributeBuffer (0, &pAttribs);
//	if (FAILED (hr) )
//		goto LCleanReturn;
//
//	for (i = 0; i < 40; i++)
//		*pAttribs++ = 0;
//
//	for (i = 0; i < 2; i++)
//		*pAttribs++ = 1;
//
//	for (i = 0; i < 2; i++)
//		*pAttribs++ = 2;
//
//	for (i = 0; i < 4; i++)
//		*pAttribs++ = 3;
//
//	pWorldMeshTemp->UnlockAttributeBuffer();
//	D3DXComputeNormals (pWorldMeshTemp, NULL);
//
//	// Optimize the mesh
//	hr = pWorldMeshTemp->CloneMeshFVF (D3DXMESH_VB_WRITEONLY | D3DXMESH_IB_WRITEONLY,
//		WorldVertex::FVF, m_pD3DDevice, &m_pWorldMesh);
//	if (FAILED (hr) )
//		goto LCleanReturn;
//
//	hr = S_OK;
//
//LCleanReturn:
//	SAFE_RELEASE (pWorldMeshTemp);
//	return hr;
//}

//-----------------------------------------------------------------------------
// Name: BuildColumn()
// Desc: Helper function for BuildWorldMesh to add column quads to the scene 
//-----------------------------------------------------------------------------
//HRESULT CAHDRLighting::BuildColumn (WorldVertex*& pV, float x, float y, float z, float width)
//{
//	float w = width / 2;
//
//	SetTextureCoords (pV, 1.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (x - w, y, z - w);
//	(pV++)->p = D3DXVECTOR3 (x + w, y, z - w);
//	(pV++)->p = D3DXVECTOR3 (x + w, 0.0f, z - w);
//	(pV++)->p = D3DXVECTOR3 (x - w, 0.0f, z - w);
//
//	SetTextureCoords (pV, 1.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (x + w, y, z - w);
//	(pV++)->p = D3DXVECTOR3 (x + w, y, z + w);
//	(pV++)->p = D3DXVECTOR3 (x + w, 0.0f, z + w);
//	(pV++)->p = D3DXVECTOR3 (x + w, 0.0f, z - w);
//
//	SetTextureCoords (pV, 1.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (x + w, y, z + w);
//	(pV++)->p = D3DXVECTOR3 (x - w, y, z + w);
//	(pV++)->p = D3DXVECTOR3 (x - w, 0.0f, z + w);
//	(pV++)->p = D3DXVECTOR3 (x + w, 0.0f, z + w);
//
//	SetTextureCoords (pV, 1.0f, 2.0f);
//	(pV++)->p = D3DXVECTOR3 (x - w, y, z + w);
//	(pV++)->p = D3DXVECTOR3 (x - w, y, z - w);
//	(pV++)->p = D3DXVECTOR3 (x - w, 0.0f, z - w);
//	(pV++)->p = D3DXVECTOR3 (x - w, 0.0f, z + w);
//
//	return S_OK;
//}

//-----------------------------------------------------------------------------
// Name: SetTextureCoords()
// Desc: Helper function for BuildWorldMesh to set texture coordinates
//       for vertices
//-----------------------------------------------------------------------------
//void CAHDRLighting::SetTextureCoords (WorldVertex* pVertex, float u, float v)
//{
//	(pVertex++)->t = D3DXVECTOR2 (0.0f, 0.0f);
//	(pVertex++)->t = D3DXVECTOR2 (u, 0.0f);
//	(pVertex++)->t = D3DXVECTOR2 (u, v);
//	(pVertex++)->t = D3DXVECTOR2 (0.0f, v);
//}

void CAHDRLighting::Update (float fElapsedTime)
{
	// Set the flag to refresh the user's simulated adaption level.
	// Frame move is not called when the scene is paused or single-stepped. 
	// If the scene is paused, the user's adaptation level needs to remain
	// unchanged.
	fRElapsedTime = fElapsedTime;

	m_bAdaptationInvalid = true;

	// Calculate the position of the lights in view space
	D3DXVECTOR4 avLightViewPosition [MAX_LIGHTS];
	for (UINT iLight = 0; iLight < NUM_LIGHTS; ++iLight)
	{
		D3DXMATRIX mView = *ACAMERA->GetViewMatrix ();
		D3DXVec4Transform (&avLightViewPosition [iLight], &m_avLightPosition [iLight], &mView);

		printf ("%d Position : %lf %lf %lf %lf\n", iLight, avLightViewPosition [iLight].x, avLightViewPosition [iLight].y, avLightViewPosition [iLight].z, avLightViewPosition [iLight].w);
	}
	
	// Set frame shader constants
	m_pEffect->SetInt ("g_nNumLights", NUM_LIGHTS);
	m_pEffect->SetBool  ("g_bEnableToneMap", m_bToneMap);
	m_pEffect->SetValue  ("g_avLightPositionView", avLightViewPosition, sizeof (D3DXVECTOR4) * MAX_LIGHTS);
	m_pEffect->SetValue  ("g_avLightIntensity", m_avLightIntensity, sizeof (D3DXVECTOR4) * MAX_LIGHTS);
}

bool CAHDRLighting::PreRender (LPDIRECT3DDEVICE9 pD3DDevice)
{
	// Store the old render target
	pD3DDevice->GetRenderTarget (0, &pRSurfLDR);
	pD3DDevice->GetDepthStencilSurface (&pRSurfDS);

	// Setup HDR render target
	m_pTexScene->GetSurfaceLevel (0, &pRSurfHDR);
	if (m_bUseMultiSampleFloat16)
	{
		pD3DDevice->SetRenderTarget (0, m_pFloatMSRT);
		pD3DDevice->SetDepthStencilSurface (m_pFloatMSDS);
	}
	else
		pD3DDevice->SetRenderTarget (0, pRSurfHDR);

	// Clear the viewport
	//m_pD3DDevice->Clear (0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA (0, 0, 0, 0), 1.0f, 0L);

	// Render the scene
	if (SUCCEEDED (pD3DDevice->BeginScene () ))
	// Render the HDR Scene
	{
		//CDXUTPerfEventGenerator g (DXUT_PERFEVENTCOLOR, L"Scene");
		return true;
	}
	else
		return false;
}

void CAHDRLighting::PostRender (LPDIRECT3DDEVICE9 pD3DDevice)
{
	{
		// Render the HDR Scene
		{
			RenderScene (pD3DDevice);
		}

		// If using floating point multi sampling, stretchrect to the rendertarget
		if (m_bUseMultiSampleFloat16)
		{
			pD3DDevice->StretchRect (m_pFloatMSRT, NULL, pRSurfHDR, NULL, D3DTEXF_NONE);
			pD3DDevice->SetRenderTarget (0, pRSurfHDR);
			pD3DDevice->SetDepthStencilSurface (pRSurfDS);
		}

		// Create a scaled copy of the scene
		Scene_To_SceneScaled (pD3DDevice);

		// Setup tone mapping technique
		if (m_bToneMap)
			MeasureLuminance (pD3DDevice);

		// If FrameMove has been called, the user's adaptation level has also changed
		// and should be updated
		if (m_bAdaptationInvalid)
		{
			// Clear the update flag
			m_bAdaptationInvalid = false;

			// Calculate the current luminance adaptation level
			CalculateAdaptation (pD3DDevice, fRElapsedTime);
		}

		// Now that luminance information has been gathered, the scene can be bright-pass filtered
		// to remove everything except bright lights and reflections.
		SceneScaled_To_BrightPass (pD3DDevice);

		// Blur the bright-pass filtered image to create the source texture for the star effect
		BrightPass_To_StarSource (pD3DDevice);

		// Scale-down the source texture for the star effect to create the source texture
		// for the bloom effect
		StarSource_To_BloomSource (pD3DDevice);

		// Render post-process lighting effects
		{
			//CDXUTPerfEventGenerator g(DXUT_PERFEVENTCOLOR, L"Bloom");
			RenderBloom (pD3DDevice);
		}
		{
			//CDXUTPerfEventGenerator g(DXUT_PERFEVENTCOLOR, L"Star");
			RenderStar (pD3DDevice);
		}

		// Draw the high dynamic range scene texture to the low dynamic range
		// back buffer. As part of this final pass, the scene will be tone-mapped
		// using the user's current adapted luminance, blue shift will occur
		// if the scene is determined to be very dark, and the post-process lighting
		// effect textures will be added to the scene.
		UINT uiPassCount, uiPass;

		m_pEffect->SetTechnique ("FinalScenePass");
		m_pEffect->SetFloat ("g_fMiddleGray", m_fKeyValue);

		pD3DDevice->SetRenderTarget (0, pRSurfLDR);
		pD3DDevice->SetTexture (0, m_pTexScene);
		pD3DDevice->SetTexture (1, m_apTexBloom [0]);
		pD3DDevice->SetTexture (2, m_apTexStar [0]);
		pD3DDevice->SetTexture (3, m_pTexAdaptedLuminanceCur);
		pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState (1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState (1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState (2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState (2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState (3, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState (3, D3DSAMP_MINFILTER, D3DTEXF_POINT);

		m_pEffect->Begin (&uiPassCount, 0);
		{
			//CDXUTPerfEventGenerator g(DXUT_PERFEVENTCOLOR, L"Final Scene Pass");
			for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
			{
				m_pEffect->BeginPass (uiPass);

				DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

				m_pEffect->EndPass ();
			}
		}
		m_pEffect->End ();

		pD3DDevice->SetTexture (1, NULL);
		pD3DDevice->SetTexture (2, NULL);
		pD3DDevice->SetTexture (3, NULL);

		pD3DDevice->EndScene ();
	}

	// Release surfaces
	SAFE_RELEASE (pRSurfHDR);
	SAFE_RELEASE (pRSurfLDR);
	SAFE_RELEASE (pRSurfDS);
}

/**
	@brief			: RenderScene
	@description	: Render the world objects and lights
*/
HRESULT CAHDRLighting::RenderScene (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;

	UINT uiPassCount, uiPass;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mTrans;
	D3DXMATRIXA16 mRotate;
	D3DXMATRIXA16 mObjectToView;

	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	D3DXMATRIX mView = *ACAMERA->GetViewMatrix ();

	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity (&matWorld);
	pD3DDevice->SetTransform (D3DTS_WORLD, &matWorld);
	pD3DDevice->SetTransform (D3DTS_PROJECTION, ACAMERA->GetProjectionMatrix () );
	pD3DDevice->SetTransform (D3DTS_VIEW, ACAMERA->GetViewMatrix () );

	m_pEffect->SetTechnique ("RenderScene");
	m_pEffect->SetMatrix ("g_mObjectToView", &mView);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		return hr;

	APROJECT_WINDOW->GetCurScene ()->Render (pD3DDevice, false);

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Turn off emissive lighting
        D3DXVECTOR4 vNull (0.0f, 0.0f, 0.0f, 0.0f);
        m_pEffect->SetVector ("g_vEmissive", &vNull);

		// Enable texture
        m_pEffect->SetBool ("g_bEnableTexture", true);
        pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		// Draw the light spheres.
		m_pEffect->SetFloat ("g_fPhongExponent", 5.0f);
		m_pEffect->SetFloat ("g_fPhongCoefficient", 1.0f);
		m_pEffect->SetFloat ("g_fDiffuseCoefficient", 1.0f);
		m_pEffect->SetBool ("g_bEnableTexture", false);

		for (UINT iLight = 0; iLight < NUM_LIGHTS; ++iLight)
		{
			// Just position the point light -- no need to orient it
			D3DXMATRIXA16 mScale;
			D3DXMatrixScaling (&mScale, 0.45f, 0.45f, 0.45f);

			mView = *ACAMERA->GetViewMatrix ();
			D3DXMatrixTranslation (&mWorld, m_avLightPosition [iLight].x, m_avLightPosition [iLight].y, m_avLightPosition [iLight].z);
			mWorld = mScale * mWorld;
			mObjectToView = mWorld * mView;
			pD3DDevice->SetTransform (D3DTS_WORLD, &mWorld);
			m_pEffect->SetMatrix ("g_mObjectToView", &mObjectToView);

			// A light which illuminates objects at 80 lum/sr should be drawn
			// at 3183 lumens/meter^2/steradian, which equates to a multiplier
			// of 39.78 per lumen.
			D3DXVECTOR4 vEmissive = EMISSIVE_COEFFICIENT * m_avLightIntensity [iLight];
			m_pEffect->SetVector ("g_vEmissive", &vEmissive);

			m_pEffect->CommitChanges ();
			m_pmeshSphere->DrawSubset (0);
		}
		m_pEffect->EndPass ();
	}

	m_pEffect->End ();

	return S_OK;
}

/**
	@brief			: MeasureLuminance
	@description	: Measure the average log luminance in the scene.
*/
HRESULT CAHDRLighting::MeasureLuminance (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;
	UINT uiPassCount, uiPass;
	int i, x, y, index;
	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];

	DWORD dwCurTexture = NUM_TONEMAP_TEXTURES - 1;

	// Sample log average luminance
	PDIRECT3DSURFACE9 apSurfToneMap [NUM_TONEMAP_TEXTURES] = {0};

	// Retrieve the tonemap surfaces
	for (i = 0; i < NUM_TONEMAP_TEXTURES; ++i)
	{
		hr = m_apTexToneMap [i]->GetSurfaceLevel (0, &apSurfToneMap [i]);
		if (FAILED (hr) )
			goto LCleanReturn;
	}

	D3DSURFACE_DESC desc;
	m_apTexToneMap [dwCurTexture]->GetLevelDesc (0, &desc);

	// Initialize the sample offsets for the initial luminance pass.
	float tU, tV;
	tU = 1.0f / (3.0f * desc.Width);
	tV = 1.0f / (3.0f * desc.Height);

	index = 0;
	for (x = -1; x <= 1; ++x)
	{
		for (y = -1; y <= 1; ++y)
		{
			avSampleOffsets [index].x = x * tU;
			avSampleOffsets [index].y = y * tV;

			index++;
		}
	}

	// After this pass, the g_apTexToneMap[NUM_TONEMAP_TEXTURES-1] texture will contain
	// a scaled, grayscale copy of the HDR scene. Individual texels contain the log 
	// of average luminance values for points sampled on the HDR texture.
	m_pEffect->SetTechnique ("SampleAvgLum");
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );

	pD3DDevice->SetRenderTarget (0, apSurfToneMap [dwCurTexture]);
	pD3DDevice->SetTexture (0, m_pTexSceneScaled);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();
	dwCurTexture --;

	// Initialize the sample offsets for the iterative luminance passes
	while (dwCurTexture > 0)
	{
		m_apTexToneMap [dwCurTexture + 1]->GetLevelDesc (0, &desc);
		GetSampleOffsets_DownScale4x4 (desc.Width, desc.Height, avSampleOffsets);

		// Each of these passes continue to scale down the log of average
		// luminance texture created above, storing intermediate results in 
		// g_apTexToneMap[1] through g_apTexToneMap[NUM_TONEMAP_TEXTURES-1].
		m_pEffect->SetTechnique ("ResampleAvgLum");
		m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );

		pD3DDevice->SetRenderTarget (0, apSurfToneMap [dwCurTexture]);
		pD3DDevice->SetTexture (0, m_apTexToneMap [dwCurTexture + 1]);
		pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

		hr = m_pEffect->Begin (&uiPassCount, 0);
		if (FAILED (hr) )
			goto LCleanReturn;

		for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
		{
			m_pEffect->BeginPass (uiPass);

			// Draw a fullscreen quad to sample the RT
			DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

			m_pEffect->EndPass ();
		}
		m_pEffect->End ();
		dwCurTexture --;
	}

	// Downsample to 1x1
	m_apTexToneMap [1]->GetLevelDesc (0, &desc);
	GetSampleOffsets_DownScale4x4 (desc.Width, desc.Height, avSampleOffsets);

	// Perform the final pass of the average luminance calculation. This pass
	// scales the 4x4 log of average luminance texture from above and performs
	// an exp() operation to return a single texel cooresponding to the average
	// luminance of the scene in g_apTexToneMap[0].
	m_pEffect->SetTechnique ("ResampleAvgLumExp");
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );

	pD3DDevice->SetRenderTarget (0, apSurfToneMap [0]);
	pD3DDevice->SetTexture (0, m_apTexToneMap [1]);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();

	hr = S_OK;

LCleanReturn:
	for (i = 0; i < NUM_TONEMAP_TEXTURES; ++i)
		SAFE_RELEASE (apSurfToneMap [i]);

	return hr;
}

/**
	@brief			: CalculateAdaptation
	@description	: Increment the user's adapted luminance
*/
HRESULT CAHDRLighting::CalculateAdaptation (LPDIRECT3DDEVICE9 pD3DDevice, float fElapsedTime)
{
	UINT uiPass, uiPassCount;

	// Swap current & last luminance
	PDIRECT3DTEXTURE9 pTexSwap = m_pTexAdaptedLuminanceLast;
	m_pTexAdaptedLuminanceLast = m_pTexAdaptedLuminanceCur;
	m_pTexAdaptedLuminanceCur = pTexSwap;

	PDIRECT3DSURFACE9 pSurfAdaptedLum = NULL;
	m_pTexAdaptedLuminanceCur->GetSurfaceLevel (0, &pSurfAdaptedLum);

	// This simulates the light adaptation that occurs when moving from a 
	// dark area to a bright area, or vice versa. The g_pTexAdaptedLuminance
	// texture stores a single texel cooresponding to the user's adapted 
	// level.
	m_pEffect->SetTechnique ("CalculateAdaptedLum");
	m_pEffect->SetFloat ("g_fElapsedTime", fElapsedTime);

	pD3DDevice->SetRenderTarget (0, pSurfAdaptedLum);
	pD3DDevice->SetTexture (0, m_pTexAdaptedLuminanceLast);
	pD3DDevice->SetTexture (1, m_apTexToneMap [0]);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	m_pEffect->Begin (&uiPassCount, 0);
	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();

	SAFE_RELEASE (pSurfAdaptedLum);

	return S_OK;
}

/**
	@brief			: RenderStar
	@description	: Render the blooming effect
*/
HRESULT CAHDRLighting::RenderStar (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;
	UINT uiPassCount, uiPass;
	int i, d, p, s; // Loop variables

	ClearTexture (pD3DDevice, m_apTexStar [0]);

	// Avoid rendering the star if it's not being used in the current glare
	if (m_GlareDef.m_fGlareLuminance <= 0.0f || m_GlareDef.m_fStarLuminance <= 0.0f)
		return S_OK;

	// Initialize the constants used during the effect
	const CAStarDefine& starDef = m_GlareDef.m_starDef;
	const float fTanFoV = atanf (D3DX_PI / 8);
	const D3DXVECTOR4 vWhite (1.0f, 1.0f, 1.0f, 1.0f);
	static const int s_maxPasses = 3;
	static const int nSamples = 8;
	static D3DXVECTOR4 s_aaColor [s_maxPasses][8];
	static const D3DXCOLOR s_colorWhite (0.63f, 0.63f, 0.63f, 0.0f);

	D3DXVECTOR4 avSampleWeights [MAX_SAMPLES];
	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];

	pD3DDevice->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
	pD3DDevice->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ONE);

	LPDIRECT3DSURFACE9 pSurfSource = NULL;
	LPDIRECT3DSURFACE9 pSurfDest = NULL;

	// Set aside all the star texture surfaces as a convenience
	LPDIRECT3DSURFACE9 apSurfStar [NUM_STAR_TEXTURES] = {0};
	for (i = 0; i < NUM_STAR_TEXTURES; ++i)
	{
		hr = m_apTexStar [i]->GetSurfaceLevel (0, &apSurfStar [i]);
		if (FAILED (hr) )
			goto LCleanReturn;
	}

	// Get the source texture dimensions
	hr = m_pTexStarSource->GetSurfaceLevel (0, &pSurfSource);
	if (FAILED (hr) )
		goto LCleanReturn;

	D3DSURFACE_DESC desc;
	hr = pSurfSource->GetDesc (&desc);
	if (FAILED (hr) )
		goto LCleanReturn;

	SAFE_RELEASE (pSurfSource);

	float srcW;
	srcW = (FLOAT)desc.Width;
	float srcH;
	srcH = (FLOAT)desc.Height;

	for (p = 0; p < s_maxPasses; ++p)
	{
		float ratio;
		ratio = (float)(p + 1) / (float)s_maxPasses;

		for (s = 0; s < nSamples; ++s)
		{
			D3DXCOLOR chromaticAberrColor;
			D3DXColorLerp (&chromaticAberrColor, &(CAStarDefine::GetChromaticAberrationColor (s) ), &s_colorWhite, ratio);
			D3DXColorLerp  ((D3DXCOLOR*)&(s_aaColor [p][s]), &s_colorWhite, &chromaticAberrColor, m_GlareDef.m_fChromaticAberration);
		}
	}

	float radOffset;
	radOffset = m_GlareDef.m_fStarInclination + starDef.m_fInclination;

	LPDIRECT3DTEXTURE9 pTexSource;

	// Direction loop
	for (d = 0; d < starDef.m_nStarLines; ++d)
	{
		CONST STARLINE& starLine = starDef.m_pStarLine [d];

		pTexSource = m_pTexStarSource;

		float rad;
		rad = radOffset + starLine.fInclination;
		float sn, cs;
		sn = sinf (rad), cs = cosf (rad);
		D3DXVECTOR2 vtStepUV;
		vtStepUV.x = sn / srcW * starLine.fSampleLength;
		vtStepUV.y = cs / srcH * starLine.fSampleLength;

		float attnPowScale;
		attnPowScale = (fTanFoV + 0.1f) * 1.0f * (160.0f + 120.0f) / (srcW + srcH) * 1.2f;

		// 1 direction expansion loop
		pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);

		int iWorkTexture;
		iWorkTexture = 1;
		for (p = 0; p < starLine.nPasses; ++p)
		{
			if (p == starLine.nPasses - 1)
				// Last pass move to other work buffer
				pSurfDest = apSurfStar [d + 4];
			else
				pSurfDest = apSurfStar [iWorkTexture];

			// Sampling configration for each stage
			for (i = 0; i < nSamples; ++i)
			{
				float lum;
				lum = powf (starLine.fAttenuation, attnPowScale * i);

				avSampleWeights [i] = s_aaColor [starLine.nPasses - 1 - p][i] * lum * (p + 1.0f) * 0.5f;

				// Offset of sampling coordinate
				avSampleOffsets [i].x = vtStepUV.x * i;
				avSampleOffsets [i].y = vtStepUV.y * i;
				if (fabs (avSampleOffsets [i].x) >= 0.9f || fabs (avSampleOffsets [i].y) >= 0.9f)
				{
					avSampleOffsets [i].x = 0.0f;
					avSampleOffsets [i].y = 0.0f;
					avSampleWeights [i] *= 0.0f;
				}
			}

			m_pEffect->SetTechnique ("Star");
			m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );
			m_pEffect->SetVectorArray ("g_avSampleWeights", avSampleWeights, nSamples);

			pD3DDevice->SetRenderTarget (0, pSurfDest);
			pD3DDevice->SetTexture (0, pTexSource);
			pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

			hr = m_pEffect->Begin (&uiPassCount, 0);
			if (FAILED (hr) )
				return hr;

			for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
			{
				m_pEffect->BeginPass (uiPass);

				// Draw a fullscreen quad to sample the RT
				DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

				m_pEffect->EndPass ();
			}
			m_pEffect->End ();

			// Setup next expansion
			vtStepUV *= nSamples;
			attnPowScale *= nSamples;

			// Set the work drawn just before to next texture source.
			pTexSource = m_apTexStar [iWorkTexture];

			iWorkTexture += 1;
			if (iWorkTexture > 2)
				iWorkTexture = 1;
		}
	}

	pSurfDest = apSurfStar [0];

	for (i = 0; i < starDef.m_nStarLines; ++i)
	{
		pD3DDevice->SetTexture (i, m_apTexStar [i + 4]);
		pD3DDevice->SetSamplerState (i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState (i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		avSampleWeights [i] = vWhite * 1.0f / (FLOAT)starDef.m_nStarLines;
	}

	CHAR strTechnique [256];
	sprintf_s (strTechnique, 256, "MergeTextures_%d", starDef.m_nStarLines);

	m_pEffect->SetTechnique (strTechnique);

	m_pEffect->SetVectorArray ("g_avSampleWeights", avSampleWeights, starDef.m_nStarLines);

	pD3DDevice->SetRenderTarget (0, pSurfDest);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, 0.0f, 0.0f, 1.0f, 1.0f);

		m_pEffect->EndPass ();
	}
	m_pEffect->End ();

	for (i = 0; i < starDef.m_nStarLines; ++i)
		pD3DDevice->SetTexture (i, NULL);

	hr = S_OK;

LCleanReturn:
	for (i = 0; i < NUM_STAR_TEXTURES; ++i)
		SAFE_RELEASE (apSurfStar [i]);

	return hr;
}

/**
	@brief			: RenderBloom
	@description	: Render the blooming effect
*/
HRESULT CAHDRLighting::RenderBloom (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;
	UINT uiPassCount, uiPass;
	int i = 0;

	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];
	FLOAT afSampleOffsets [MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights [MAX_SAMPLES];

	LPDIRECT3DSURFACE9 pSurfScaledHDR;
	m_pTexSceneScaled->GetSurfaceLevel (0, &pSurfScaledHDR);

	LPDIRECT3DSURFACE9 pSurfBloom;
	m_apTexBloom [0]->GetSurfaceLevel (0, &pSurfBloom);

	LPDIRECT3DSURFACE9 pSurfHDR;
	m_pTexScene->GetSurfaceLevel (0, &pSurfHDR);

	LPDIRECT3DSURFACE9 pSurfTempBloom;
	m_apTexBloom [1]->GetSurfaceLevel (0, &pSurfTempBloom);

	PDIRECT3DSURFACE9 pSurfBloomSource;
	m_apTexBloom [2]->GetSurfaceLevel (0, &pSurfBloomSource);

	// Clear the bloom texture
	pD3DDevice->ColorFill (pSurfBloom, NULL, D3DCOLOR_ARGB (0, 0, 0, 0) );

	if (m_GlareDef.m_fGlareLuminance <= 0.0f || m_GlareDef.m_fBloomLuminance <= 0.0f)
	{
		hr = S_OK;
		goto LCleanReturn;
	}

	RECT rectSrc;
	GetTextureRect (m_pTexBloomSource, &rectSrc);
	InflateRect (&rectSrc, -1, -1);

	RECT rectDest;
	GetTextureRect (m_apTexBloom [2], &rectDest);
	InflateRect (&rectDest, -1, -1);

	CoordRect coords;
	GetTextureCoords (m_pTexBloomSource, &rectSrc, m_apTexBloom [2], &rectDest, &coords);

	D3DSURFACE_DESC desc;
	hr = m_pTexBloomSource->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	m_pEffect->SetTechnique ("GaussBlur5x5");

	hr = GetSampleOffsets_GaussBlur5x5 (desc.Width, desc.Height, avSampleOffsets, avSampleWeights, 1.0f);

	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );
	m_pEffect->SetValue ("g_avSampleWeights", avSampleWeights, sizeof (avSampleWeights) );

	pD3DDevice->SetRenderTarget (0, pSurfBloomSource);
	pD3DDevice->SetTexture (0, m_pTexBloomSource);
	pD3DDevice->SetScissorRect (&rectDest);
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}
	m_pEffect->End ();
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, FALSE);

	hr = m_apTexBloom [2]->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	hr = GetSampleOffsets_Bloom (desc.Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
	for (i = 0; i < MAX_SAMPLES; ++i)
		avSampleOffsets [i] = D3DXVECTOR2 (afSampleOffsets [i], 0.0f);

	m_pEffect->SetTechnique ("Bloom");
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );
	m_pEffect->SetValue ("g_avSampleWeights", avSampleWeights, sizeof (avSampleWeights) );

	pD3DDevice->SetRenderTarget (0, pSurfTempBloom);
	pD3DDevice->SetTexture (0, m_apTexBloom [2]);
	pD3DDevice->SetScissorRect (&rectDest);
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	m_pEffect->Begin (&uiPassCount, 0);
	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}
	m_pEffect->End ();
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, FALSE);

	hr = m_apTexBloom [1]->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	hr = GetSampleOffsets_Bloom (desc.Height, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
	for (i = 0; i < MAX_SAMPLES; ++i)
		avSampleOffsets [i] = D3DXVECTOR2 (0.0f, afSampleOffsets [i]);

	GetTextureRect (m_apTexBloom [1], &rectSrc);
	InflateRect (&rectSrc, -1, -1);

	GetTextureCoords (m_apTexBloom [1], &rectSrc, m_apTexBloom [0], NULL, &coords);

	m_pEffect->SetTechnique ("Bloom");
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );
	m_pEffect->SetValue ("g_avSampleWeights", avSampleWeights, sizeof (avSampleWeights) );

	pD3DDevice->SetRenderTarget (0, pSurfBloom);
	pD3DDevice->SetTexture (0, m_apTexBloom [1]);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}
	m_pEffect->End ();

	hr = S_OK;

LCleanReturn:
	SAFE_RELEASE (pSurfBloomSource);
	SAFE_RELEASE (pSurfTempBloom);
	SAFE_RELEASE (pSurfBloom);
	SAFE_RELEASE (pSurfHDR);
	SAFE_RELEASE (pSurfScaledHDR);

	return hr;
}

/**
	@brief			: DrawFullScreenQuad
	@description	: Draw a properly aligned quad covering the entire render target
*/
void CAHDRLighting::DrawFullScreenQuad (LPDIRECT3DDEVICE9 pD3DDevice, float fLeftU, float fTopV, float fRightU, float fBottomV)
{
	D3DSURFACE_DESC dtdsdRT;
	PDIRECT3DSURFACE9 pSurfRT;

	// Acquire render target width and height
	pD3DDevice->GetRenderTarget (0, &pSurfRT);
	pSurfRT->GetDesc (&dtdsdRT);
	pSurfRT->Release ();

	// Ensure that we're directly mapping texels to pixels by offset by 0.5
	// For more info see the doc page titled "Directly Mapping Texels to Pixels"
	FLOAT fWidth5 = (FLOAT)dtdsdRT.Width - 0.5f;
	FLOAT fHeight5 = (FLOAT)dtdsdRT.Height - 0.5f;

	// Draw the quad
	ScreenVertex svQuad [4];

	svQuad [0].p = D3DXVECTOR4 (-0.5f, -0.5f, 0.5f, 1.0f);
	svQuad [0].t = D3DXVECTOR2 (fLeftU, fTopV);

	svQuad [1].p = D3DXVECTOR4 (fWidth5, -0.5f, 0.5f, 1.0f);
	svQuad [1].t = D3DXVECTOR2 (fRightU, fTopV);

	svQuad [2].p = D3DXVECTOR4 (-0.5f, fHeight5, 0.5f, 1.0f);
	svQuad [2].t = D3DXVECTOR2 (fLeftU, fBottomV);

	svQuad [3].p = D3DXVECTOR4 (fWidth5, fHeight5, 0.5f, 1.0f);
	svQuad [3].t = D3DXVECTOR2 (fRightU, fBottomV);

	pD3DDevice->SetRenderState (D3DRS_ZENABLE, FALSE);
	pD3DDevice->SetFVF (ScreenVertex::FVF);
	pD3DDevice->DrawPrimitiveUP (D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof (ScreenVertex) );
	pD3DDevice->SetRenderState (D3DRS_ZENABLE, TRUE);
}

/**
	@brief			: GetTextureRect
	@description	: Get the dimensions of the texture
*/
HRESULT CAHDRLighting::GetTextureRect (LPDIRECT3DTEXTURE9 pTexture, RECT* pRect)
{
	HRESULT hr = S_OK;

	if (pTexture == NULL || pRect == NULL)
		return E_INVALIDARG;

	D3DSURFACE_DESC desc;
	hr = pTexture->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	pRect->left = 0;
	pRect->top = 0;
	pRect->right = desc.Width;
	pRect->bottom = desc.Height;

	return S_OK;
}

/**
	@brief			: GetTextureCoords
	@description	: Get the texture coordinates to use when rendering into the destination
					  texture, given the source and destination rectangles
*/
HRESULT CAHDRLighting::GetTextureCoords (LPDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, LPDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords)
{
	HRESULT hr = S_OK;
	D3DSURFACE_DESC desc;
	float tU, tV;

	// Validate arguments
	if (pTexSrc == NULL || pTexDest == NULL || pCoords == NULL)
		return E_INVALIDARG;

	// Start with a default mapping of the complete source surface to complete 
	// destination surface
	pCoords->fLeftU = 0.0f;
	pCoords->fTopV = 0.0f;
	pCoords->fRightU = 1.0f;
	pCoords->fBottomV = 1.0f;

	// If not using the complete source surface, adjust the coordinates
	if (pRectSrc != NULL)
	{
		// Get destination texture description
		hr = pTexSrc->GetLevelDesc (0, &desc);
		if (FAILED (hr) )
			return hr;

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;

		pCoords->fLeftU += pRectSrc->left * tU;
		pCoords->fTopV += pRectSrc->top * tV;
		pCoords->fRightU -= (desc.Width - pRectSrc->right) * tU;
		pCoords->fBottomV -= (desc.Height - pRectSrc->bottom) * tV;
	}

	// If not drawing to the complete destination surface, adjust the coordinates
	if (pRectDest != NULL)
	{
		// Get source texture description
		hr = pTexDest->GetLevelDesc (0, &desc);
		if (FAILED (hr) )
			return hr;

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;

		pCoords->fLeftU -= pRectDest->left * tU;
		pCoords->fTopV -= pRectDest->top * tV;
		pCoords->fRightU += (desc.Width - pRectDest->right) * tU;
		pCoords->fBottomV += (desc.Height - pRectDest->bottom) * tV;
	}

	return S_OK;
}

/**
	@brief			: Scene_To_SceneScaled
	@description	: Scale down g_pTexScene by 1/4 x 1/4 and place the result in 
					  g_pTexSceneScaled
*/
HRESULT CAHDRLighting::Scene_To_SceneScaled (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;
	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
	hr = m_pTexSceneScaled->GetSurfaceLevel (0, &pSurfScaledScene);
	if (FAILED (hr) )
		goto LCleanReturn;

	D3DSURFACE_DESC pBackBufferDesc;

	LPDIRECT3DSURFACE9 backbuffer = NULL;
	pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	backbuffer->GetDesc (&pBackBufferDesc);

	// Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
	// are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
	// to keep the dimensions evenly divisible by 8; this allows for precise 
	// control over sampling inside pixel shaders.
	m_pEffect->SetTechnique ("DownScale4x4");

	// Place the rectangle in the center of the back buffer surface
	RECT rectSrc;
	rectSrc.left = (pBackBufferDesc.Width - m_dwCropWidth) / 2;
	rectSrc.top = (pBackBufferDesc.Height - m_dwCropHeight) / 2;
	rectSrc.right = rectSrc.left + m_dwCropWidth;
	rectSrc.bottom = rectSrc.top + m_dwCropHeight;

	// Get the texture coordinates for the render target
	CoordRect coords;
	GetTextureCoords (m_pTexScene, &rectSrc, m_pTexSceneScaled, NULL, &coords);

	// Get the sample offsets used within the pixel shader
	GetSampleOffsets_DownScale4x4 (pBackBufferDesc.Width, pBackBufferDesc.Height, avSampleOffsets);
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );

	pD3DDevice->SetRenderTarget (0, pSurfScaledScene);
	pD3DDevice->SetTexture (0, m_pTexScene);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	UINT uiPassCount, uiPass;
	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}
	m_pEffect->End ();

	hr = S_OK;

LCleanReturn:
	SAFE_RELEASE (pSurfScaledScene);
	return hr;
}

/**
	@brief			: SceneScaled_To_BrightPass
	@description	: Run the bright-pass filter on g_pTexSceneScaled and place the result
					  in g_pTexBrightPass
*/
HRESULT CAHDRLighting::SceneScaled_To_BrightPass (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;

	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights [MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBrightPass;
	hr = m_pTexBrightPass->GetSurfaceLevel (0, &pSurfBrightPass);
	if (FAILED (hr) )
		goto LCleanReturn;

	D3DSURFACE_DESC desc;
	m_pTexSceneScaled->GetLevelDesc (0, &desc);

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	GetTextureRect (m_pTexSceneScaled, &rectSrc);
	InflateRect (&rectSrc, -1, -1);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect (m_pTexBrightPass, &rectDest);
	InflateRect (&rectDest, -1, -1);

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords (m_pTexSceneScaled, &rectSrc, m_pTexBrightPass, &rectDest, &coords);

	// The bright-pass filter removes everything from the scene except lights and
	// bright reflections
	m_pEffect->SetTechnique ("BrightPassFilter");

	pD3DDevice->SetRenderTarget (0, pSurfBrightPass);
	pD3DDevice->SetTexture (0, m_pTexSceneScaled);
	pD3DDevice->SetTexture (1, m_pTexAdaptedLuminanceCur);
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
	pD3DDevice->SetScissorRect (&rectDest);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	UINT uiPass, uiPassCount;
	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, FALSE);

	hr = S_OK;

LCleanReturn:
	SAFE_RELEASE (pSurfBrightPass);
	return hr;
}

/**
	@brief			: BrightPass_To_StarSource
	@description	: Perform a 5x5 gaussian blur on g_pTexBrightPass and place the result
					  in g_pTexStarSource. The bright-pass filtered image is blurred before
					  being used for star operations to avoid aliasing artifacts.
*/
HRESULT CAHDRLighting::BrightPass_To_StarSource (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;

	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights [MAX_SAMPLES];

	// Get the new render target surface
	LPDIRECT3DSURFACE9 pSurfStarSource;
	hr = m_pTexStarSource->GetSurfaceLevel (0, &pSurfStarSource);
	if (FAILED (hr) )
		goto LCleanReturn;

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect (m_pTexStarSource, &rectDest);
	InflateRect (&rectDest, -1, -1);

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords (m_pTexBrightPass, NULL, m_pTexStarSource, &rectDest, &coords);

	// Get the sample offsets used within the pixel shader
	D3DSURFACE_DESC desc;
	hr = m_pTexBrightPass->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	GetSampleOffsets_GaussBlur5x5 (desc.Width, desc.Height, avSampleOffsets, avSampleWeights);
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );
	m_pEffect->SetValue ("g_avSampleWeights", avSampleWeights, sizeof (avSampleWeights) );

	// The gaussian blur smooths out rough edges to avoid aliasing effects
	// when the star effect is run
	m_pEffect->SetTechnique ("GaussBlur5x5");

	pD3DDevice->SetRenderTarget (0, pSurfStarSource);
	pD3DDevice->SetTexture (0, m_pTexBrightPass);
	pD3DDevice->SetScissorRect (&rectDest);
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	UINT uiPassCount, uiPass;
	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, FALSE);

	hr = S_OK;

LCleanReturn:
	SAFE_RELEASE (pSurfStarSource);
	return hr;
}

/**
	@brief			: StarSource_To_BloomSource
	@description	: Scale down g_pTexStarSource by 1/2 x 1/2 and place the result in 
					  g_pTexBloomSource
*/
HRESULT CAHDRLighting::StarSource_To_BloomSource (LPDIRECT3DDEVICE9 pD3DDevice)
{
	HRESULT hr = S_OK;

	D3DXVECTOR2 avSampleOffsets [MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBloomSource;
	hr = m_pTexBloomSource->GetSurfaceLevel (0, &pSurfBloomSource);
	if (FAILED (hr) )
		goto LCleanReturn;

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	GetTextureRect (m_pTexStarSource, &rectSrc);
	InflateRect (&rectSrc, -1, -1);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect (m_pTexBloomSource, &rectDest);
	InflateRect (&rectDest, -1, -1);

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords (m_pTexStarSource, &rectSrc, m_pTexBloomSource, &rectDest, &coords);

	// Get the sample offsets used within the pixel shader
	D3DSURFACE_DESC desc;
	hr = m_pTexBrightPass->GetLevelDesc (0, &desc);
	if (FAILED (hr) )
		return hr;

	GetSampleOffsets_DownScale2x2 (desc.Width, desc.Height, avSampleOffsets);
	m_pEffect->SetValue ("g_avSampleOffsets", avSampleOffsets, sizeof (avSampleOffsets) );

	// Create an exact 1/2 x 1/2 copy of the source texture
	m_pEffect->SetTechnique ("DownScale2x2");

	pD3DDevice->SetRenderTarget (0, pSurfBloomSource);
	pD3DDevice->SetTexture (0, m_pTexStarSource);
	pD3DDevice->SetScissorRect (&rectDest);
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
	pD3DDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	UINT uiPassCount, uiPass;
	hr = m_pEffect->Begin (&uiPassCount, 0);
	if (FAILED (hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
	{
		m_pEffect->BeginPass (uiPass);

		// Draw a fullscreen quad
		DrawFullScreenQuad (pD3DDevice, coords);

		m_pEffect->EndPass ();
	}

	m_pEffect->End ();
	pD3DDevice->SetRenderState (D3DRS_SCISSORTESTENABLE, FALSE);

	hr = S_OK;

LCleanReturn:
	SAFE_RELEASE (pSurfBloomSource);
	return hr;
}

/**
	@brief			: GetSampleOffsets_DownScale4x4
	@description	: Get the texture coordinate offsets to be used inside the DownScale4x4
					  pixel shader.
*/
HRESULT CAHDRLighting::GetSampleOffsets_DownScale4x4 (DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets [])
{
	if (NULL == avSampleOffsets)
		return E_INVALIDARG;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	// Sample from the 16 surrounding points. Since the center point will be in
	// the exact center of 16 texels, a 0.5f offset is needed to specify a texel
	// center.
	int index = 0;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			avSampleOffsets [index].x =  (x - 1.5f) * tU;
			avSampleOffsets [index].y =  (y - 1.5f) * tV;

			index ++;
		}
	}

	return S_OK;
}

/**
	@brief			: GetSampleOffsets_DownScale2x2
	@description	: Get the texture coordinate offsets to be used inside the DownScale2x2 pixel shader.
*/
HRESULT CAHDRLighting::GetSampleOffsets_DownScale2x2 (DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets [])
{
	if (NULL == avSampleOffsets)
		return E_INVALIDARG;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	// Sample from the 4 surrounding points. Since the center point will be in
	// the exact center of 4 texels, a 0.5f offset is needed to specify a texel
	// center.
	int index = 0;
	for (int y = 0; y < 2; ++y)
	{
		for (int x = 0; x < 2; ++x)
		{
			avSampleOffsets [index].x =  (x - 0.5f) * tU;
			avSampleOffsets [index].y =  (y - 0.5f) * tV;

			index ++;
		}
	}

	return S_OK;
}

/**
	@brief			: GetSampleOffsets_GaussBlur5x5
	@description	: Get the texture coordinate offsets to be used inside the GaussBlur5x5 pixel shader.
*/
HRESULT CAHDRLighting::GetSampleOffsets_GaussBlur5x5 (DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR2* avTexCoordOffset, D3DXVECTOR4* avSampleWeight, FLOAT fMultiplier)
{
	float tu = 1.0f /  (float)dwD3DTexWidth;
	float tv = 1.0f /  (float)dwD3DTexHeight;

	D3DXVECTOR4 vWhite (1.0f, 1.0f, 1.0f, 1.0f);

	float totalWeight = 0.0f;
	int index = 0;
	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -2; y <= 2; ++y)
		{
			// Exclude pixels with a block distance greater than 2. This will
			// create a kernel which approximates a 5x5 kernel using only 13
			// sample points instead of 25; this is necessary since 2.0 shaders
			// only support 16 texture grabs.
			if (abs (x) + abs (y) > 2)
				continue;

			// Get the unscaled Gaussian intensity for this offset
			avTexCoordOffset [index] = D3DXVECTOR2 (x * tu, y * tv);
			avSampleWeight [index] = vWhite * GaussianDistribution ( (float)x,  (float)y, 1.0f);
			totalWeight += avSampleWeight [index].x;

			index ++;
		}
	}

	// Divide the current weight by the total weight of all the samples; Gaussian
	// blur kernels add to 1.0f to ensure that the intensity of the image isn't
	// changed when the blur occurs. An optional multiplier variable is used to
	// add or remove image intensity during the blur.
	for (int i = 0; i < index; ++i)
	{
		avSampleWeight [i] /= totalWeight;
		avSampleWeight [i] *= fMultiplier;
	}

	return S_OK;
}

/**
	@brief			: GetSampleOffsets_Bloom
	@description	: Get the texture coordinate offsets to be used inside the Bloom pixel shader.
*/
HRESULT CAHDRLighting::GetSampleOffsets_Bloom (DWORD dwD3DTexSize, float afTexCoordOffset [15], D3DXVECTOR4* avColorWeight, float fDeviation, float fMultiplier)
{
	int i = 0;
	float tu = 1.0f /  (float)dwD3DTexSize;

	// Fill the center texel
	float weight = fMultiplier * GaussianDistribution (0, 0, fDeviation);
	avColorWeight [0] = D3DXVECTOR4 (weight, weight, weight, 1.0f);

	afTexCoordOffset [0] = 0.0f;

	// Fill the first half
	for (i = 1; i < 8; ++i)
	{
		// Get the Gaussian intensity for this offset
		weight = fMultiplier * GaussianDistribution ( (float)i, 0, fDeviation);
		afTexCoordOffset [i] = i * tu;

		avColorWeight [i] = D3DXVECTOR4 (weight, weight, weight, 1.0f);
	}

	// Mirror to the second half
	for (i = 8; i < 15; ++i)
	{
		avColorWeight [i] = avColorWeight [i - 7];
		afTexCoordOffset [i] = -afTexCoordOffset [i - 7];
	}

	return S_OK;
}

/**
	@brief			: GetSampleOffsets_Bloom
	@description: Get the texture coordinate offsets to be used inside the Bloom pixel shader.
*/
HRESULT CAHDRLighting::GetSampleOffsets_Star (DWORD dwD3DTexSize, float afTexCoordOffset [15], D3DXVECTOR4* avColorWeight, float fDeviation)
{
	int i = 0;
	float tu = 1.0f /  (float)dwD3DTexSize;

	// Fill the center texel
	float weight = 1.0f * GaussianDistribution (0, 0, fDeviation);
	avColorWeight [0] = D3DXVECTOR4 (weight, weight, weight, 1.0f);

	afTexCoordOffset [0] = 0.0f;

	// Fill the first half
	for (i = 1; i < 8; ++i)
	{
		// Get the Gaussian intensity for this offset
		weight = 1.0f * GaussianDistribution ( (float)i, 0, fDeviation);
		afTexCoordOffset [i] = i * tu;

		avColorWeight [i] = D3DXVECTOR4 (weight, weight, weight, 1.0f);
	}

	// Mirror to the second half
	for (i = 8; i < 15; ++i)
	{
		avColorWeight [i] = avColorWeight [i - 7];
		afTexCoordOffset [i] = -afTexCoordOffset [i - 7];
	}

	return S_OK;
}

void CAHDRLighting::Lost ()
{
	int i;

	if (m_pEffect)
		m_pEffect->OnLostDevice ();

	SAFE_RELEASE (m_pmeshSphere);

	SAFE_RELEASE (m_pFloatMSRT);
	SAFE_RELEASE (m_pFloatMSDS);
	SAFE_RELEASE (m_pTexScene);
	SAFE_RELEASE (m_pTexSceneScaled);
	//SAFE_RELEASE (m_pTexWall);
	//SAFE_RELEASE (m_pTexFloor);
	//SAFE_RELEASE (m_pTexCeiling);
	//SAFE_RELEASE (m_pTexPainting);
	SAFE_RELEASE (m_pTexAdaptedLuminanceCur);
	SAFE_RELEASE (m_pTexAdaptedLuminanceLast);
	SAFE_RELEASE (m_pTexBrightPass);
	SAFE_RELEASE (m_pTexBloomSource);
	SAFE_RELEASE (m_pTexStarSource);

	for (i = 0; i < NUM_TONEMAP_TEXTURES; ++i)
		SAFE_RELEASE (m_apTexToneMap [i]);

	for (i = 0; i < NUM_STAR_TEXTURES; ++i)
		SAFE_RELEASE (m_apTexStar [i]);

	for (i = 0; i < NUM_BLOOM_TEXTURES; ++i)
		SAFE_RELEASE (m_apTexBloom [i]);
}

void CAHDRLighting::Destroy ()
{
	SAFE_RELEASE (m_pEffect);
}