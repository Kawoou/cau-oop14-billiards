#include "AModelInclude.h"

static char STR_BUFFER[256] = {0,};

CAModelLoader::CAModelLoader (void)
{
}

CAModelLoader::~CAModelLoader (void)
{
}

CAModelLoader* CAModelLoader::GetSingleton (void)
{
	static CAModelLoader pModelLoader;

	return &pModelLoader;
}

bool CAModelLoader::Load (CAModel* pModel, const char* strFileName, int nSkinningMode)
{
	m_listModelVIB.clear ();

	if (!m_xmlFile.Open (strFileName) )
	{
		DebugLog ("Load(Could not find %s) 실패!", strFileName);
		DebugConsoleLog (DebugError, "CAModelLoader: Load(Could not find %s) 실패!", strFileName);
		return false;
	}

	// 기본 정보 로드
	if (!ParseInfo (pModel) )
		return false;
	
	// 재질 정보 로드
	if (!ParseMaterial (pModel) )
		return false;

	// 실제 오브젝트 객체 로드
	if (!ParseObject (pModel, nSkinningMode) )
		return false;

	DebugConsoleLog (DebugClear, "CAModelLoader: Load Model XML File, the success");

	return true;
}

bool CAModelLoader::ParseInfo (CAModel* pModel)
{
	if (GetTextFromXML ("Body/Info/ObjectCount", STR_BUFFER) )
		pModel->SetObjCount (atoi (STR_BUFFER) );

	if (GetTextFromXML ("Body/Info/MeshCount", STR_BUFFER) )
		pModel->SetMeshCount (atoi (STR_BUFFER) );
	
	if (GetTextFromXML ("Body/Info/AnimationStart", STR_BUFFER) )
		pModel->SetAniStart ((float)atof (STR_BUFFER) );
	
	if (GetTextFromXML ("Body/Info/AnimationEnd", STR_BUFFER) )
		pModel->SetAniEnd ((float)atof (STR_BUFFER) );
	
	return true;
}

