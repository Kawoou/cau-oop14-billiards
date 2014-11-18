#ifndef __APROJECT_OBJECT_MODEL_LOADER__
#define __APROJECT_OBJECT_MODEL_LOADER__

#pragma once

class CAMeshSWSkin;
class CAModelVIB;
class CAModelLoader
{
public:
	CAModelLoader (void);
	virtual ~CAModelLoader (void);

public:
	static CAModelLoader* GetSingleton (void);
	bool Load (CAModel* pModel, const char* strFileName, int nSkinningMode);

private:
	bool ParseInfo (CAModel* pModel);
	bool ParseMaterial (CAModel* pModel);
	bool ParseObject (CAModel* pModel, int nSkinningMode);

	CAModelObject* CreateModelObject (char* strClass, CAModel* pModel, bool bSkin, int nSkinningMode);

	void ParseVertex (CAModelVIB& modelVIB, IXMLDOMNode* pNode);
	void ParseNormal (CAModelVIB& modelVIB, IXMLDOMNode* pNode);
	void ParseUV (CAModelVIB& modelVIB, IXMLDOMNode* pNode);
	void ParseIndex (CAModelVIB& modelVIB, IXMLDOMNode* pNode);
	void ParseUVIndex (CAModelVIB& modelVIB, IXMLDOMNode* pNode);
	void ParseWeight (CAModelVIB& modelVIB, IXMLDOMNode* pNode);

	void SplitModelVIB (CAModelVIB& modelVIB);
	void OrganizeModelVIB (CAModelVIB& modelVIB);

	void OrganizeWeight (CAModelVIB& modelVIB);
	void OrganizeIndex (CAModelVIB& modelVIB);

	void ParseObjectParent (CAModelObject* pObject, IXMLDOMNode* pNode);
	void ParseObjectBBox (CAModelObject* pObject, IXMLDOMNode* pNode);
	void ParseObjectLocalTM (CAModelObject* pObject, IXMLDOMNode* pNode);
	void ParseObjectWorldTM (CAModelObject* pObject, IXMLDOMNode* pNode);
	void ParseObjectKeyPos (CAModelObject* pObject, IXMLDOMNode* pNode);
	void ParseObjectKeyRot (CAModelObject* pObject, IXMLDOMNode* pNode, float fStartFrame);
	void ParseObjectKeyScl (CAModelObject* pObject, IXMLDOMNode* pNode);

	bool GetTextFromXML (const char* strElement, char* strText);
	bool GetTextFromXML (IXMLDOMNode* pNode, const char* strElement, char* strText);

	D3DCOLORVALUE StrToColor3f (char* strText);
	D3DXVECTOR2 StrToVector2f (char* strText);
	D3DXVECTOR3 StrToVector3f (char* strText);
	D3DXVECTOR4 StrToVector4f (char* strText);
	void StrToIndex (char* strText, int* nIndex);

private:
	CAXMLFile			m_xmlFile;

	vector<CAModelVIB>	m_listModelVIB;
};

#endif