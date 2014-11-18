#ifndef __APROJECT_SPRITE__
#define __APROJECT_SPRITE__

#pragma once

class CASprite : public CAObject
{
private:
	vector<CATexture*>	m_vList;

public:
	CASprite (void);
	virtual ~CASprite (void);

public:
	AInt GetTextureCount (void);

	CATexture* Trans (AInt nCount);
	AInt Commit (CATexture* pObject);
	HRESULT Remove (AInt nCount);

public:
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);
};

#endif