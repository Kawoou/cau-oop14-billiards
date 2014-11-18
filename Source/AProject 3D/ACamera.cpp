#include "AWindow.h"

// CAFrustum
CAFrustum::CAFrustum (void)
{
	ZeroMemory (m_vtx, sizeof (m_vtx [0]) * 8);
	ZeroMemory (m_plane, sizeof (m_plane [0]) * 6);

	// ������ı��� ��ġ�� ��� 3���� ������ǥ�� ���� (-1,-1,0) ~ (1,1,1)������ ������ �ٲ��.
	// m_vtx�� �� ���������� ��谪�� �־�д�.
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

	// view * proj�� ������� ���Ѵ�.
	D3DXMatrixInverse (&matInv, NULL, pmatViewProj);

	// Vertex_���� = Vertex_local * Matrix_world * Matrix_view * Matrix_Proj �ε�,
	// Vertex_world = Vertex_local * Matrix_world�̹Ƿ�,
	// Vertex_���� = Vertex_world * Matrix_view * Matrix_Proj �̴�.
	// Vertex_���� = Vertex_world * ( Matrix_view * Matrix_Proj ) ����
	// �����( Matrix_view * Matrix_Proj )^-1�� �纯�� ���ϸ�
	// Vertex_���� * �����( Matrix_view * Matrix_Proj )^-1 = Vertex_World �� �ȴ�.
	// �׷��Ƿ�, m_vtx * matInv = Vertex_world�� �Ǿ�, ������ǥ���� �������� ��ǥ�� ���� �� �ִ�.
	for (i = 0; i < 8; ++i)
		D3DXVec3TransformCoord (&vTransformed [i], &m_vtx [i], &matInv);
	
	// 0���� 5���� ���������� near����� ������ܰ� �����ϴ��̹Ƿ�, ���� ��ǥ�� ���ؼ� 2�� ������
	// ī�޶��� ��ǥ�� ���� �� �ִ�.(��Ȯ�� ��ġ�ϴ� ���� �ƴϴ�.)
	m_vPos = (vTransformed [0] + vTransformed [3]) / 2.0f;
	
	// ����� ������ǥ�� �������� ����� �����
	// ���Ͱ� �������� ���ʿ��� �ٱ������� ������ �����̴�.
	D3DXPlaneFromPoints (&m_plane [0], vTransformed + 0,	vTransformed + 1,	vTransformed + 2);	// �� ��� (far)
	D3DXPlaneFromPoints (&m_plane [1], vTransformed + 5,	vTransformed + 4,	vTransformed + 7);	// �� ��� (near)
	D3DXPlaneFromPoints (&m_plane [2], vTransformed + 4,	vTransformed + 0,	vTransformed + 6);	// �� ��� (left)
	D3DXPlaneFromPoints (&m_plane [3], vTransformed + 1,	vTransformed + 5,	vTransformed + 3);	// �� ��� (right)
	D3DXPlaneFromPoints (&m_plane [4], vTransformed + 4,	vTransformed + 5,	vTransformed + 2);	// �� ��� (top)
	D3DXPlaneFromPoints (&m_plane [5], vTransformed + 2,	vTransformed + 3,	vTransformed + 6);	// �� ��� (bottom)
}

bool CAFrustum::IsIn (D3DXVECTOR3* pv, bool b3DCheck)
{
	int i;

	for (i = 0; i < 6; i ++)
	{
		if (!b3DCheck && i == 4)
			break;

		if (D3DXPlaneDotCoord (&m_plane [i], pv) > PLANE_EPSILON)				// plane�� normal���Ͱ� far�� ���ϰ� �����Ƿ� ����̸� ���������� �ٱ���
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

		if (D3DXPlaneDotCoord (&m_plane [i], pv) > (radius + PLANE_EPSILON) )	// ���� �߽����� �Ÿ��� ���������� ũ�� �������ҿ� ����
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

	// �Ķ������� ��,�� ����� �׸���.
    APROJECT_WINDOW->GetD3DDevice ()->SetRenderState (D3DRS_LIGHTING, FALSE);
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.g = mtrl.Diffuse.g = 0.1f;
	mtrl.Ambient.r = mtrl.Diffuse.r = 0.1f;
	mtrl.Ambient.a = mtrl.Diffuse.a = 0.5f;
    APROJECT_WINDOW->GetD3DDevice ()->SetMaterial (&mtrl);
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, 8, 4, index, D3DFMT_INDEX16, vtx, sizeof (vtx [0]) );

	// ������� ��,�� ����� �׸���.
    ZeroMemory (&mtrl, sizeof (D3DMATERIAL9) );
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.a = mtrl.Diffuse.a = 0.5f;
    APROJECT_WINDOW->GetD3DDevice ()->SetMaterial( &mtrl );
	APROJECT_WINDOW->GetD3DDevice ()->DrawIndexedPrimitiveUP (D3DPT_TRIANGLELIST, 0, 8, 4, index + 4 * 3, D3DFMT_INDEX16, vtx, sizeof (vtx [0]) );

	// ���������� ��,�� ����� �׸���.
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
	m = *GetViewMatrix () * m_matProj;				// World��ǥ�� ������ؼ� View * Proj����� ����Ѵ�.
	if (!m_bLockFrustum) m_pFrustum->Make (&m);		// View*Proj��ķ� Frustum�� �����.
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