bool CAModelLoader::ParseMaterial (CAModel* pModel)
{
	IXMLDOMNodeList* pList = m_xmlFile.FindElement ("Body/Material/Slot");

	if (!pList)
		return false;

	long nCount = 0;
	pList->get_length (&nCount);

	IXMLDOMNode* pNode = NULL;
	for (int i = 0; i != nCount; i ++)
	{
		std::string tmp;
		CAModelMaterial* pMaterial = new CAModelMaterial ();

		pList->get_item (i, &pNode);

		if (GetTextFromXML (pNode, "Diffuse", STR_BUFFER) )
			pMaterial->m_Material.Diffuse = StrToColor3f (STR_BUFFER);

		if (GetTextFromXML (pNode, "Ambient", STR_BUFFER) )
			pMaterial->m_Material.Ambient = StrToColor3f (STR_BUFFER);

		if (GetTextFromXML (pNode, "Specular", STR_BUFFER) )
			pMaterial->m_Material.Specular =StrToColor3f (STR_BUFFER);

		if (GetTextFromXML (pNode, "Emissive", STR_BUFFER) )
			pMaterial->m_Material.Emissive = StrToColor3f (STR_BUFFER);
			
		if (GetTextFromXML (pNode, "Opacity", STR_BUFFER) )
			pMaterial->m_fOpacity = (float)atof (STR_BUFFER);

		if (GetTextFromXML (pNode, "Power", STR_BUFFER) )
			pMaterial->m_Material.Power = (float)atof (STR_BUFFER);

		if (GetTextFromXML (pNode, "BumpMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pBumpTexture);
		}
		if (GetTextFromXML (pNode, "DiffuseMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pDiffuseTexture);
		}
		if (GetTextFromXML (pNode, "HeightMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pHeightTexture);
		}
		if (GetTextFromXML (pNode, "LuminanceMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pLuminanceTexture);
		}
		if (GetTextFromXML (pNode, "SpecularMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pSpecularTexture);
		}
		if (GetTextFromXML (pNode, "OpacityMap", STR_BUFFER) )
		{
			tmp = m_xmlFile.m_strDir + STR_BUFFER;
			D3DXCreateTextureFromFileA (APROJECT_WINDOW->GetD3DDevice (), tmp.data (), &pMaterial->m_pOpacityTexture);
		}

		// [diffuse, normal, bump, etc...]

		pModel->AddMaterial (pMaterial);
	}
	return true;
}

bool CAModelLoader::ParseObject (CAModel* pModel, int nSkinningMode)
{
	IXMLDOMNodeList* pList = m_xmlFile.FindElement ("Body/Object");

	if (!pList)
		return false;

	long nCount = 0;
	pList->get_length (&nCount);

	IXMLDOMNode* pNode = NULL;
	for (int i = 0; i != nCount; ++i)
	{
//		[작업순서]
//		Vertex관련정보를 모두 파싱하고 정리한후 이를 바탕으로 Object
//		타입을 결정하여 만들고 정보를 셋팅한다.
//		
//		[부가작업]
//		1. 재질의 두개이상인 오브젝트쪼개기.
//		2. 버텍스인덱스와 텍스쳐인덱스에 따른 정점리스트 및 가중치리스트 재정리

		pList->get_item (i, &pNode);

		CAModelVIB modelVIB;
		ParseVertex (modelVIB, pNode);
		ParseNormal (modelVIB, pNode);
		ParseUV (modelVIB, pNode);
		ParseIndex (modelVIB, pNode);
		ParseUVIndex (modelVIB, pNode);
		ParseWeight (modelVIB, pNode);

		SplitModelVIB (modelVIB);
		for (int i = 0; i != (int)m_listModelVIB.size () ; ++i)
			OrganizeModelVIB (m_listModelVIB [i]);

		for (int i = 0; i != (int)m_listModelVIB.size () ; ++i)
		{
			m_xmlFile.GetAttributeText (pNode, "Class", STR_BUFFER);
			CAModelObject* pObject = CreateModelObject (STR_BUFFER, pModel, !m_listModelVIB [i].m_listWeight.empty (), nSkinningMode);

			pModel->AddObject (pObject, STR_BUFFER);

			m_xmlFile.GetAttributeText (pNode, "ID", STR_BUFFER);		
			pObject->SetObjectID (atoi (STR_BUFFER) );

			if (!m_listModelVIB [i].m_listIndex.empty () )
				pObject->SetMaterialID (max (m_listModelVIB [i].m_listIndex [0].m_nMaterialID, 0) );
			
			ParseObjectParent (pObject, pNode);
			ParseObjectBBox (pObject, pNode);
			ParseObjectLocalTM (pObject, pNode);
			ParseObjectWorldTM (pObject, pNode);
			ParseObjectKeyPos (pObject, pNode);
			ParseObjectKeyRot (pObject, pNode, pModel->GetStartFrame () );
			ParseObjectKeyScl (pObject, pNode);

			pObject->MakeVIB (m_listModelVIB [i]);
		}

		m_listModelVIB.clear ();
	}
	return true;
}

CAModelObject* CAModelLoader::CreateModelObject (char* strClass, CAModel* pModel, bool bSkin, int nSkinningMode)
{
	if (strcmp (strClass , "Editable_mesh") == 0)	
	{
		if (bSkin)
		{
			switch (nSkinningMode)
			{
			case SKINNING_SW:	return (CAModelObject*)new CAMeshSWSkin (pModel);
			case SKINNING_FF:	return (CAModelObject*)new CAMeshFFSkin (pModel);
			case SKINNING_SH:	return (CAModelObject*)new CAMeshSHSkin (pModel);

			default:			return NULL;
			}
		}
		else
		{
			return (CAModelObject*)new CAMeshRigid (pModel);
		}
	}
	else if (strcmp (strClass, "Bone") == 0)			return new CAModelBone (pModel);
	else if (strcmp (strClass, "Biped_Object") == 0)	return new CAModelBone (pModel);
	else if (strcmp (strClass, "BoneGeometry") == 0)	return new CAModelBone (pModel);
	else												return new CAModelObject (pModel);
}

// 1
void CAModelLoader::ParseVertex (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "Vertex/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);
		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		modelVIB.m_listVertex.push_back (StrToVector3f (STR_BUFFER) );
	}
}

void CAModelLoader::ParseNormal (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "VertexNormal/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);
		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);
		
		modelVIB.m_listNormal.push_back (StrToVector3f (STR_BUFFER) );
	}
}

void CAModelLoader::ParseUV (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "TexVertex/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);
		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		modelVIB.m_listUV.push_back (StrToVector2f (STR_BUFFER) );
	}
}

void CAModelLoader::ParseIndex (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "TriIndex");

	if (!pListChild)
		return;

	pListChild->get_item (0, &pNodeChild);

	if (!pNodeChild)
		return;

	m_xmlFile.GetAttributeText (pNodeChild, "MaterialCount", STR_BUFFER);
	modelVIB.m_nNumMaterial = atoi (STR_BUFFER);

	pListChild = m_xmlFile.FindElement (pNode, "TriIndex/Value");

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		CAVertexIndex vertexIndex;

		pListChild->get_item (i, &pNodeChild);
		m_xmlFile.GetAttributeText (pNodeChild, "MaterialID", STR_BUFFER);

		vertexIndex.m_nMaterialID = atoi (STR_BUFFER);

		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		StrToIndex (STR_BUFFER, vertexIndex.m_nIndex);

		modelVIB.m_listIndex.push_back (vertexIndex);
	}
}

