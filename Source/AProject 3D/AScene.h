#ifndef __APROJECT_SCENE__
#define __APROJECT_SCENE__

#pragma once

using namespace std;

class CARootScene
{
protected:
	int									m_nLayerCount;
	hash_map<wstring, CAObject*>*		m_mapTable;

private:
	bool								m_bInitalize;
	DWORD								m_dwBGColor;
	CACamera*							m_pCamera;

public:
	CARootScene (void);
	virtual ~CARootScene (void);

public:
	HRESULT Initialize (void);

public:
	CAObject* Trans (const WCHAR* pTitle);
	AInt32 FindLayer (const WCHAR* pTitle);
	HRESULT ChangeInLayer (const WCHAR* pTitle, int nToLayer);
	HRESULT ChangeInPosZ (const WCHAR* pTitle, float fZ);
	HRESULT Commit (int nLayer, const WCHAR* pTitle, CAObject* pObject);
	HRESULT Remove (const WCHAR* pTitle);

public:
	CACamera* GetCamera (void);
	DWORD GetColor (void);

	void SetColor (DWORD dwBGColor);

public:
	virtual void Create (LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void Reset (LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void Update (LPDIRECT3DDEVICE9 pD3DDevice, AReal32 dt);
	virtual void Render (LPDIRECT3DDEVICE9 pD3DDevice, BOOL bRenderOrtho2D);
	virtual void Destroy (void);

	virtual void CALLBACK MouseEvent (AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown);
	virtual void CALLBACK KeyboardEvent (AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown);
	virtual void CALLBACK ObjectEvent (const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif