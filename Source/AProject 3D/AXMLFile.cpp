#include "Include.h"

CAXMLFile::CAXMLFile (void)
{
	m_pXMLDoc = NULL;

	CoInitialize (NULL);
}

CAXMLFile::CAXMLFile (const char* strFileName)
{
	m_pXMLDoc = NULL;

	Open (strFileName);
}

CAXMLFile::~CAXMLFile (void)
{
	Close ();
}

bool CAXMLFile::Open (const char* strFileName)
{
	Close ();

	m_strDir = strFileName;
	for (int i = strlen (m_strDir.data () ) - 1; i >= 0; --i)
	{
		if (m_strDir.data () [i] == '/' || m_strDir.data () [i] == '\\')
			break;

		m_strDir.pop_back ();
	}

	if (CoCreateInstance (CLSID_DOMDocument, NULL, CLSCTX_ALL, IID_IXMLDOMDocument, (void**)&m_pXMLDoc) != S_OK)
		return false;

	if (CreateFileA (strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) == INVALID_HANDLE_VALUE)
	{
		Close ();
		return false;
	}

	if (m_pXMLDoc->put_async (FALSE) != S_OK)
	{
		Close ();
		return false;
	}

	_bstr_t varString = (const char*)strFileName;
	VARIANT path;
	path.vt = VT_BSTR;
	path.bstrVal = varString;

	short sResult = 0;
	if (m_pXMLDoc->load (path, &sResult) != S_OK)
	{
		Close ();
		return false;
	}

	return true;
}

void CAXMLFile::Close()
{
	SAFE_RELEASE (m_pXMLDoc);
}

IXMLDOMNodeList* CAXMLFile::FindElement (const char* strElement)
{
	if (m_pXMLDoc == NULL)
		return NULL;

	IXMLDOMNodeList* pNodeList = NULL;
	
	_bstr_t bstrPath = strElement;

	if (m_pXMLDoc->selectNodes (bstrPath, &pNodeList) != S_OK)
		return NULL;

	return pNodeList;
}

IXMLDOMNodeList* CAXMLFile::FindElement (IXMLDOMNode* pNode, const char* strElement)
{
	if (pNode == NULL)
		return NULL;

	IXMLDOMNodeList* pNodeList = NULL;
	
	_bstr_t bstrPath = strElement;

	if (pNode->selectNodes (bstrPath, &pNodeList) != S_OK)
		return NULL;

	return pNodeList;
}

bool CAXMLFile::GetElementText (IXMLDOMNode* pNode, char* strRet)
{
	BSTR bstr = NULL;
	
	if (pNode->get_text (&bstr) != S_OK)
		return false;

	strcpy (strRet, (const char*)_bstr_t (bstr,FALSE) );

	return true;
}

bool CAXMLFile::GetAttributeText (IXMLDOMNode* pNode, char* strAttrName, char* strRet)
{
	wchar_t					wstrAttr [128];
	IXMLDOMNode*			pAttrNode = NULL;
	IXMLDOMNamedNodeMap*	pMap = NULL;
	VARIANT					varValue;

	mbstowcs (wstrAttr, strAttrName, 128);

	if (pNode->get_attributes (&pMap) != S_OK)
		return false;

	if (pMap->getNamedItem (wstrAttr, &pAttrNode) != S_OK)
	{
		SAFE_RELEASE (pMap);
		return false;
	}

	if (pAttrNode->get_nodeValue (&varValue) != S_OK)
	{
		SAFE_RELEASE (pAttrNode);
		SAFE_RELEASE (pMap);
		return false;
	}

	strcpy (strRet, (const char*)_bstr_t (varValue.bstrVal, FALSE) );

	SAFE_RELEASE (pAttrNode);
	SAFE_RELEASE (pMap);
	return true;
}