void CAModelLoader::ParseUVIndex (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "TexIndex/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		CAUVIndex uvIndex;

		pListChild->get_item (i, &pNodeChild);
		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		StrToIndex (STR_BUFFER, uvIndex.m_nIndex);

		modelVIB.m_listUVIndex.push_back (uvIndex);
	}
}

void CAModelLoader::ParseWeight (CAModelVIB& modelVIB, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild1 = NULL;
	IXMLDOMNodeList* pListChild2 = NULL;
	IXMLDOMNode* pNodeChild1 = NULL;
	IXMLDOMNode* pNodeChild2 = NULL;

	pListChild1 = m_xmlFile.FindElement (pNode, "VertexWeight/Vertex");

	if (!pListChild1)
		return;

	long nCount1 = 0;
	pListChild1->get_length (&nCount1);

	for (int i = 0; i != nCount1; ++i)
	{
		pListChild1->get_item (i, &pNodeChild1);
		pListChild2 = m_xmlFile.FindElement (pNodeChild1, "Value");

		if (!pListChild2)
			continue;

		CAVertexWeight vertexWeight;

		long nCount2 = 0;
		pListChild2->get_length (&nCount2);

		for (int j = 0; j != nCount2 ; ++j)
		{
			pListChild2->get_item (j, &pNodeChild2);
			m_xmlFile.GetAttributeText (pNodeChild2, "BoneID", STR_BUFFER);			

			vertexWeight.m_listBoneID.push_back (atoi (STR_BUFFER) );

			pListChild2->get_item (j, &pNodeChild2);
			m_xmlFile.GetElementText (pNodeChild2, STR_BUFFER);

			vertexWeight.m_listWeight.push_back ((float)atof (STR_BUFFER) );
		}
		
		modelVIB.m_listWeight.push_back (vertexWeight);
	}
}

// 2
void CAModelLoader::SplitModelVIB (CAModelVIB& modelVIB)
{
	if (modelVIB.m_nNumMaterial <= 1)
	{
		m_listModelVIB.push_back (modelVIB);
		return;
	}

	set<int> setMaterialID;
	for (int i = 0; i != (int)modelVIB.m_listIndex.size (); ++i)
		setMaterialID.insert (modelVIB.m_listIndex [i].m_nMaterialID);

	for (set<int>::iterator itor = setMaterialID.begin (); itor != setMaterialID.end (); ++itor)
	{
		CAModelVIB vib;
		vib = modelVIB;

		vib.m_listIndex.clear();

		for (int i = 0; i != (int)modelVIB.m_listIndex.size (); ++i)
		{
			if (*itor == modelVIB.m_listIndex [i].m_nMaterialID)
				vib.m_listIndex.push_back (modelVIB.m_listIndex [i]);
		}

		m_listModelVIB.push_back (vib);
	}
}

void CAModelLoader::OrganizeModelVIB (CAModelVIB& modelVIB)
{
	OrganizeWeight (modelVIB);
	OrganizeIndex (modelVIB);
}

void CAModelLoader::OrganizeWeight (CAModelVIB& modelVIB)
{
	for (int i = 0; i != (int)modelVIB.m_listWeight.size (); ++i)
	{
		int nCount = (int)modelVIB.m_listWeight [i].m_listBoneID.size ();

		// blend weight가 4개이하라면 강제로 4개로 만든다. 
		while (nCount < 4)
		{
			modelVIB.m_listWeight [i].m_listBoneID.push_back (0);
			modelVIB.m_listWeight [i].m_listWeight.push_back (0.0f);

			++ nCount;
		}

		// Matrix Palette를 쓰기위한 인덱스 DWORD를 만든다.
		modelVIB.m_listWeight [i].m_dwWeightIndex = (modelVIB.m_listWeight [i].m_listBoneID [3]) << 24 | 
													(modelVIB.m_listWeight [i].m_listBoneID [2]) << 16 |
													(modelVIB.m_listWeight [i].m_listBoneID [1]) << 8  |
													(modelVIB.m_listWeight [i].m_listBoneID [0]);
	}
}

