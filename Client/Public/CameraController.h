#pragma once
#include <Transform.h>

#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CCameraController : public IScript
{
public :
    _float3             m_vOffsetToPlayer = { 0.f, 4.f, -8.f };
    _float              m_fMouseSensor = 0.05f;
    SCRIPT_OBJECT_REF   m_refCamera{};
    SCRIPT_OBJECT_REF   m_refTarget{};

public:
    SCRIPT_FIELDS_BEGIN(CCameraController)
        SCRIPT_FIELD_FLOAT3(m_vOffsetToPlayer)
        SCRIPT_FIELD_FLOAT(m_fMouseSensor)
        SCRIPT_FIELD_OBJECT_REF(m_refCamera)
        SCRIPT_FIELD_OBJECT_REF(m_refTarget)
    SCRIPT_FIELDS_END(CCameraController)

private:
    Engine::CGameObject*    m_goNormalCam{};
    CTransform              m_trNormalCam;

    Engine::CGameObject*    m_goTarget{};
    CTransform              m_trTarget;

private:
    _float      m_fYawDegree = 0.f;
    _float      m_fPitchDegree = 0.f;
    _float      m_fDistance = 0.f;
    _float      m_fHeight = 0.f;

private:
    _float      m_fTargetYawDegree = 0.f;       
    _float      m_fTargetPitchDegree = 0.f;

    _float      m_fYawSharpness = 22.f;         /* 좌우 회전이 목표값을 따라가는 속도. 높을수록 더 즉각적 */
    _float      m_fPitchSharpness = 18.f;       /* 상하 회전이 목표값을 따라가는 속도. 높을수록 더 즉각적 */
    _float      m_fFollowSharpness = 15.f;      /* 카메라 위치가 목표 위치를 따라가는 속도. 높을수록 덜 묵직함 */
    _float      m_fLookSharpness = 17.f;        /* 카메라가 바라보는 지점 보간 속도. 높을수록 시선이 빠르게 따라감 */

    _float3     m_vCurrentLookTargetPos{};
    _bool       m_bCameraInitialized = false;

    _float      m_fYawInputAccum = 0.f;         /* 아직 처리되지 않은 좌우 입력 누적값 */
    _float      m_fPitchInputAccum = 0.f;       /* 아직 처리되지 않은 상하 입력 누적값 */
    _float      m_fInputResponseSharpness = 25.f;   /* 마우스 입력이 얼마나 빨리 반영될지. 높을수록 손에 더 붙음 */

private:
    void Follow_Target(_float fDT);
    _float WrapAngleDeg(_float fAngle);

public :
    void Pitch(_float fDegree);
    void Yaw(_float fDegree);
    void Add_Yaw_Input(_float fDegree);
    void Add_Pitch_Input(_float fDegree);

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
