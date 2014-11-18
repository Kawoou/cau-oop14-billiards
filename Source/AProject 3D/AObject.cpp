#include "AWindow.h"

CAObject::CAObject ()
{
	m_vPosition = AVector3 (0.0f);
	m_vSize = AVector3 (1.0f);
	m_vOffset = AVector3 (0.0f);
	m_fRotX = 0.0f;
	m_fRotY = 0.0f;
	m_fRotZ = 0.0f;
	m_bOrtho2D = false;
	m_bEnable = false;
}

CAObject::~CAObject ()
{
}

void CAObject::MoveX (AReal32 fX)
{
	m_vPosition.x += fX;
}

void CAObject::MoveY (AReal32 fY)
{
	m_vPosition.y += fY;
}

void CAObject::MoveZ (AReal32 fZ)
{
	m_vPosition.z += fZ;
}

void CAObject::MoveRotX (AReal32 fRotate)
{
	m_fRotX += fRotate;
}

void CAObject::MoveRotY (AReal32 fRotate)
{
	m_fRotY += fRotate;
}

void CAObject::MoveRotZ (AReal32 fRotate)
{
	m_fRotZ += fRotate;
}

void CAObject::SetName (const WCHAR* pTitle)
{
	wsprintf (m_wsName, L"%s", pTitle);
}

void CAObject::SetLayer (AInt nLayer)
{
	m_nLayer = nLayer;
}

void CAObject::SetPos (AReal32 fX, AReal32 fY)
{
	m_vPosition.x = fX;
	m_vPosition.y = fY;
}

void CAObject::SetPos (AReal32 fX, AReal32 fY, AReal32 fZ)
{
	SetPos (fX, fY);
	m_vPosition.z = fZ;

	if (m_vPosition.z != fZ)
		APROJECT_WINDOW->GetCurScene ()->ChangeInPosZ (m_wsName, fZ);
}

void CAObject::SetPos (AVector3 vPos)
{
	SetPos (vPos.x, vPos.y);
	m_vPosition.z = vPos.z;

	if (m_vPosition.z != vPos.z)
		APROJECT_WINDOW->GetCurScene ()->ChangeInPosZ (m_wsName, vPos.z);
}

void CAObject::SetRotX (AReal32 fRotate)
{
	m_fRotX = fRotate;
}

void CAObject::SetRotY (AReal32 fRotate)
{
	m_fRotY = fRotate;
}

void CAObject::SetRotZ (AReal32 fRotate)
{
	m_fRotZ = fRotate;
}

void CAObject::SetSize (AReal32 fSizeX, AReal32 fSizeY)
{
	m_vSize.x = fSizeX;
	m_vSize.y = fSizeY;
}

void CAObject::SetSize (AReal32 fSizeX, AReal32 fSizeY, AReal32 fSizeZ)
{
	m_vSize = AVector3 (fSizeX, fSizeY, fSizeZ);
}

void CAObject::SetSize (AVector3 vSize)
{
	m_vSize = vSize;
}

void CAObject::SetOffset (AReal32 fX, AReal32 fY)
{
	m_vOffset.x = fX;
	m_vOffset.y = fY;
}

void CAObject::SetOffset (AReal32 fX, AReal32 fY, AReal32 fZ)
{
	m_vOffset = AVector3 (fX, fY, fZ);
}

void CAObject::SetOffset (AVector3 vPos)
{
	m_vOffset = vPos;
}

void CAObject::SetOrtho2D(bool bState)
{
	m_bOrtho2D = bState;
}

void CAObject::SetEnable (bool bEnable)
{
	m_bEnable = bEnable;
}

WCHAR* CAObject::GetName (void)
{
	return m_wsName;
}

AInt CAObject::GetLayer (void)
{
	return m_nLayer;
}

AVector3 CAObject::GetPos (void)
{
	return m_vPosition;
}

AReal32 CAObject::GetRotX (void)
{
	return m_fRotX;
}

AReal32 CAObject::GetRotY (void)
{
	return m_fRotY;
}

AReal32 CAObject::GetRotZ (void)
{
	return m_fRotZ;
}

AVector3 CAObject::GetSize (void)
{
	return m_vSize;
}

AVector3 CAObject::GetOffset (void)
{
	return m_vOffset;
}

bool CAObject::GetOrtho2D(void)
{
	return m_bOrtho2D;
}

bool CAObject::GetEnable (void)
{
	return m_bEnable;
}

void CAObject::Reset (void)
{
}

void CAObject::Update (void)
{
}

void CAObject::Render (void)
{
}