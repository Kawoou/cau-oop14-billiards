#include "AWindow.h"

string IntToString(int nInteger)
{
	char number[50];
	string strData;
	itoa(nInteger, number, 10);

	strData = number;

	return strData;
}

wstring IntToWstring(int nInteger)
{
	char c_number[50];
	wchar_t number[50];
	itoa(nInteger, c_number, 10);

	wstring strData;
	ConvertToWChar(c_number, number);

	strData = number;

	return strData;
}

AUInt VKToDIK(AUInt cVk)
{
	AUInt tmp[] = {
		0, false, false, false, false, false, false, false, DIK_BACK, DIK_TAB, false, false, false, DIK_RETURN, false, false,												// 0x00-0x0F
		DIK_LSHIFT, DIK_LCONTROL, DIK_LMENU, DIK_PAUSE, DIK_CAPITAL, DIK_KANA, false, false, false, DIK_KANJI, false, DIK_ESCAPE, DIK_CONVERT, DIK_NOCONVERT, false, false,		// 0x10-0x1F
		DIK_SPACE, DIK_PRIOR, DIK_NEXT, DIK_END, DIK_HOME, DIK_LEFT, DIK_UP, DIK_RIGHT, DIK_DOWN, DIK_MEDIASELECT, false, false, DIK_SYSRQ, DIK_INSERT, DIK_DELETE, false,	// 0x20-0x2F
		DIK_0, DIK_1, DIK_2, DIK_3, DIK_4, DIK_5, DIK_6, DIK_7, DIK_8, DIK_9, false, false, false, false, false, false,														// 0x30-0x3F
		false, DIK_A, DIK_B, DIK_C, DIK_D, DIK_E, DIK_F, DIK_G, DIK_H, DIK_I, DIK_J, DIK_K, DIK_L, DIK_N, DIK_M, DIK_O,														// 0x40-0x4F
		DIK_P, DIK_Q, DIK_R, DIK_S, DIK_T, DIK_U, DIK_V, DIK_W, DIK_X, DIK_Y, DIK_Z, DIK_LWIN, DIK_RWIN, DIK_APPS, false, DIK_SLEEP,										// 0x50-0x5F
		DIK_NUMPAD0, DIK_NUMPAD1, DIK_NUMPAD2, DIK_NUMPAD3, DIK_NUMPAD4, DIK_NUMPAD5, DIK_NUMPAD6, DIK_NUMPAD7, DIK_NUMPAD8, DIK_NUMPAD9, DIK_MULTIPLY, DIK_ADD, false, DIK_SUBTRACT, DIK_DECIMAL, DIK_DIVIDE, // 0x60-0x6F
		DIK_F1, DIK_F2, DIK_F3, DIK_F4, DIK_F5, DIK_F6, DIK_F7, DIK_F8, DIK_F9, DIK_F10, DIK_F11, DIK_F12, DIK_F13, DIK_F14, DIK_F15, false,								// 0x70-0x7F
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,										// 0x80-0x8F
		DIK_NUMLOCK, DIK_SCROLL, DIK_NUMPADEQUALS, false, false, false, false, false, false, false, false, false, false, false, false, false,								// 0x90-0x9F
		DIK_LSHIFT, DIK_RSHIFT, DIK_LCONTROL, DIK_RCONTROL, DIK_LMENU, DIK_RMENU, DIK_WEBBACK, DIK_WEBFORWARD, DIK_WEBREFRESH, DIK_WEBSTOP, DIK_WEBSEARCH, DIK_WEBFAVORITES, DIK_WEBHOME, DIK_MUTE, DIK_VOLUMEDOWN, DIK_VOLUMEUP,	// 0xA0-0xAF
		DIK_NEXTTRACK, DIK_PREVTRACK, DIK_MEDIASTOP, DIK_PLAYPAUSE, DIK_MAIL, DIK_MEDIASELECT, false, false, false, false, DIK_SEMICOLON, DIK_ADD, DIK_COMMA, DIK_MINUS, DIK_PERIOD, DIK_SLASH,	// 0xB0-0x0F
		DIK_GRAVE, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,													// 0xC0-0xCF
		false, false, false, false, false, false, false, false, false, false, false, DIK_LBRACKET, false, DIK_RBRACKET, DIK_APOSTROPHE, false,								// 0xD0-0xDF
		false, DIK_AX, DIK_OEM_102, false, false, false, false, false, false, false, false, false, false, false, false, false,												// 0xE0-0xEF
		false, false, false, false, false, false, DIK_AT, FALSE, false, false, false, false, false, false, false, 0,														// 0xF0-0xFF
	};

	return tmp[cVk];
}

