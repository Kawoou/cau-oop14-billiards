#include "AWindow.h"

CAXModel::CAXModel(void)
{
	m_pMeshObject = NULL;

	m_pIB = NULL;
	m_pVB = NULL;
	m_pDecl = NULL;

	m_dwNumMaterials = 0;
	m_pTextures = NULL;
	m_pMaterials = NULL;
	m_strMaterials = NULL;

	m_bUpdateMatrix = true;
}

CAXModel::~CAXModel(void)
{
	Unload();

	for (unsigned int i = 0; i < m_dwNumMaterials; i++)
		SAFE_RELEASE(m_pTextures[i]);
	SAFE_DELETE(m_pTextures);
	SAFE_DELETE_ARRAY(m_pMaterials);
	SAFE_DELETE_ARRAY(m_strMaterials);

	SAFE_RELEASE(m_pIB);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pDecl);

	SAFE_RELEASE(m_pMeshObject);
}

bool CAXModel::Create(const char* strFileName)
{
	return this->Load(strFileName);
}

bool CAXModel::Load(const char* strFileName)
{
	this->Unload();

	m_strFile = strFileName;

	LPD3DXBUFFER pAdjacencyBuffer = NULL;
	LPD3DXBUFFER pMtrlBuffer = NULL;
	if (FAILED(D3DXLoadMeshFromXA(m_strFile.c_str(), D3DXMESH_MANAGED, APROJECT_WINDOW->GetD3DDevice(), &pAdjacencyBuffer, &pMtrlBuffer, NULL, &m_dwNumMaterials, &m_pMeshObject)))
		return false;

	// Optimize the mesh for performance
	if (FAILED(m_pMeshObject->OptimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL)))
	{
		SAFE_RELEASE(pAdjacencyBuffer);
		SAFE_RELEASE(pMtrlBuffer);
		return false;
	}

	D3DXMATERIAL* d3dxMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer();
	do
	{
		if (d3dxMtrls && m_dwNumMaterials > 0)
		{
			// Allocate memory for the materials and textures
			m_pMaterials = new D3DMATERIAL9[m_dwNumMaterials];
			if (m_pMaterials == NULL)
				break;

			m_pTextures = new LPDIRECT3DBASETEXTURE9[m_dwNumMaterials];
			if (m_pTextures == NULL)
				break;

			m_strMaterials = new CHAR[m_dwNumMaterials][MAX_PATH];
			if (m_strMaterials == NULL)
				break;

			// Copy each material and create its texture
			for (DWORD i = 0; i < m_dwNumMaterials; i++)
			{
				// Copy the material
				m_pMaterials[i] = d3dxMtrls[i].MatD3D;
				m_pTextures[i] = NULL;

				// Create a texture
				if (d3dxMtrls[i].pTextureFilename)
				{
					strcpy_s(m_strMaterials[i], MAX_PATH, d3dxMtrls[i].pTextureFilename);

					CHAR strTexture[MAX_PATH];
					D3DXIMAGE_INFO ImgInfo;

					// First attempt to look for texture in the same folder as the input folder.
					int p = 0;
					strcpy_s(strTexture, MAX_PATH, m_strFile.c_str());
					for (DWORD j = 0; j < strlen(strTexture); j++)
					{
						if (strTexture[j] == '/')
							p = j;
					}
					strTexture[p + 1] = 0;
					strcat_s(strTexture, MAX_PATH, d3dxMtrls[i].pTextureFilename);

					// Inspect the texture file to determine the texture type.
					if (FAILED(D3DXGetImageInfoFromFileA(strTexture, &ImgInfo)))
						continue;

					// Call the appropriate loader according to the texture type.
					switch (ImgInfo.ResourceType)
					{
						case D3DRTYPE_TEXTURE:
						{
							IDirect3DTexture9* pTex;
							if (SUCCEEDED(D3DXCreateTextureFromFileA(APROJECT_WINDOW->GetD3DDevice(), strTexture, &pTex)))
							{
								pTex->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&m_pTextures[i]);
								pTex->Release();
							}
							break;
						}

						case D3DRTYPE_CUBETEXTURE:
						{
							IDirect3DCubeTexture9* pTex;
							if (SUCCEEDED(D3DXCreateCubeTextureFromFileA(APROJECT_WINDOW->GetD3DDevice(), strTexture, &pTex)))
							{
								pTex->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&m_pTextures[i]);
								pTex->Release();
							}
							break;
						}

						case D3DRTYPE_VOLUMETEXTURE:
						{
							IDirect3DVolumeTexture9* pTex;
							if (SUCCEEDED(D3DXCreateVolumeTextureFromFileA(APROJECT_WINDOW->GetD3DDevice(), strTexture, &pTex)))
							{
								pTex->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&m_pTextures[i]);
								pTex->Release();
							}
							break;
						}
					}
				}
			}
		}
	} while (0);

	// Extract data from m_pMesh for easy access
	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
	m_dwNumVertices = m_pMeshObject->GetNumVertices();
	m_dwNumFaces = m_pMeshObject->GetNumFaces();
	m_dwBytesPerVertex = m_pMeshObject->GetNumBytesPerVertex();
	m_pMeshObject->GetIndexBuffer(&m_pIB);
	m_pMeshObject->GetVertexBuffer(&m_pVB);
	m_pMeshObject->GetDeclaration(decl);
	APROJECT_WINDOW->GetD3DDevice()->CreateVertexDeclaration(decl, &m_pDecl);

	SAFE_RELEASE(pAdjacencyBuffer);
	SAFE_RELEASE(pMtrlBuffer);

	return true;
}

