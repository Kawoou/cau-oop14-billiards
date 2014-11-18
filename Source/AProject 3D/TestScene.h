#pragma once

using namespace std;

class CTestScene : public CARootScene
{
private:
	CATexture*			m_pTexture;
	CAGeometry*			m_pGeometry;
	int					m_nBrushType;

public:
	CTestScene (void);
	virtual ~CTestScene (void);

public:
	void DefaultSetting (LPDIRECT3DDEVICE9 pD3DDevice);

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