AUInt DIKToVK(AUInt cDik)
{
	AUInt i = 0;
	for (i = 0; i < 256; i++)
	if (VKToDIK(i) == cDik)
		break;

	return i;
}

void ConvertToChar (WCHAR* pWChar, char* pChar)
{
	int strSize = WideCharToMultiByte (CP_ACP, 0, pWChar, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte (CP_ACP, 0, pWChar, -1, pChar, strSize, 0, 0);
}

void ConvertToWChar (char* pChar, WCHAR* pWChar)
{
	int strSize = MultiByteToWideChar (CP_ACP, 0, pChar, -1, NULL, NULL);
	MultiByteToWideChar (CP_ACP, 0, pChar, strlen (pChar) + 1, pWChar, strSize);
}

void SaveScreenShot (PTCHAR filename, int w, int h)
{
	//임시로 사용할 Surface
	IDirect3DSurface9* BackBuff;

	// 시스템 메모리에 정해진 크기의 버퍼를 만듬.
	// 파D3DXIFF_PNG일로 저장하려면 당연히 시스템 메모리에 이미지가 있어야 되겠죠.
	APROJECT_WINDOW->GetD3DDevice ()->CreateOffscreenPlainSurface (w, h, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &BackBuff, NULL);

	// 백버퍼의 얻어옴
	if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuff) ) )
	{
		BackBuff->Release();
		return;
	}

	//파일로 저장
	//2번째 인자는 저장할 이미지 타입을 넘겨주는 겁니다.
	//bmp, dds는 되는데..아직 tga나 jpg는 제대로 지원을 못합니다.
	D3DXSaveSurfaceToFile (filename, D3DXIFF_PNG, BackBuff, NULL, NULL);

	//풀어줌
	BackBuff->Release();
}

bool PointToRectangleCollision(const AVector3& v, const RECT& rt)
{
	if (rt.left <= v.x && v.x <= rt.right &&
		rt.top <= v.y && v.y <= rt.bottom)
		return true;

	return false;
}

bool LineToLineCollision(const AVector3& v1, const AVector3& v2, const AVector3& w1, const AVector3& w2)
{
	float denominator = ((v2.x - v1.x) * (w2.z - w1.z)) - ((v2.z - v1.z) * (w2.x - w1.x));
	float numerator1 = ((v1.z - w1.z) * (w2.x - w1.x)) - ((v1.x - w1.x) * (w2.z - w1.z));
	float numerator2 = ((v1.z - w1.z) * (v2.x - v1.x)) - ((v1.x - w1.x) * (v2.z - v1.z));

	if (denominator == 0)
		return numerator1 == 0 && numerator2 == 0;

	float r = numerator1 / denominator;
	float s = numerator2 / denominator;

	return (r >= 0 && r <= 1) && (s >= 0 && s <= 1);
}

bool PolyToPolyCollision(const AVector3* pPoly1, const AVector3* pPoly2, const AUInt32 nPoly1, const AUInt32 nPoly2)
{
	bool intersect = true;
	AVector3 edge;

	for (unsigned int edgeIndex = 0; edgeIndex < nPoly1 + nPoly2; edgeIndex++)
	{
		if (edgeIndex < nPoly1)
			edge = (AVector3)pPoly1[edgeIndex];
		else
			edge = (AVector3)pPoly2[edgeIndex - nPoly1];

		float minA = 0; float minB = 0;
		float maxA = 0; float maxB = 0;
		AVector3 axis = AVector3(-edge.z, edge.y, edge.x).Normalize();

		// Project Polygon A
		float dotProduct = AVector3::Dot(axis, (AVector3)pPoly1[0]);
		minA = dotProduct;
		maxA = dotProduct;
		for (unsigned int i = 0; i < nPoly1; i++)
		{
			dotProduct = AVector3::Dot(pPoly1[i], axis);
			if (dotProduct < minA)
				minA = dotProduct;
			else if (dotProduct> maxA)
				maxA = dotProduct;
		}
		
		// Project Polygon B
		dotProduct = AVector3::Dot(axis, (AVector3)pPoly2[0]);
		minB = dotProduct;
		maxB = dotProduct;
		for (unsigned int i = 0; i < nPoly2; i++)
		{
			dotProduct = AVector3::Dot(pPoly2[i], axis);
			if (dotProduct < minB)
				minB = dotProduct;
			else if (dotProduct> maxB)
				maxB = dotProduct;
		}

		if ((minA < minB ? minB - maxA : minA - maxB) > 0)
			intersect = false;
	}

	return intersect;
}