void CAXModel::Unload(void)
{
	SAFE_RELEASE(m_pIB);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pDecl);

	for (UINT i = 0; i < m_dwNumMaterials; i++)
		SAFE_RELEASE(m_pTextures[i]);
	SAFE_DELETE_ARRAY(m_pTextures);
	SAFE_DELETE_ARRAY(m_pMaterials);
	SAFE_DELETE_ARRAY(m_strMaterials);

	SAFE_RELEASE(m_pMeshObject);

	m_dwNumMaterials = 0L;
}

void CAXModel::SetAnchorPoint(AVector3 vector)
{
	m_vAnchorPoint = vector;
}

AVector3 CAXModel::GetAnchorPoint()
{
	return m_vAnchorPoint;
}

void CAXModel::Reset(void)
{
	
}

void CAXModel::MoveX(AReal32 fX)										{ CAObject::MoveX(fX); m_bUpdateMatrix = true; }
void CAXModel::MoveY(AReal32 fY)										{ CAObject::MoveY(fY); m_bUpdateMatrix = true; }
void CAXModel::MoveZ(AReal32 fZ)										{ CAObject::MoveZ(fZ); m_bUpdateMatrix = true; }
void CAXModel::MoveRotX(AReal32 fRotate)								{ m_fOldRotX = m_fRotX; CAObject::MoveRotX(fRotate); m_bUpdateMatrix = true; }
void CAXModel::MoveRotY(AReal32 fRotate)								{ m_fOldRotY = m_fRotY; CAObject::MoveRotY(fRotate); m_bUpdateMatrix = true; }
void CAXModel::MoveRotZ(AReal32 fRotate)								{ m_fOldRotZ = m_fRotZ; CAObject::MoveRotZ(fRotate); m_bUpdateMatrix = true; }
void CAXModel::SetPos(AReal32 fX, AReal32 fY)							{ CAObject::SetPos(fX, fY); m_bUpdateMatrix = true; }
void CAXModel::SetPos(AReal32 fX, AReal32 fY, AReal32 fZ)				{ CAObject::SetPos(fX, fY, fZ); m_bUpdateMatrix = true; }
void CAXModel::SetPos(AVector3 vPos)									{ CAObject::SetPos(vPos); m_bUpdateMatrix = true; }
void CAXModel::SetSize(AReal32 fSizeX, AReal32 fSizeY)					{ CAObject::SetSize(fSizeX, fSizeY); m_bUpdateMatrix = true; }
void CAXModel::SetSize(AReal32 fSizeX, AReal32 fSizeY, AReal32 fSizeZ)	{ CAObject::SetSize(fSizeX, fSizeY, fSizeZ); m_bUpdateMatrix = true; }
void CAXModel::SetSize(AVector3 vSize)									{ CAObject::SetSize(vSize); m_bUpdateMatrix = true; }
void CAXModel::SetOffset(AReal32 fX, AReal32 fY)						{ CAObject::SetOffset(fX, fY);  m_bUpdateMatrix = true; }
void CAXModel::SetOffset(AReal32 fX, AReal32 fY, AReal32 fZ)			{ CAObject::SetOffset(fX, fY, fZ); m_bUpdateMatrix = true; }
void CAXModel::SetOffset(AVector3 vPos)									{ CAObject::SetOffset(vPos); m_bUpdateMatrix = true; }
void CAXModel::SetRotX(AReal32 fRotate)									{ m_fOldRotX = m_fRotX; CAObject::SetRotX(fRotate); m_bUpdateMatrix = true; }
void CAXModel::SetRotY(AReal32 fRotate)									{ m_fOldRotY = m_fRotY; CAObject::SetRotY(fRotate); m_bUpdateMatrix = true; }
void CAXModel::SetRotZ(AReal32 fRotate)									{ m_fOldRotZ = m_fRotZ; CAObject::SetRotZ(fRotate); m_bUpdateMatrix = true; }

