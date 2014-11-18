#include "AWindow.h"

CARootScene::CARootScene (void)
{
	m_bInitalize = false;
	m_mapTable = new hash_map<wstring, CAObject*>();
	m_mapTable->clear ();

	m_pCamera = NULL;
}

CARootScene::~CARootScene (void)
{
	hash_map<wstring, CAObject*>::iterator itr;
	itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		SAFE_DELETE (itr->second);
		m_mapTable->erase(itr++);
	}
	SAFE_DELETE(m_mapTable);
}

HRESULT CARootScene::Initialize (void)
{
	if (m_bInitalize)
	{
		DebugLog ("Initialize (Already Initializing) 실패!");
		DebugConsoleLog (DebugError, "ARootScene: Initialize (Already Initializing) 실패!");
		return AE_ERROR_ALREADY_INITIALIZE;
	}

	m_nLayerCount = 1;

	m_dwBGColor = 0xFF99D9EA;

	m_bInitalize = true;

	return AE_SUCCESS;
}

AInt32 CARootScene::FindLayer (const WCHAR* pTitle)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->find(pTitle);
	if (itr != m_mapTable->end())
		return itr->second->GetLayer ();

	return -1;
}

CAObject* CARootScene::Trans (const WCHAR* pTitle)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->find(pTitle);
	if (itr != m_mapTable->end())
		return itr->second;

	return NULL;
}

HRESULT CARootScene::ChangeInLayer (const WCHAR* pTitle, int nToLayer)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->find(pTitle);
	if (itr != m_mapTable->end())
	{
		itr->second->SetLayer (nToLayer);
		return AE_SUCCESS;
	}

	return AE_ERROR_NOT_FIND_NAME;
}

HRESULT CARootScene::ChangeInPosZ (const WCHAR* pTitle, float fZ)
{
	return AE_SUCCESS;
	/*
	wstring wsTemp;
	CAObject* pTemp;
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable.find (pTitle);
	if (itr != m_mapTable.end () )
	{
		wsTemp = itr->first;
		pTemp = itr->second;

		m_mapTable.erase (itr);

		if (fZ + pTemp->GetOffset ().z < itr->second->GetPos ().z + itr->second->GetOffset ().z)
		{
			for (itr --; itr != m_mapTable.begin (); itr --)
				if (fZ + pTemp->GetOffset ().z >= itr->second->GetPos ().z + itr->second->GetOffset ().z)
				{
					itr ++;
					break;
				}
		}
		else if (fZ + pTemp->GetOffset ().z > itr->second->GetPos ().z + itr->second->GetOffset ().z)
			for (itr ++; itr != m_mapTable.end (); itr ++)
				if (fZ + pTemp->GetOffset ().z <= itr->second->GetPos ().z + itr->second->GetOffset ().z)
				{
					itr --;
					break;
				}
		else
			itr ++;

		pTemp->SetPos (itr->second->GetPos ().x, itr->second->GetPos ().y, fZ);
		m_mapTable.insert (itr, map<wstring, CAObject*>::value_type (wsTemp, pTemp) );

		return AE_SUCCESS;
	}

	return AE_ERROR_NOT_FIND_NAME;
	*/
}

HRESULT CARootScene::Commit (int nLayer, const WCHAR* pTitle, CAObject* pObject)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->find(pTitle);
	if (itr != m_mapTable->end())
	{
		SAFE_DELETE (pObject);
		return AE_ERROR_EQUAL_NAME;
	}

	itr = m_mapTable->end();
	//for (itr = m_mapTable.begin (); itr != m_mapTable.end (); itr ++)
	//{
	//	if (pObject->GetPos ().z + pObject->GetOffset ().z > itr->second->GetPos ().z + itr->second->GetOffset ().z)
	//	{
	//		//itr --;
	//		break;
	//	}
	//}

	pObject->SetName (pTitle);
	pObject->SetLayer (nLayer);
	if (m_mapTable->begin() == m_mapTable->end())
		m_mapTable->insert (hash_map<wstring, CAObject*>::value_type(pTitle, pObject));
	else
		m_mapTable->insert (itr, hash_map<wstring, CAObject*>::value_type (pTitle, pObject) );

	return AE_SUCCESS;
}