RAY CreatePickRay (D3DXVECTOR2 vScreenPos)
{
	RAY Ray;

	D3DVIEWPORT9 Viewport;
	APROJECT_WINDOW->GetD3DDevice ()->GetViewport (&Viewport);
	
	D3DXMATRIXA16 matProj;
	D3DXMATRIXA16 vInvView;
	matProj = *APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetProjectionMatrix ();
    D3DXMatrixInverse (&vInvView, NULL, APROJECT_WINDOW->GetCurScene ()->GetCamera ()->GetViewMatrix () );
    
	D3DXVECTOR3 vec;
	vec.x = ( ( ( ( (vScreenPos.x - Viewport.X) * 2.0f / Viewport.Width ) - 1.0f) ) - matProj._31) / matProj._11;
	vec.y = ( (-( ( (vScreenPos.y - Viewport.Y) * 2.0f / Viewport.Height) - 1.0f) ) - matProj._32) / matProj._22;
	vec.z = 1.0f;
	
    Ray.Dir.x = vec.x * vInvView._11 + vec.y * vInvView._21 + vec.z * vInvView._31;
    Ray.Dir.y = vec.x * vInvView._12 + vec.y * vInvView._22 + vec.z * vInvView._32;
    Ray.Dir.z = vec.x * vInvView._13 + vec.y * vInvView._23 + vec.z * vInvView._33;
    D3DXVec3Normalize (&Ray.Dir, &Ray.Dir);
	
    Ray.Origin.x = vInvView._41;
    Ray.Origin.y = vInvView._42;
    Ray.Origin.z = vInvView._43;

	return Ray;
}

bool SphereToSphereCollision (const D3DXVECTOR3& vC1,AReal32 fR1, const D3DXVECTOR3& vC2, AReal32 fR2, D3DXVECTOR3* pvD)
{
	D3DXVECTOR3	vNorm, vDist;
	D3DXVECTOR3	v	= vC1 - vC2;
	AReal32		fD	= D3DXVec3Dot (&v, &v);

	if (fD > (fR1 + fR2) * (fR1 + fR2) )
		return false;

	if (pvD)
	{
		D3DXVec3Normalize (&vNorm, &v);

		AReal32 fLength = D3DXVec3Length (&v);
		vDist = vNorm * ( (fR1 + fR2) - fLength);

		*pvD = vDist;
	}

	return true;
}

bool IntersectSphere (RAY Ray, const D3DXVECTOR3& vC, AReal32 fR, AReal32* pfD, D3DXVECTOR3* pvP)
{
	D3DXVECTOR3	vL;
	FLOAT		fS;
	FLOAT		fL2;
	FLOAT		fM2;
	FLOAT		fQ;

	vL	= vC - Ray.Origin;
	fS	= D3DXVec3Dot (&vL, &Ray.Dir);
	fL2	= D3DXVec3Dot (&vL, &vL);

	if ( (fS < 0) && (fL2 > (fR * fR) ) )
		return false;

	fM2 = fL2 - (fS * fS);

	if (fM2 > (fR * fR) )
		return false;

	fQ = (AReal32)sqrt ( (fR * fR) - fM2);

	if (fL2 > (fR * fR) )
	{
		if (pfD != NULL)
			*pfD = fS - fQ;
	}
	else
	{
		if (pfD != NULL)
			*pfD = fS + fQ;
	}

	if (pvP != NULL)
		*pvP = Ray.Origin + (*pfD) * Ray.Dir;

	return true;
}

bool IntersectTriangle (RAY Ray, const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, float& t, float& u, float& v)
{
    D3DXVECTOR3 vEdge1 = v1 - v0;
    D3DXVECTOR3 vEdge2 = v2 - v0;
	
	D3DXVECTOR3 vP;
	D3DXVec3Cross (&vP, &Ray.Dir, &vEdge2);
	
	float fDet = D3DXVec3Dot (&vEdge1, &vP);
	if (fDet < 0.0001f)
		return false;
		
	D3DXVECTOR3 vT = Ray.Origin - v0;
	
	u = D3DXVec3Dot (&vT, &vP);
	if (u < 0.0f || u > fDet)
		return false;
	
	D3DXVECTOR3 vQ;
	D3DXVec3Cross (&vQ, &vT, &vEdge1);
	
	v = D3DXVec3Dot (&Ray.Dir, &vQ);
	if (v < 0.0f || u + v > fDet)
		return false;
	
	t = D3DXVec3Dot (&vEdge2, &vQ);
	float fInvDet = 1.0f / fDet;
	
	t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;
	
    return true;
}