void CAXModel::SetMatrix(D3DXMATRIX& mat)
{
	m_matComplate = mat;

	m_bUpdateMatrix = false;
}

void CAXModel::UpdateMatrix(void)
{
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;
	D3DXMATRIX matRotate;
	D3DXMATRIX matAnchorPoint;
	D3DXMATRIX matComplate;
	
	D3DXMATRIX mT;
	{
		AVector3 vLeft(1.0f, 0.0f, 0.0f);
		AVector3 vUp(0.0f, 1.0f, 0.0f);
		AVector3 vLook(0.0f, 0.0f, 1.0f);
		AVector3 vUpNew, vLeftNew, vLookNew;
		D3DXMATRIX mX, mY, mZ;

		D3DXMatrixRotationX(&mX, m_fRotX);
		D3DXMatrixRotationY(&mY, m_fRotY);
		D3DXMatrixRotationZ(&mZ, -m_fRotZ);

		D3DXMatrixTranslation(&mT, vUp.x, vUp.y, vUp.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vUpNew = AVector3(mT._41, mT._42, mT._43);

		D3DXMatrixTranslation(&mT, vLeft.x, vLeft.y, vLeft.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vLeftNew = AVector3(mT._41, mT._42, mT._43);

		D3DXMatrixTranslation(&mT, vLook.x, vLook.y, vLook.z);
		mT *= mX; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mY; D3DXMatrixTranslation(&mT, mT._41, mT._42, mT._43);
		mT *= mZ; vLookNew = AVector3(mT._41, mT._42, mT._43);

		mT._11 = vLeftNew.x;   mT._12 = vLeftNew.y;   mT._13 = vLeftNew.z;   mT._14 = 0.0f;		// Left Vector
		mT._21 = vUpNew.x;     mT._22 = vUpNew.y;     mT._23 = vUpNew.z;     mT._24 = 0.0f;		// Up Vector
		mT._31 = vLookNew.x;   mT._32 = vLookNew.y;   mT._33 = vLookNew.z;   mT._34 = 0.0f;		// Front Vector
		mT._41 = 0.0f;         mT._42 = 0.0f;         mT._43 = 0.0f;         mT._44 = 1.0f;		// Translation
	}

	D3DXMatrixTranslation(&matAnchorPoint, m_vAnchorPoint.x, m_vAnchorPoint.y, m_vAnchorPoint.z);
	D3DXMatrixScaling(&matSize, m_vSize.x, m_vSize.y, m_vSize.z);
	D3DXMatrixTranslation(&matPosition, m_vPosition.x, m_vPosition.y, m_vPosition.z);

	m_matComplate = matAnchorPoint * matSize * mT * matPosition;
	
	m_bUpdateMatrix = false;
}

void CAXModel::Update(void)
{
	if (m_bUpdateMatrix)
		UpdateMatrix();
}

void CAXModel::Render(void)
{
	unsigned int i;
	//if (!m_bFrustumState)
	//	return;

	APROJECT_WINDOW->GetLightEffect()->SetMatrix("World", &m_matComplate);
	APROJECT_WINDOW->GetLightEffect()->SetMatrix("View", APROJECT_WINDOW->GetCurScene()->GetCamera()->GetViewMatrix());
	APROJECT_WINDOW->GetLightEffect()->SetMatrix("Projection", APROJECT_WINDOW->GetCurScene()->GetCamera()->GetProjectionMatrix());
	{
		UINT totalPasses;
		D3DXHANDLE hTechnique;
		
		for (i = 0; i < m_dwNumMaterials; ++i)
		{
			stMaterial material = {
				m_pMaterials[i].Ambient,
				m_pMaterials[i].Diffuse,
				m_pMaterials[i].Emissive,
				m_pMaterials[i].Specular,
				m_pMaterials[i].Power
			};
			if (m_pTextures[i] != NULL)
				hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("Obj_Dif");
			else
				hTechnique = APROJECT_WINDOW->GetLightEffect()->GetTechniqueByName("Obj_None");
			
			APROJECT_WINDOW->GetLightEffect()->SetValue("g_mMaterial", &material, sizeof (material));
			APROJECT_WINDOW->GetLightEffect()->SetTexture("Texture1", m_pTextures[i]);

			if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->SetTechnique(hTechnique)))
			{
				if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->Begin(&totalPasses, 0)))
				{
					for (UINT pass = 0; pass < totalPasses; ++pass)
					{
						if (SUCCEEDED(APROJECT_WINDOW->GetLightEffect()->BeginPass(pass)))
						{
							m_pMeshObject->DrawSubset(i);
							APROJECT_WINDOW->GetLightEffect()->EndPass();
						}
					}
					APROJECT_WINDOW->GetLightEffect()->End();
				}
			}
		} // End For
	}
}
