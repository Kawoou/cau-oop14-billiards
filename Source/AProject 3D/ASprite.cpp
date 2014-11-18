#include "AWindow.h"

CASprite::CASprite (void)
	:CAObject ()
{
	m_bEnable = true;
}

CASprite::~CASprite (void)
{
	for (AUInt i = 0; i < m_vList.size (); i ++)
		SAFE_DELETE (m_vList [i]);
	m_vList.clear ();

	DebugConsoleLog (DebugClear, "CASprite: Sprite Pointer Destroy!");
}

AInt CASprite::GetTextureCount (void)
{
	return m_vList.size ();
}

CATexture* CASprite::Trans (AInt nCount)
{
	if (0 <= nCount && nCount < m_vList.size () )
		return m_vList [nCount];
	else
		return NULL;
}

AInt CASprite::Commit (CATexture* pObject)
{
	m_vList.push_back (pObject);

	return m_vList.size () - 1;
}

HRESULT CASprite::Remove (AInt nCount)
{
	m_vList.erase (m_vList.begin () + nCount);

	return AE_SUCCESS;
}

void CASprite::Reset (void)
{
	for (AUInt i = 0; i < m_vList.size (); i ++)
		m_vList [i]->Reset ();
}

void CASprite::Update (void)
{
	for (AUInt i = 0; i < m_vList.size (); i ++)
		m_vList [i]->Update ();
}

void CASprite::Render (void)
{
	for (AUInt i = 0; i < m_vList.size (); i ++)
		m_vList [i]->Render (NULL, NULL, -1);
}