#include "AWindow.h"

// CAFrustum
CAFrustum::CAFrustum (void)
{
	ZeroMemory (m_vtx, sizeof (m_vtx [0]) * 8);
	ZeroMemory (m_plane, sizeof (m_plane [0]) * 6);

	// 투영행렬까지 거치면 모든 3차원 월드좌표의 점은 (-1,-1,0) ~ (1,1,1)사이의 값으로 바뀐다.
	// m_vtx에 이 동차공간의 경계값을 넣어둔다.
	m_vtx [0] = D3DXVECTOR3 (-1.0f,  1.0f, 0.0f);
	m_vtx [1] = D3DXVECTOR3 ( 1.0f,  1.0f, 0.0f);
	m_vtx [2] = D3DXVECTOR3 (-1.0f, -1.0f, 0.0f);
	m_vtx [3] = D3DXVECTOR3 ( 1.0f, -1.0f, 0.0f);
	m_vtx [4] = D3DXVECTOR3 (-1.0f,  1.0f, 1.0f);
	m_vtx [5] = D3DXVECTOR3 ( 1.0f,  1.0f, 1.0f);
	m_vtx [6] = D3DXVECTOR3 (-1.0f, -1.0f, 1.0f);
	m_vtx [7] = D3DXVECTOR3 ( 1.0f, -1.0f, 1.0f);
}

CAFrustum::~CAFrustum (void)
{
}

void CAFrustum::Make (D3DXMATRIXA16* pmatViewProj)
{
	int				i;
	D3DXMATRIXA16	matInv;
	D3DXVECTOR3		vTransformed [8];

	// view * proj의 역행렬을 구한다.
	D3DXMatrixInverse (&matInv, NULL, pmatViewProj);

	// Vertex_최종 = Vertex_local * Matrix_world * Matrix_view * Matrix_Proj 인데,
	// Vertex_world = Vertex_local * Matrix_world이므로,
	// Vertex_최종 = Vertex_world * Matrix_view * Matrix_Proj 이다.
	// Vertex_최종 = Vertex_world * ( Matrix_view * Matrix_Proj ) 에서
	// 역행렬( Matrix_view * Matrix_Proj )^-1를 양변에 곱하면
	// Vertex_최종 * 역행렬( Matrix_view * Matrix_Proj )^-1 = Vertex_World 가 된다.
	// 그러므로, m_vtx * matInv = Vertex_world가 되어, 월드좌표계의 프러스텀 좌표를 얻을 수 있다.
	for (i = 0; i < 8; ++i)
		D3DXVec3TransformCoord (&vTransformed [i], &m_vtx [i], &matInv);
	
	// 0번과 5번은 프러스텀중 near평면의 좌측상단과 우측하단이므로, 둘의 좌표를 더해서 2로 나누면
	// 카메라의 좌표를 얻을 수 있다.(정확히 일치하는 것은 아니다.)
	m_vPos = (vTransformed [0] + vTransformed [3]) / 2.0f;
	
	// 얻어진 월드좌표로 프러스텀 평면을 만든다
	// 벡터가 프러스텀 안쪽에서 바깥쪽으로 나가는 평면들이다.
	D3DXPlaneFromPoints (&m_plane [0], vTransformed + 0,	vTransformed + 1,	vTransformed + 2);	// 원 평면 (far)
	D3DXPlaneFromPoints (&m_plane [1], vTransformed + 5,	vTransformed + 4,	vTransformed + 7);	// 근 평면 (near)
	D3DXPlaneFromPoints (&m_plane [2], vTransformed + 4,	vTransformed + 0,	vTransformed + 6);	// 좌 평면 (left)
	D3DXPlaneFromPoints (&m_plane [3], vTransformed + 1,	vTransformed + 5,	vTransformed + 3);	// 우 평면 (right)
	D3DXPlaneFromPoints (&m_plane [4], vTransformed + 4,	vTransformed + 5,	vTransformed + 2);	// 상 평면 (top)
	D3DXPlaneFromPoints (&m_plane [5], vTransformed + 2,	vTransformed + 3,	vTransformed + 6);	// 하 평면 (bottom)
}

bool CAFrustum::IsIn (D3DXVECTOR3* pv, bool b3DCheck)
{
	int i;

	for (i = 0; i < 6; i ++)
	{
		if (!b3DCheck && i == 4)
			break;

		if (D3DXPlaneDotCoord (&m_plane [i], pv) > PLANE_EPSILON)				// plane의 normal벡터가 far로 향하고 있으므로 양수이면 프러스텀의 바깥쪽
			return false;
	}

	return true;
}