bool isFileExists(char* s)
{
	_finddatai64_t c_file;
	intptr_t hFile;
	bool result = false;

	if ((hFile = _findfirsti64(s, &c_file)) == -1L)
		// 파일이 없다면
		result = false;
	else
	if (c_file.attrib & _A_SUBDIR)
		// 디렉토리라면
		result = false;
	else
		result = true;

	_findclose(hFile);

	return result;
}

bool isDirExists(char* s)
{
	_finddatai64_t c_file;
	intptr_t hFile;
	bool result = false;

	hFile = _findfirsti64(s, &c_file);
	if (c_file.attrib & _A_SUBDIR)
		result = true;

	_findclose(hFile);

	return result;
}

/**
	@brief			: GaussianDistribution
	@description	: Helper function for GetSampleOffsets function to compute the 
					  2 parameter Gaussian distrubution using the given standard deviation rho
*/
float GaussianDistribution (float x, float y, float rho)
{
	float g = 1.0f / sqrtf (2.0f * D3DX_PI * rho * rho);
	g *= expf (- (x * x + y * y) /  (2 * rho * rho) );

	return g;
}

AReal32 expoEasingOut(AReal32 t, AReal32 b, AReal32 c, AReal32 d)
{
	return c * (-pow(2, -10 * t / d) + 1) + b;
}

AReal32 expoEasingInOut(AReal32 t, AReal32 b, AReal32 c, AReal32 d)
{
	t /= d / 2;
	if (t < 1.0f)
		return c / 2 * pow(2, 10 * (t - 1)) + b;

	t -= 1.0f;
	return c / 2 * (-pow(2, -10 * t) + 2) + b;
}

typedef struct
{
	AReal32 x, y, z, w;
	AReal32 u, v;
}QuadRenderStruct;
void QuadRender (D3DXVECTOR2 p1, D3DXVECTOR2 p2)
{
	AReal32 w = p2.x - p1.x;
	AReal32 h = p2.y - p1.y;
	QuadRenderStruct axPlaneVertices[] =
	{
		{ 0, 0, .5f, 1, 0 + .5f / w, 0 + .5f / h },
		{ w, 0, .5f, 1, 1 + .5f / w, 0 + .5f / h },
		{ w, h, .5f, 1, 1 + .5f / w, 1 + .5f / h },
		{ 0, h, .5f, 1, 0 + .5f / w, 1 + .5f / h }
	};
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZRHW | D3DFVF_TEX1);
	APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitiveUP (D3DPT_TRIANGLEFAN, 2, axPlaneVertices, sizeof (QuadRenderStruct));
}

void DrawRect (D3DXVECTOR2 p1, D3DXVECTOR2 p2)
{
	LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
	{
		if (FAILED (APROJECT_WINDOW->GetD3DDevice ()->CreateVertexBuffer (4 * sizeof (Texture2DVertex), D3DUSAGE_WRITEONLY,
			D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pVertexBuffer, NULL) ) )
		{
			return;
		}
		Texture2DVertex* pVertices;
		Texture2DVertex vertices [] = {
			{p1.x,	p1.y,	0.0f, 1.0f, 0.0f, 0.0f},
			{p2.x,	p1.y,	0.0f, 1.0f, 1.0f, 0.0f},
			{p1.x,	p2.y,	0.0f, 1.0f, 0.0f, 1.0f},
			{p2.x,	p2.y,	0.0f, 1.0f, 1.0f, 1.0f},
		};
		if (FAILED (pVertexBuffer->Lock (0, sizeof (*vertices) * 4, (void**)&pVertices, 0) ) )
		{
			return;
		}
		memcpy (pVertices, vertices, sizeof (*vertices) * 4);
		pVertexBuffer->Unlock();
	}
	
	{
		APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, pVertexBuffer, 0, sizeof (Texture2DVertex) );
		APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZRHW | D3DFVF_TEX1);
		APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitive (D3DPT_TRIANGLESTRIP, 0, 2);
	}
	{
		SAFE_RELEASE (pVertexBuffer);
	}
}

void DrawLine (D3DXVECTOR3 p1, D3DXVECTOR3 p2, AColor color)
{
	struct Tmp { float x, y, z; DWORD color; };
	Tmp vertices [] =
	{
		{p1.x, p1.y, p1.z, color.GetD3DColor () },	// v0
		{p2.x, p2.y, p2.z, color.GetD3DColor () },	// v1
	};

	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE);
	APROJECT_WINDOW->GetD3DDevice ()->DrawPrimitiveUP (D3DPT_TRIANGLELIST, 2, vertices, 0);
}