void CAModelLoader::OrganizeIndex (CAModelVIB& modelVIB)
{
	if (modelVIB.m_listUVIndex.empty () )
		return;

	vector<CAVertexIndex>& vertexIndex = modelVIB.m_listIndex;
	vector<CAUVIndex>& uvIndex = modelVIB.m_listUVIndex;

	for (int i = 0; i != (int)vertexIndex.size (); ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			int nVertexIndex = vertexIndex [i].m_nIndex [j];
			int nUVIndex = uvIndex [i].m_nIndex [j];

			map<int,int>::iterator itor = modelVIB.m_mapIndex.find (nVertexIndex);

			if (itor == modelVIB.m_mapIndex.end () )
				modelVIB.m_mapIndex.insert (make_pair (nVertexIndex, nUVIndex) );
			else
			{
				if (itor->second != nUVIndex)
				{
					int nNewVertexIndex = (int)modelVIB.m_listVertex.size ();

					D3DXVECTOR3 newVertex = modelVIB.m_listVertex [nVertexIndex];
					modelVIB.m_listVertex.push_back (newVertex);

					if (!modelVIB.m_listNormal.empty () )
					{
						D3DXVECTOR3 newNormal = modelVIB.m_listNormal [nVertexIndex];
						modelVIB.m_listNormal.push_back (newNormal);
					}

					if (!modelVIB.m_listWeight.empty () )
					{
						CAVertexWeight newWeight = modelVIB.m_listWeight [nVertexIndex];
						modelVIB.m_listWeight.push_back (newWeight);
					}

					vertexIndex [i].m_nIndex [j] = nNewVertexIndex;

					modelVIB.m_mapIndex.insert (make_pair (nNewVertexIndex, nUVIndex) );
				}
			}				
		}
	}	
}

// 3
void CAModelLoader::ParseObjectParent (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "Parent");

	if (!pListChild)
		return;

	pListChild->get_item (0, &pNodeChild);

	m_xmlFile.GetAttributeText (pNodeChild, "ID", STR_BUFFER);

	int nParentID = atoi (STR_BUFFER);

	pObject->SetParentID (nParentID < 0 ? 0 : nParentID);
}

void CAModelLoader::ParseObjectBBox (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	if (GetTextFromXML (pNode,"BoundingBox/Max", STR_BUFFER) )
		pObject->SetBBoxMax (StrToVector3f (STR_BUFFER) );

	if (GetTextFromXML (pNode,"BoundingBox/Min", STR_BUFFER) )
		pObject->SetBBoxMin (StrToVector3f (STR_BUFFER) );
}

void CAModelLoader::ParseObjectLocalTM (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;
	D3DXVECTOR4 v [4];
	ZeroMemory (v, sizeof (D3DXVECTOR4) * 4);

	pListChild = m_xmlFile.FindElement (pNode, "LocalTM/Value");

	if (!pListChild)
		return;

	for (int i = 0; i != 4; ++i)
	{
		pListChild->get_item (i, &pNodeChild);

		if (!pNodeChild)
			return;
        
		if (m_xmlFile.GetElementText (pNodeChild, STR_BUFFER) )
			v [i] = StrToVector4f (STR_BUFFER);
	}

	pObject->SetLocalTM (D3DXMATRIX(v[0].x, v[0].y, v[0].z, v[0].w, 
									v[1].x, v[1].y, v[1].z, v[1].w, 
									v[2].x, v[2].y, v[2].z, v[2].w, 
									v[3].x, v[3].y, v[3].z, v[3].w) );
}

void CAModelLoader::ParseObjectWorldTM (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;
	D3DXVECTOR4 v [4];
	ZeroMemory (v, sizeof (D3DXVECTOR4) * 4);

	pListChild = m_xmlFile.FindElement (pNode, "WorldTM/Value");

	if (!pListChild)
		return;

	for (int i = 0; i != 4; ++i)
	{
		pListChild->get_item (i, &pNodeChild);

		if (!pNodeChild)
			return;

		if (m_xmlFile.GetElementText (pNodeChild, STR_BUFFER) )
			v [i] = StrToVector4f (STR_BUFFER);
	}

	pObject->SetWorldTM (D3DXMATRIX(v[0].x, v[0].y, v[0].z, v[0].w, 
									v[1].x, v[1].y, v[1].z, v[1].w, 
									v[2].x, v[2].y, v[2].z, v[2].w, 
									v[3].x, v[3].y, v[3].z, v[3].w) );
}