bool CAFrustum::IsInSphere (D3DXVECTOR3* pv, float radius, bool b3DCheck)
{
	int i;

	for (i = 0; i < 6; i ++)
	{
		if (!b3DCheck && i == 4)
			break;

		if (D3DXPlaneDotCoord (&m_plane [i], pv) > (radius + PLANE_EPSILON) )	// 평면과 중심점의 거리가 반지름보다 크면 프러스텀에 없음
			return false;
	}

	return true;
}

void CAFrustum::Draw (void)
{
	WORD index [] = { 0, 1, 2,
					0, 2, 3,
					4, 7, 6,
					4, 6, 5,
					1, 5, 6,
					1, 6, 2,
					0, 3, 7,
					0, 7, 4,
					0, 4, 5,
					0, 5, 1,
					3, 7, 6,
					3, 6, 2 };

    D3DMATERIAL9 mtrl;
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );

	typedef struct tagVTX
	{
		D3DXVECTOR3	p;
	} VTX;

	VTX vtx [8];

	for (int i = 0; i < 8; i ++)
		vtx [i].p = m_vtx [i];

	DWORD dw0Colorop, dw1Colorop;
	DWORD dwAlphaEnable;
	DWORD dwSrcBlend;
	DWORD dwDestBlend;
	DWORD dwLighting;

	// Get Data
	APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (0, D3DTSS_COLOROP, &dw0Colorop);
	APROJECT_WINDOW->GetD3DDevice ()->GetTextureStageState (1, D3DTSS_COLOROP, &dw1Colorop);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_ALPHABLENDENABLE, &dwAlphaEnable);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_SRCBLEND, &dwSrcBlend);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_DESTBLEND, &dwDestBlend);
	APROJECT_WINDOW->GetD3DDevice ()->GetRenderState (D3DRS_LIGHTING, &dwLighting);

	// Set New Data
	APROJECT_WINDOW->GetD3DDevice ()->SetFVF (D3DFVF_XYZ);
	APROJECT_WINDOW->GetD3DDevice ()->SetStreamSource (0, NULL, 0, sizeof (VTX) );
	APROJECT_WINDOW->GetD3DDevice ()->SetTexture (0, NULL);
	APROJECT_WINDOW->GetD3DDevice ()->SetIndices (0);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, true);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ZERO);

	// 파란색으로 상,하 평면을 그린다.
    APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_LIGHTING, FALSE);
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.g = mtrl.Diffuse.g = 0.1f;
	mtrl.Ambient.r = mtrl.Diffuse.r = 0.1f;
	mtrl.Ambient.a = mtrl.Diffuse.a = 0.5f;
    APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&mtrl);
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, 8, 4, index, D3DFMT_INDEX16, vtx, sizeof (vtx [0]) );

	// 녹색으로 좌,우 평면을 그린다.
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.a = mtrl.Diffuse.a = 0.5f;
    APROJECT_WINDOW->GetD3DDevice ()->SetMaterial( &mtrl );
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, 8, 4, index + 4 * 3, D3DFMT_INDEX16, vtx, sizeof (vtx [0]) );

	// 붉은색으로 원,근 평면을 그린다.
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = mtrl.Diffuse.g = 0.1f;
	mtrl.Ambient.b = mtrl.Diffuse.b = 0.1f;
	mtrl.Ambient.a = mtrl.Diffuse.a = 0.5f;
    APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&mtrl);
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, 8, 4, index + 8 * 3, D3DFMT_INDEX16, vtx, sizeof (vtx [0]) );

	// Original Data
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (0, D3DTSS_COLOROP, dw0Colorop);
	APROJECT_WINDOW->GetD3DDevice ()->SetTextureStageState (1, D3DTSS_COLOROP, dw1Colorop);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_ALPHABLENDENABLE, dwAlphaEnable);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_SRCBLEND, dwSrcBlend);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_DESTBLEND, dwDestBlend);
	APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_LIGHTING, dwLighting);
}

// CACamera
CACamera::CACamera (_D3DXMATRIXA16 mmatProj)
	:m_matProj (mmatProj)
{
	m_pFrustum = new CAFrustum ();

	D3DXVECTOR3 eye (0.0f, 0.0f, 0.0f);
	D3DXVECTOR3	lookat (0.0f, 0.0f, -1.0f);
	D3DXVECTOR3	up (0.0f, 1.0f, 0.0f);
	D3DXMatrixIdentity (&m_matView);
	D3DXMatrixIdentity (&m_matBill);
	SetView (&eye, &lookat, &up);

	//
	vRight.x = 1.0f; 
	vRight.y = 0.0f; 
	vRight.z = 0.0f; 

	vLook.x = 0.0f; 
	vLook.y = 0.0f; 
	vLook.z = 1.0f; 

	vUp.x = 0.0f; 
	vUp.y = 1.0f; 
	vUp.z = 0.0f; 

	vTRight = vRight; 
	vTLook = vLook; 
	vTUp = vUp; 

	v3OLook.x = 0.0f; 
	v3OLook.y = 0.0f; 
	v3OLook.z = -1.0f; 

	v3OEye.x = 0.0f; 
	v3OEye.y = 0.0f; 
	v3OEye.z = -1.0f; 

	tx_angle = 0.0f; 
	ty_angle = 0.0f;

	m_bLockFrustum = false;
}

