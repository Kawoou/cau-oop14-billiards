#ifndef __APROJECT_ALGORITHM__
#define __APROJECT_ALGORITHM__

#pragma once

string IntToString(int nInteger);
wstring IntToWstring(int nInteger);

AUInt VKToDIK(AUInt cVk);
AUInt DIKToVK(AUInt cDik);

void ConvertToChar (WCHAR* pWChar, char* pChar);
void ConvertToWChar (char* pChar, WCHAR* pWChar);
void SaveScreenShot (PTCHAR filename, int w, int h);

bool PointToRectangleCollision(const AVector3& v, const RECT& rt);
bool LineToLineCollision(const AVector3& v1, const AVector3& v2, const AVector3& w1, const AVector3& w2);
bool PolyToPolyCollision(const AVector3* pPoly1, const AVector3* pPoly2, const AUInt32 nPoly1, const AUInt32 nPoly2);

RAY CreatePickRay (D3DXVECTOR2 vScreenPos);

bool SphereToSphereCollision (const D3DXVECTOR3& vC1,AReal32 fR1, const D3DXVECTOR3& vC2, AReal32 fR2, D3DXVECTOR3* pvD);

bool IntersectSphere (RAY Ray, const D3DXVECTOR3& vC, AReal32 fR, AReal32* pfD, D3DXVECTOR3* pvP);
bool IntersectTriangle (RAY Ray, const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, float& t, float& u, float& v);

float GaussianDistribution (float x, float y, float rho);

bool isFileExists(char* s);
bool isDirExists(char* s);

AReal32 expoEasingOut(AReal32 t, AReal32 b, AReal32 c, AReal32 d);
AReal32 expoEasingInOut(AReal32 t, AReal32 b, AReal32 c, AReal32 d);

void QuadRender (D3DXVECTOR2 p1, D3DXVECTOR2 p2);
void DrawRect (D3DXVECTOR2 p1, D3DXVECTOR2 p2);
void DrawLine (D3DXVECTOR3 p1, D3DXVECTOR3 p2, AColor color);

#endif