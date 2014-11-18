#ifndef __APROJECT_TEXT__
#define __APROJECT_TEXT__

#pragma once

class CAText : public CAObject
{
public:
	enum FontAlignType {AA_LEFT, AA_CENTER, AA_RIGHT, AA_TOPLEFT, AA_TOPCENTER, AA_TOPRIGHT, AA_BOTTOMLEFT, AA_BOTTOMCENTER, AA_BOTTOMRIGHT};

private:
	D3DXFONT_DESC			m_descTextInfo;
	LPD3DXSPRITE			m_pSprite;
	HFONT					m_hFont2D;

private:
	LPD3DXFONT				m_pFont2D;
	LPD3DXMESH				m_pFont3D;
	WCHAR*					m_pText;

private:
	FontAlignType			m_aAlign;
	AColor					m_cColor;
	bool					m_bOrtho2D;
	float					m_fDeviation;
	float					m_fExtrusion;

public:
	CAText (const WCHAR* pFaceName, int nSize, bool bBold = false, bool bItalic = false, DWORD dwQuality = DEFAULT_QUALITY);
	virtual ~CAText (void);

public:
	void SetAlign (FontAlignType aAlign);
	void SetColor (AColor cColor);
	void SetOrtho2D (bool bState);
	void SetText3D (float fDeviation, float fExtrusion);

	FontAlignType GetAlign (void);
	AColor GetColor (void);
	bool GetOrtho2D (void);
	void GetText3D (float& fDeviation, float& fExtrusion);

public:
	HRESULT Create (const WCHAR* pText);
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);
};

#endif