CACamera::~CACamera ()
{
	SAFE_DELETE (m_pFrustum);
}

D3DXMATRIXA16* CACamera::SetView (D3DXVECTOR3* pvEye, D3DXVECTOR3* pvLookat, D3DXVECTOR3* pvUp)
{
	m_vEye		= *pvEye;
	m_vLookat	= *pvLookat;
	m_vUp		= *pvUp;
	D3DXVec3Normalize (&m_vView, &(m_vLookat - m_vEye) );
	D3DXVec3Cross (&m_vCross, &m_vUp, &m_vView);

	D3DXMatrixLookAtLH (&m_matView, &m_vEye, &m_vLookat, &m_vUp);
	D3DXMatrixInverse (&m_matBill, NULL, &m_matView);
	m_matBill._41 = 0.0f;
	m_matBill._42 = 0.0f;
	m_matBill._43 = 0.0f;

	UpdateFrustum ();

	return &m_matView;
}

void CACamera::UpdateFrustum (void)
{
	D3DXMATRIXA16 m;
	m = *GetViewMatrix () * m_matProj;				// World좌표를 얻기위해서 View * Proj행렬을 계산한다.
	if (!m_bLockFrustum) m_pFrustum->Make (&m);		// View*Proj행렬로 Frustum을 만든다.
}

D3DXMATRIXA16* CACamera::RotateLocalX (float angle)
{
	if (angle == 0)
		return &m_matView;

	D3DXMATRIXA16 matRot;
	D3DXMatrixRotationAxis (&matRot, &m_vCross, angle);

	D3DXVECTOR3 vNewDst, vNewUp;
	D3DXVec3TransformCoord (&vNewDst, &m_vView, &matRot);	// View * Rot => DST Vector
	vNewDst += m_vEye;										// Real DST Position = Eye Position + DST Vector

	return SetView (&m_vEye, &vNewDst, &m_vUp);
}

D3DXMATRIXA16* CACamera::RotateLocalY (float angle)
{
	if (angle == 0)
		return &m_matView;

	D3DXMATRIXA16 matRot;
	D3DXMatrixRotationAxis (&matRot, &m_vUp, angle);

	D3DXVECTOR3 vNewDst;
	D3DXVec3TransformCoord (&vNewDst, &m_vView, &matRot);	// View * Rot => DST Vector
	vNewDst += m_vEye;										// Real DST Position = Eye Position + DST Vector

	return SetView (&m_vEye, &vNewDst, &m_vUp);
}

D3DXMATRIXA16* CACamera::MoveTo (D3DXVECTOR3* pv)
{
	D3DXVECTOR3	dv = *pv - m_vEye;
	m_vEye = *pv;
	m_vLookat += dv;
	return SetView (&m_vEye, &m_vLookat, &m_vUp);
}

D3DXMATRIXA16* CACamera::MoveLocalX (float dist)
{
	D3DXVECTOR3 vNewEye	= m_vEye;
	D3DXVECTOR3 vNewDst	= m_vLookat;

	D3DXVECTOR3 vMove;
	D3DXVec3Normalize (&vMove, &m_vCross);
	vMove	*= dist;
	vNewEye += vMove;
	vNewDst += vMove;

	return SetView (&vNewEye, &vNewDst, &m_vUp);
}

D3DXMATRIXA16* CACamera::MoveLocalY (float dist)
{
	D3DXVECTOR3 vNewEye	= m_vEye;
	D3DXVECTOR3 vNewDst	= m_vLookat;

	D3DXVECTOR3 vMove;
	D3DXVec3Normalize (&vMove, &m_vUp);
	vMove	*= dist;
	vNewEye += vMove;
	vNewDst += vMove;

	return SetView (&vNewEye, &vNewDst, &m_vUp);
}

D3DXMATRIXA16* CACamera::MoveLocalZ (float dist)
{
	D3DXVECTOR3 vNewEye	= m_vEye;
	D3DXVECTOR3 vNewDst	= m_vLookat;

	D3DXVECTOR3 vMove;
	D3DXVec3Normalize (&vMove, &m_vView);
	vMove	*= dist;
	vNewEye += vMove;
	vNewDst += vMove;

	return SetView (&vNewEye, &vNewDst, &m_vUp);
}