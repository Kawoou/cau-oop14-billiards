#ifndef __APROJECT_XML_PARSAR__
#define __APROJECT_XML_PARSAR__

#pragma once

class CAXMLFile
{
private:
	IXMLDOMDocument* m_pXMLDoc;

public:
	std::string m_strDir;

public:
	CAXMLFile (void);
	CAXMLFile (const char* strFileName);
	~CAXMLFile (void);

public:
	bool Open (const char* strFileName);
	void Close ();

	IXMLDOMNodeList* FindElement (const char* strElement);
	IXMLDOMNodeList* FindElement (IXMLDOMNode* pNode, const char* strElement);

	bool GetElementText (IXMLDOMNode* pNode, char* strRet);
	bool GetAttributeText (IXMLDOMNode* pNode, char* strAttrName, char* strRet);
};

#endif