#ifndef __APROJECT_OBJECT__
#define __APROJECT_OBJECT__

#pragma once

class CAObject
{
protected:
	WCHAR					m_wsName [256];
	AInt					m_nLayer;

protected:
	AVector3				m_vSize;
	AVector3				m_vPosition;
	AVector3				m_vOffset;

	AReal32					m_fRotX;
	AReal32					m_fRotY;
	AReal32					m_fRotZ;

	bool					m_bOrtho2D;
	bool					m_bEnable;

public:
	CAObject ();
	virtual ~CAObject ();

public:
	void MoveX (AReal32 fX);
	void MoveY (AReal32 fY);
	void MoveZ (AReal32 fZ);
	void MoveRotX (AReal32 fRotate);
	void MoveRotY (AReal32 fRotate);
	void MoveRotZ (AReal32 fRotate);

	void SetName (const WCHAR* pTitle);
	void SetLayer (AInt nLayer);
	void SetPos (AReal32 fX, AReal32 fY);
	void SetPos (AReal32 fX, AReal32 fY, AReal32 fZ);
	void SetPos (AVector3 vPos);
	void SetSize (AReal32 fSizeX, AReal32 fSizeY);
	void SetSize (AReal32 fSizeX, AReal32 fSizeY, AReal32 fSizeZ);
	void SetSize (AVector3 vSize);
	void SetOffset (AReal32 fX, AReal32 fY);
	void SetOffset (AReal32 fX, AReal32 fY, AReal32 fZ);
	void SetOffset (AVector3 vPos);
	void SetRotX (AReal32 fRotate);
	void SetRotY (AReal32 fRotate);
	void SetRotZ (AReal32 fRotate);
	void SetOrtho2D(bool bState);
	void SetEnable (bool bEnable);

	WCHAR* GetName (void);
	AInt GetLayer (void);
	AVector3 GetPos (void);
	AVector3 GetSize (void);
	AVector3 GetOffset (void);
	AReal32 GetRotX (void);
	AReal32 GetRotY (void);
	AReal32 GetRotZ (void);
	bool GetOrtho2D(void);
	bool GetEnable (void);
	
public:
	virtual void Reset (void);
	virtual void Update (void);
	virtual void Render (void);
};

#endif