HRESULT CARootScene::Remove (const WCHAR* pTitle)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->find(pTitle);
	if (itr != m_mapTable->end () )
	{
		SAFE_DELETE (itr->second);
		m_mapTable->erase (itr);

		return AE_SUCCESS;
	}

	return AE_ERROR_NOT_FIND_NAME;
}

CACamera* CARootScene::GetCamera (void)
{
	return m_pCamera;
}

DWORD CARootScene::GetColor (void)
{
	return m_dwBGColor;
}

void CARootScene::SetColor (DWORD dwBGColor)
{
	m_dwBGColor = dwBGColor;
}

void CARootScene::Create (LPDIRECT3DDEVICE9 pD3DDevice)
{
	// 월드 행렬 설정
    D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity (&matWorld);
    pD3DDevice->SetTransform (D3DTS_WORLD, &matWorld);

	// 뷰 행렬 설정
	D3DXVECTOR3 vEyePt (0.0f, 50.0f, -30.0f);
    D3DXVECTOR3 vLookatPt (0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec (0.0f, 1.0f, 0.0f);

    // 프로젝션 행렬 설정
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH (&matProj, D3DX_PI / 4, (float)APROJECT_WINDOW->GetSetting ()->BackBufferWidth / (float)APROJECT_WINDOW->GetSetting ()->BackBufferHeight, 0.2f, 200.0f);
    pD3DDevice->SetTransform (D3DTS_PROJECTION, &matProj);

	D3DXMatrixPerspectiveFovLH (&matProj, D3DX_PI / 4, (float)APROJECT_WINDOW->GetSetting ()->BackBufferWidth / (float)APROJECT_WINDOW->GetSetting ()->BackBufferHeight, 0.2f, 200.0f);

	// Camera Process
	m_pCamera = new CACamera (matProj);
	m_pCamera->SetLockFrustum (false);
	m_pCamera->SetView (&vEyePt, &vLookatPt, &vUpVec);
}

void CARootScene::Reset (LPDIRECT3DDEVICE9 pD3DDevice)
{
	hash_map<wstring, CAObject*>::iterator itr;

	itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		itr->second->Reset ();
		itr ++;
	}
}

void CARootScene::Update (LPDIRECT3DDEVICE9 pD3DDevice, AReal32 dt)
{
	hash_map<wstring, CAObject*>::iterator itr;

	// Camera Process
	pD3DDevice->SetTransform (D3DTS_VIEW, m_pCamera->GetViewMatrix () );

	itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		itr->second->Update ();
		itr ++;
	}
}

void CARootScene::Render (LPDIRECT3DDEVICE9 pD3DDevice, BOOL bRenderOrtho2D)
{
	hash_map<wstring, CAObject*>::iterator itr;

	// Camera Process
	//m_pCamera->GetFrustum ()->Draw ();

	// Clear the window to bgcolor
	if (bRenderOrtho2D == FALSE)
		pD3DDevice->Clear (0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_dwBGColor, 1.0f, 0);

	itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		if (itr->second->GetOrtho2D() == bRenderOrtho2D)
			itr->second->Render ();
		itr ++;
	}
}

void CARootScene::Destroy (void)
{
	hash_map<wstring, CAObject*>::iterator itr;

	// Camera Process
	SAFE_DELETE (m_pCamera);

	itr = m_mapTable->begin();
	while (itr != m_mapTable->end())
	{
		SAFE_DELETE (itr->second);
		itr ++;
	}
	m_mapTable->clear();
}

void CALLBACK CARootScene::MouseEvent (AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown)
{
}

void CALLBACK CARootScene::KeyboardEvent (AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown)
{
}

void CALLBACK CARootScene::ObjectEvent (const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam)
{
}