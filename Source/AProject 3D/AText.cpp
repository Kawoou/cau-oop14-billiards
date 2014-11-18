#include "Include.h"

CAText::CAText (const WCHAR* pFaceName, int nSize, bool bBold, bool bItalic, DWORD dwQuality)
{
	m_hFont2D = NULL;

	m_pFont2D = NULL;
	m_pFont3D = NULL;
	m_pText = NULL;
	m_cColor = AColor (1.0f);
	m_bOrtho2D = true;

	D3DXCreateSprite (APROJECT_WINDOW->GetD3DDevice (), &m_pSprite);

	m_descTextInfo.CharSet = DEFAULT_CHARSET;
	wcscpy (m_descTextInfo.FaceName, pFaceName);

	m_descTextInfo.Height = MulDiv ( (!nSize) ? 2000 : nSize, GetDeviceCaps (GetDC (APROJECT_WINDOW->GetHWND () ), LOGPIXELSY), 72);
	m_descTextInfo.Width = 0;
	m_descTextInfo.Weight = bBold ? 800 : 100;
	m_descTextInfo.Quality = dwQuality;
	m_descTextInfo.MipLevels = 1;
	m_descTextInfo.Italic = bItalic;
	m_descTextInfo.OutputPrecision = OUT_DEFAULT_PRECIS;
	m_descTextInfo.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	m_fDeviation = 200.0f;
	m_fExtrusion = 3.0f;

	m_hFont2D = CreateFont (m_descTextInfo.Height, m_descTextInfo.Width, 0, 0, m_descTextInfo.Weight, m_descTextInfo.Italic, false, false,
			m_descTextInfo.CharSet, m_descTextInfo.OutputPrecision, CLIP_DEFAULT_PRECIS, m_descTextInfo.Quality, m_descTextInfo.PitchAndFamily, m_descTextInfo.FaceName);

	D3DXCreateFontIndirect (APROJECT_WINDOW->GetD3DDevice (), &m_descTextInfo, &m_pFont2D);
}

CAText::~CAText (void)
{
	SAFE_RELEASE (m_pSprite);
	DeleteObject (m_hFont2D);

	SAFE_RELEASE (m_pFont2D);
	SAFE_RELEASE (m_pFont3D);
	SAFE_DELETE (m_pText);
}

void CAText::SetAlign (FontAlignType aAlign)
{
	m_aAlign = aAlign;
}

void CAText::SetColor (AColor cColor)
{
	m_cColor = cColor;
}

void CAText::SetOrtho2D (bool bState)
{
	m_bOrtho2D = bState;
}

void CAText::SetText3D (float fDeviation, float fExtrusion)
{
	m_fDeviation = fDeviation;
	m_fExtrusion = fExtrusion;

	Create (NULL);
}

CAText::FontAlignType CAText::GetAlign (void)
{
	return m_aAlign;
}

AColor CAText::GetColor (void)
{
	return m_cColor;
}

bool CAText::GetOrtho2D (void)
{
	return m_bOrtho2D;
}

void CAText::GetText3D (float& fDeviation, float& fExtrusion)
{
	fDeviation = m_fDeviation;
	fExtrusion = m_fExtrusion;
}

HRESULT CAText::Create (const WCHAR* pText)
{
	if (m_pText && wcscmp (m_pText, pText) == 0)
		return false;

	if (pText != NULL)
	{
		int len = wcslen (pText) + 1;
		SAFE_DELETE (m_pText);
		m_pText = new WCHAR [len];
		wcscpy (m_pText, pText);
	}

	if (!m_bOrtho2D)
	{
		SAFE_DELETE (m_pFont3D);

		HDC hdc = CreateCompatibleDC (NULL);

		// Save the old font
		HFONT hFontOld = (HFONT)SelectObject (hdc, m_hFont2D);

		// Create the text mesh
		if (FAILED (D3DXCreateText (APROJECT_WINDOW->GetD3DDevice (), hdc, m_pText, m_fDeviation, m_fExtrusion, &m_pFont3D, NULL, NULL) ) )
		{
			DebugLog ("Create(Create the text mesh) 실패!");
			DebugConsoleLog (DebugError, "CAText: Create(Create the text mesh) 실패!");
			return false;
		}

		// Restore the old font and clean up 
		SelectObject (hdc, hFontOld);
		DeleteDC (hdc);
	}

	return true;
}

void CAText::Reset (void)
{
	SAFE_RELEASE (m_pSprite);
	SAFE_DELETE (m_pText);
	SAFE_RELEASE (m_pFont2D);
	SAFE_RELEASE (m_pFont3D);

	D3DXCreateSprite (APROJECT_WINDOW->GetD3DDevice (), &m_pSprite);

	D3DXCreateFontIndirect (APROJECT_WINDOW->GetD3DDevice (), &m_descTextInfo, &m_pFont2D);

	Create (NULL);
}

void CAText::Update (void)
{
}

void CAText::Render (void)
{
	if (m_bOrtho2D)
	{
		DWORD format = DT_EXPANDTABS;
		if (m_vSize.x == 0 || m_vSize.y == 0)
			format |= DT_NOCLIP;
		else
		{
			format |= DT_WORDBREAK;
			switch (m_aAlign)
			{
			case AA_LEFT:
				format |= DT_LEFT;
				break;
			case AA_CENTER:
				format |= DT_CENTER;
				break;
			case AA_RIGHT:
				format |= DT_RIGHT;
				break;
			case AA_TOPLEFT:
				format |= DT_LEFT | DT_TOP;
				break;
			case AA_TOPCENTER:
				format |= DT_CENTER | DT_TOP;
				break;
			case AA_TOPRIGHT:
				format |= DT_RIGHT | DT_TOP;
				break;
			case AA_BOTTOMLEFT:
				format |= DT_LEFT | DT_BOTTOM;
				break;
			case AA_BOTTOMCENTER:
				format |= DT_CENTER | DT_BOTTOM;
				break;
			case AA_BOTTOMRIGHT:
				format |= DT_RIGHT | DT_BOTTOM;
				break;
			}
		}

		// Draw Text
		m_pSprite->Begin (D3DXSPRITE_ALPHABLEND);
		{
			RECT rtPos = CreateRECT (m_vPosition.x, m_vPosition.y, 0, 0);
			
			m_pFont2D->DrawTextW (m_pSprite, m_pText, -1, &rtPos, DT_CALCRECT, 0x00000000);
			m_pFont2D->DrawTextW (m_pSprite, m_pText, -1, &rtPos, format, m_cColor.GetD3DColor () );
		}
		m_pSprite->End ();
	}
	else
	{
		m_pFont3D->DrawSubset (0);
	}
}