void CAModelLoader::ParseObjectKeyPos (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "Key/Position/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);

		m_xmlFile.GetAttributeText (pNodeChild, "Frame", STR_BUFFER);

		float fFrame = (float)atof (STR_BUFFER);

		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		D3DXVECTOR3 pos = StrToVector3f (STR_BUFFER);
	
		pObject->GetTrack ()->AddPosKeyFrame (fFrame, pos);
	}
}

void CAModelLoader::ParseObjectKeyRot (CAModelObject* pObject, IXMLDOMNode* pNode, float fStartFrame)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "Key/Quaternion/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);

		m_xmlFile.GetAttributeText (pNodeChild, "Frame", STR_BUFFER);

		float fFrame = (float)atof (STR_BUFFER);

		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		D3DXVECTOR4 rot = StrToVector4f (STR_BUFFER);
	
		pObject->GetTrack ()->AddRotKeyFrame (fFrame, rot, fStartFrame);
	}
}

void CAModelLoader::ParseObjectKeyScl (CAModelObject* pObject, IXMLDOMNode* pNode)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, "Key/Scale/Value");

	if (!pListChild)
		return;

	long nCount = 0;
	pListChild->get_length (&nCount);

	for (int i = 0; i != nCount; ++i)
	{
		pListChild->get_item (i, &pNodeChild);

		m_xmlFile.GetAttributeText (pNodeChild, "Frame", STR_BUFFER);

		float fFrame = (float)atof (STR_BUFFER);

		m_xmlFile.GetElementText (pNodeChild, STR_BUFFER);

		D3DXVECTOR3 scl = StrToVector3f (STR_BUFFER);
	
		pObject->GetTrack ()->AddSclKeyFrame (fFrame, scl);
	}
}

bool CAModelLoader::GetTextFromXML (const char* strElement, char* strText)
{
	IXMLDOMNodeList* pList = NULL;
	IXMLDOMNode* pNode = NULL;

	pList = m_xmlFile.FindElement (strElement);	

	if(pList && pList->get_item (0, &pNode) == S_OK)
	{
		m_xmlFile.GetElementText (pNode, strText);
		return true;
	}

	return false;
}

bool CAModelLoader::GetTextFromXML (IXMLDOMNode* pNode, const char* strElement, char* strText)
{
	IXMLDOMNodeList* pListChild = NULL;
	IXMLDOMNode* pNodeChild = NULL;

	pListChild = m_xmlFile.FindElement (pNode, strElement);
	if(pListChild && pListChild->get_item (0, &pNodeChild) == S_OK)
	{
		m_xmlFile.GetElementText (pNodeChild, strText);
		return true;
	}

	return false;
}

D3DCOLORVALUE CAModelLoader::StrToColor3f (char* strText)
{
	D3DCOLORVALUE val;
	char* sep = " [],\n";

	strtok (strText, sep);		// skip 'Color' string
	val.r = (float)atof (strtok (NULL, sep) ) / 255.0f;
	val.g = (float)atof (strtok (NULL, sep) ) / 255.0f;
	val.b = (float)atof (strtok (NULL, sep) ) / 255.0f;

	return val;	
}

D3DXVECTOR2 CAModelLoader::StrToVector2f (char* strText)
{
	D3DXVECTOR2 v;
	char* sep = " [],\n";

	v.x = (float)atof (strtok (strText, sep) );
	v.y = (float)atof (strtok (NULL, sep) );

	return v;
}

D3DXVECTOR3 CAModelLoader::StrToVector3f (char* strText)
{
	D3DXVECTOR3 v;
	char* sep = " [],\n";

	v.x = (float)atof (strtok (strText, sep) );
	v.y = (float)atof (strtok (NULL, sep) );
	v.z = (float)atof (strtok (NULL, sep) );

	return v;
}

D3DXVECTOR4 CAModelLoader::StrToVector4f (char* strText)
{
	D3DXVECTOR4 v;
	char* sep = " [],\n";

	v.x = (float)atof (strtok (strText, sep) );
	v.y = (float)atof (strtok (NULL, sep) );
	v.z = (float)atof (strtok (NULL, sep) );
	v.w = (float)atof (strtok (NULL, sep) );

	return v;
}

void CAModelLoader::StrToIndex (char* strText, int* nIndex)
{
	char* sep = " [],\n";

	nIndex [0] = atoi (strtok (strText, sep) );
	nIndex [1] = atoi (strtok (NULL, sep) );
	nIndex [2] = atoi (strtok (NULL, sep) );	
}