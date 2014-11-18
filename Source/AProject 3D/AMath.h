#ifndef __APROJECT_MATH__
#define __APROJECT_MATH__

#pragma once

using namespace std;

struct AColor
{
	AReal32 a;
	AReal32 r;
	AReal32 g;
	AReal32 b;

	AColor ()
	{
		a = r = g = b = 1.0f;
	};
	AColor (AReal32 fColor)
	{
		a = r = g = b = fColor;
	};
	AColor (AReal32 fR, AReal32 fG, AReal32 fB)
	{
		r = fR;
		g = fG;
		b = fB;
	};
	AColor (AReal32 fA, AReal32 fR, AReal32 fG, AReal32 fB)
	{
		a = fA;
		r = fR;
		g = fG;
		b = fB;
	};
	~AColor ()
	{
	};

	AUInt32 GetD3DColor (void);
};

struct AVector3
{
	AReal32 x;
	AReal32 y;
	AReal32 z;

	AVector3 (AReal32 fX, AReal32 fY, AReal32 fZ)
	{
		x = fX;
		y = fY;
		z = fZ;
	};

	AVector3 (AReal32 fValue)
	{
		x = y = z = fValue;
	};

	AVector3 ()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	};

	AVector3 (D3DXVECTOR3 vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}

	bool operator== (const AVector3 &fValue) const;
	bool operator!= (const AVector3 &fValue) const;

	AVector3 operator- () const;
	AVector3 operator- (const AVector3 &fValue) const;
	AVector3 operator+ (const AVector3 &fValue) const;
	AVector3 operator/ (AReal32 divider) const;
	AVector3 operator* (AReal32 scaleFactor) const;									//!< 내적 연산

	AVector3& operator+= (const AVector3 &fValue);
	AVector3& operator-= (const AVector3 &fValue);
	AVector3& operator*= (AReal32 f);												//!< 내적 연산
	AVector3& operator/= (AReal32 f);

	AVector3 operator* (AVector3 &fValue);											//!< 외적 연산
	AVector3& operator= (AVector3 &fValue);

public:
	AReal32 Length (void);
	AReal32 LengthSquared (void);

	static AReal32 Distance (const AVector3& fOne, const AVector3& fTwo);
	static AReal32 DistanceSquared (const AVector3& fOne, const AVector3& fTwo);
	static AReal32 Dot (const AVector3& fOne, const AVector3& fTwo);				//!< 내적 연산
	AVector3 Dot (AVector3& fValue);												//!< 외적 연산
	void Copy (const AVector3& fValue);
	AVector3 Normalize (void);
	D3DXVECTOR3 D3DXVector(void);
	static AVector3 Normalize (const AVector3& fValue);
};

RECT CreateRECT (int nX, int nY, int nW, int nH);

int	Log2 (int n);
int	Pow2 (int n);
bool IsInRect (RECT* rc, POINT pt);

D3DXVECTOR3 WorldToClient (D3DXVECTOR3 vPosition);

#endif