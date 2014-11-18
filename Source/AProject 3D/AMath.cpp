#include "AWindow.h"

AUInt32 AColor::GetD3DColor (void)
{
	return D3DCOLOR_ARGB ( (AInt)(255 * a), (AInt)(255 * r), (AInt)(255 * g), (AInt)(255 * b) );
}

bool AVector3::operator ==(const AVector3 &fValue) const
{
	return (x == fValue.x && y == fValue.y && z == fValue.z);
}

bool AVector3::operator !=(const AVector3 &fValue) const
{
	return !(*this == fValue);
}

AVector3 AVector3::operator- () const
{
	return AVector3 (-x, -y, -z);
}

AVector3 AVector3::operator- (const AVector3 &fValue) const
{
	return AVector3 (x - fValue.x, y - fValue.y, z - fValue.z);
}

AVector3 AVector3::operator+ (const AVector3 &fValue) const
{
	return AVector3 (x + fValue.x, y + fValue.y, z + fValue.z);
}

AVector3 AVector3::operator/ (AReal32 divider) const
{
	return AVector3 (x / divider, y / divider, z / divider);
}

AVector3 AVector3::operator* (AReal32 scaleFactor) const
{
	return AVector3 (x * scaleFactor, y * scaleFactor, z * scaleFactor);
}

AVector3& AVector3::operator+= (const AVector3 &fValue)
{
	x += fValue.x;
	y += fValue.y;
	z += fValue.z;

	return *this;
}

AVector3& AVector3::operator-= (const AVector3 &fValue)
{
	x -= fValue.x;
	y -= fValue.y;
	z -= fValue.z;

	return *this;
}

AVector3& AVector3::operator*= (AReal32 f)
{
	x *= f;
	y *= f;
	z *= f;

	return *this;
}

AVector3& AVector3::operator/= (AReal32 f)
{
	x /= f;
	y /= f;
	z /= f;

	return *this;
}

AVector3 AVector3::operator* (AVector3 &fValue)
{
	AVector3 vTmp;

	vTmp.x = (y * fValue.z) - (z * fValue.y);
	vTmp.y = (z * fValue.x) - (x * fValue.z);
	vTmp.z = (x * fValue.y) - (y * fValue.x);

	return vTmp;
}

AVector3& AVector3::operator= (AVector3 &fValue)
{
	x = fValue.x;
	y = fValue.y;
	z = fValue.z;

	return *this;
}

AReal32 AVector3::Length (void)
{
	return sqrt (LengthSquared () );
}

AReal32 AVector3::LengthSquared (void)
{
	return ( (x * x) + (y * y) + (z * z) );
}

AReal32 AVector3::Distance (const AVector3& fOne, const AVector3& fTwo)
{
	return sqrt (DistanceSquared (fOne, fTwo) );
}

AReal32 AVector3::DistanceSquared (const AVector3& fOne, const AVector3& fTwo)
{
	return (pow (fOne.x - fTwo.x, 2) + pow (fOne.y - fTwo.y, 2) + pow (fOne.z - fTwo.z, 2) );
}

AReal32 AVector3::Dot (const AVector3& fOne, const AVector3& fTwo)
{
	return (fOne.x * fTwo.x + fOne.y * fTwo.y + fOne.z * fTwo.z);
}

AVector3 AVector3::Dot (AVector3& fValue)
{
	AVector3 vTmp;

	vTmp.x = (y * fValue.z) - (z * fValue.y);
	vTmp.y = (z * fValue.x) - (x * fValue.z);
	vTmp.z = (x * fValue.y) - (y * fValue.x);

	return vTmp;
}

void AVector3::Copy (const AVector3& fValue)
{
	x = fValue.x;
	y = fValue.y;
	z = fValue.z;
}

AVector3 AVector3::Normalize (void)
{
	AReal32 d = Length ();
	if (d == 0)
		return AVector3(0.0f);
	else
		return AVector3 (x / d, y / d, z / d);
}

AVector3 AVector3::Normalize (const AVector3& fValue)
{
	D3DXVECTOR3 v (fValue.x, fValue.y, fValue.z), w;
	D3DXVec3Normalize (&w, &v);
	AVector3 vTmp (w.x, w.y, w.z);

	return vTmp;
}

D3DXVECTOR3 AVector3::D3DXVector(void)
{
	return D3DXVECTOR3(x, y, z);
}

RECT CreateRECT (int nX, int nY, int nW, int nH)
{
	RECT rt;

	rt.left = nX;
	rt.top = nY;
	rt.right = nX + nW;
	rt.bottom = nY + nH;

	return rt;
}

int	Log2 (int n)
{
	for (int i = 1; i < 64; i ++)
	{
		n = n >> 1;
		if (n == 1)
			return i;
	}

	return 1;
}

int	Pow2 (int n)
{
	int val = 1;
	val = val << n;
	return val;
}

bool IsInRect (RECT* rc, POINT pt)
{
	if ( (rc->left <= pt.x) && (pt.x <= rc->right) &&
		(rc->bottom <= pt.y) && (pt.y <= rc->top) )
		return true;

	return false;
}

D3DXVECTOR3 WorldToClient (D3DXVECTOR3 vPosition)
{
	// 변환할 공간 좌표가 있는 viewport 얻음.
	D3DVIEWPORT9 vp;
	APROJECT_WINDOW->GetD3DDevice ()->GetViewport (&vp);   
 
	// View 변환 행렬과 Projection 변환 행렬의 곱.
	D3DXMATRIX ViewProjMatrix = (*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () ) * (*APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix () );
 
	// 좌표 변환.
	D3DXVec3TransformCoord (&vPosition, &vPosition, &ViewProjMatrix);
 
	// Projection 변환에 의해 변환된 좌표는 -1.0f ~ 1.0f 사이의 값.
	// -1.0f ~ 1.0f 범위를 0.0f ~ 1.0f (비율) 범위로 변경.
	// position.x = ( position.x + 1.0f ) / 2.0f;
	// position.y = ( 2.0f - position.y + 1.0f ) / 2.0f;    // Y 좌표의 방향이 반대임을 고려.
 
	// Viewport 크기와 비율을 계산하여 client 좌표를 얻음.
	// position.x = vp.Width * position.x + vp.X;
	// position.y = vp.Height * position.y + vp.Y;
	return D3DXVECTOR3 (vp.Width * (vPosition.x + 1.0f) / 2.0f + vp.X, vp.Height * (2.0f - (vPosition.y + 1.0f) ) / 2.0f + vp.Y, 0.0f );
}