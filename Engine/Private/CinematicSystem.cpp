#include "CinematicSystem.h"
#include "CinematicIO.h"

#include "GameObject.h"

#include "Camera.h"
#include "Transform.h"
#include "Logger.h"

#include <cstdint>

IMPLEMENT_SINGLETON(CCinematic_System)

CCinematic_System::CCinematic_System()
{
}

CCinematic_System::~CCinematic_System()
{
}

HRESULT CCinematic_System::Initialize()
{
    m_bLoaded = false;
    m_bPlaying = false;
    m_bPause = false;
    m_fCurrentTime = 0.f;
    m_iPrevPriority = 0;

    return S_OK;
}

bool CCinematic_System::Load(const std::string& strFileName)
{
    Stop();

    if (false == CCinematicIO::Load(strFileName, m_tClip))
        return false;

    m_bLoaded = true;
    return true;
}

bool CCinematic_System::Play(CCamera camera)
{
    if (false == m_bLoaded)
        return false;

    if (false == camera.Is_Valid())
        return false;

    Stop();

    m_camera = camera;

    CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(m_camera->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObject, false, "pObject is nullptr");
    m_tr = pObject->Get_Component<CTransform>();

    if (false == m_tr.Is_Valid())
        return false;

    m_iPrevPriority = m_camera->iPriority;
    m_camera.Set_Priority(255);

    m_fCurrentTime = 0.f;
    m_fPrevTime = 0.f;
    m_bPause = false;
    m_bPlaying = true;

    /* 시작 시점 평가 */
    Evaluate(0.f);
    Apply_Shake(0.f);

    return true;
}

void CCinematic_System::Stop()
{
    if (m_camera.Is_Valid())
        m_camera.Set_Priority(m_iPrevPriority);

    m_bPlaying = false;
    m_bPause = false;
    m_fCurrentTime = 0.f;

    m_camera = {};
    m_tr = {};
}

void CCinematic_System::Pause(_bool bPause)
{
    m_bPause = bPause;
}

void CCinematic_System::Set_TestClip(const CINEMATIC_CLIP& tClip)
{
    Stop();
    m_tClip = tClip;
    m_bLoaded = true;
}

_bool CCinematic_System::Preview(CCamera camera, _float fTime)
{
    if (false == camera.Is_Valid())
        return false;

    m_camera = camera;

    auto* pObj = SYS_GAMEOBJECT.Get_Wrapper(m_camera->hObject);
    if (pObj == nullptr)
        return false;

    m_tr = pObj->Get_Component<CTransform>();
    if (false == m_tr.Is_Valid())
        return false;

    const _float fClampedTime = std::clamp(fTime, 0.f, m_tClip.fDuration);

    Evaluate(fClampedTime);
    Apply_Shake(fClampedTime);

    return true;
}

void CCinematic_System::Update(_float fDT)
{
    if (false == m_bPlaying)
        return;

    if (true == m_bPause)
        return;

    if (false == m_camera.Is_Valid())
        return;

    if (false == m_tr.Is_Valid())
        return;

    if (m_tClip.vecCameraKeys.empty())
    {
        Stop();
        return;
    }

    m_fPrevTime = m_fCurrentTime;
    m_fCurrentTime += fDT;
    m_fCurrentTime = std::clamp(m_fCurrentTime, 0.f, m_tClip.fDuration);

    Evaluate(m_fCurrentTime);
    Apply_Shake(m_fCurrentTime);
    Process_EventKeys(m_fPrevTime, m_fCurrentTime);

    if (m_fCurrentTime >= m_tClip.fDuration)
    {
        Stop();
    }
}

void CCinematic_System::Evaluate(_float fTime)
{
    /* 2번 방식:
       Shot이 있으면 Shot이 주도권을 가짐.
       Shot이 하나도 없거나, 적용 가능한 Shot이 없을 때만 CameraKey 기본 보간 fallback. */
    if (Try_Apply_ShotTrack(fTime))
        return;

    Apply_DefaultCameraFallback(fTime);
}

_bool CCinematic_System::Try_Apply_ShotTrack(_float fTime)
{
    const auto& vecShotKeys = m_tClip.vecShotKeys;
    const auto& vecCameraKeys = m_tClip.vecCameraKeys;

    if (vecShotKeys.empty())
        return false;

    const int32_t iActiveShotIndex = Find_ActiveShotIndex(fTime);
    if (iActiveShotIndex < 0 || iActiveShotIndex >= (int32_t)vecShotKeys.size())
        return false;

    const auto& tShot = vecShotKeys[iActiveShotIndex];

    if (tShot.iCameraKeyIndex >= vecCameraKeys.size())
        return false;

    /* CUT:
       해당 Shot 시점부터 다음 Shot 전까지 지정 CameraKey 유지 */
    if (tShot.eType == CINEMATIC_SHOT_TYPE::CUT)
    {
        Apply_CameraKey(vecCameraKeys[tShot.iCameraKeyIndex]);
        return true;
    }

    /* BLEND:
       Shot 시작~종료 동안 이전 Shot 카메라 -> 현재 Shot 카메라 보간
       종료 후에는 현재 Shot 카메라 유지 */
    if (tShot.eType == CINEMATIC_SHOT_TYPE::BLEND)
    {
        const int32_t iPrevShotIndex = Find_PreviousValidShotIndex(iActiveShotIndex);

        /* 이전 Shot이 없으면 시작점부터 대상 카메라 유지 */
        if (iPrevShotIndex < 0)
        {
            Apply_CameraKey(vecCameraKeys[tShot.iCameraKeyIndex]);
            return true;
        }

        const auto& tPrevShot = vecShotKeys[iPrevShotIndex];

        if (tPrevShot.iCameraKeyIndex >= vecCameraKeys.size())
        {
            Apply_CameraKey(vecCameraKeys[tShot.iCameraKeyIndex]);
            return true;
        }

        const auto& tFrom = vecCameraKeys[tPrevShot.iCameraKeyIndex];
        const auto& tTo = vecCameraKeys[tShot.iCameraKeyIndex];

        if (tShot.fBlendDuration <= 0.f)
        {
            Apply_CameraKey(tTo);
            return true;
        }

        const _float fBlendStart = tShot.fTime;
        const _float fBlendEnd = tShot.fTime + tShot.fBlendDuration;

        if (fTime <= fBlendStart)
        {
            Apply_CameraKey(tFrom);
            return true;
        }

        if (fTime >= fBlendEnd)
        {
            Apply_CameraKey(tTo);
            return true;
        }

        _float fRatio = (fTime - fBlendStart) / tShot.fBlendDuration;
        fRatio = std::clamp(fRatio, 0.f, 1.f);

        Apply_CameraKey(tFrom, tTo, fRatio);
        return true;
    }

    return false;
}

_bool CCinematic_System::Apply_DefaultCameraFallback(_float fTime)
{
    auto& vecKeys = m_tClip.vecCameraKeys;

    if (vecKeys.empty())
        return false;

    if (vecKeys.size() == 1)
    {
        Apply_CameraKey(vecKeys[0]);
        return true;
    }

    /* 첫 번째 키 전 */
    if (fTime <= vecKeys.front().fTime)
    {
        Apply_CameraKey(vecKeys.front());
        return true;
    }

    /* 마지막 키 후 */
    if (fTime >= vecKeys.back().fTime)
    {
        Apply_CameraKey(vecKeys.back());
        return true;
    }

    /* Shot이 하나도 없을 때만 쓰는 기본 CameraKey 시간 보간 */
    for (size_t i = 0; i + 1 < vecKeys.size(); ++i)
    {
        const auto& tA = vecKeys[i];
        const auto& tB = vecKeys[i + 1];

        if (fTime < tA.fTime || fTime > tB.fTime)
            continue;

        _float fRange = tB.fTime - tA.fTime;
        if (fRange <= 0.f)
            fRange = 0.0001f;

        _float fRatio = (fTime - tA.fTime) / fRange;
        fRatio = std::clamp(fRatio, 0.f, 1.f);

        switch (tA.eEase)
        {
        case CINEMATIC_EASE::EASE_IN:
            fRatio = fRatio * fRatio;
            break;

        case CINEMATIC_EASE::EASE_OUT:
            fRatio = 1.f - (1.f - fRatio) * (1.f - fRatio);
            break;

        case CINEMATIC_EASE::EASE_IN_OUT:
            if (fRatio < 0.5f)
                fRatio = 2.f * fRatio * fRatio;
            else
                fRatio = 1.f - powf(-2.f * fRatio + 2.f, 2.f) * 0.5f;
            break;

        default:
            break;
        }

        Apply_CameraKey(tA, tB, fRatio);
        return true;
    }

    return false;
}

int32_t CCinematic_System::Find_ActiveShotIndex(_float fTime) const
{
    const auto& vecShotKeys = m_tClip.vecShotKeys;

    if (vecShotKeys.empty())
        return -1;

    int32_t iActiveShotIndex = -1;

    for (int32_t i = 0; i < (int32_t)vecShotKeys.size(); ++i)
    {
        if (vecShotKeys[i].fTime > fTime)
            break;

        iActiveShotIndex = i;
    }

    return iActiveShotIndex;
}

int32_t CCinematic_System::Find_PreviousValidShotIndex(int32_t iShotIndex) const
{
    const auto& vecShotKeys = m_tClip.vecShotKeys;

    for (int32_t i = iShotIndex - 1; i >= 0; --i)
    {
        if (vecShotKeys[i].iCameraKeyIndex < m_tClip.vecCameraKeys.size())
            return i;
    }

    return -1;
}

void CCinematic_System::Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tKey)
{
    m_tr.Set_Position(XMLoadFloat3(&tKey.vPosition));
    m_tr.Set_Rotation_Quaternion(XMLoadFloat4(&tKey.vRotationQuat));
    m_camera.Set_Fovy(tKey.fFovy);
}

void CCinematic_System::Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tA, const CINEMATIC_CAMERA_KEY& tB, _float fRatio)
{
    _vector vPosA = XMLoadFloat3(&tA.vPosition);
    _vector vPosB = XMLoadFloat3(&tB.vPosition);

    _vector vQuatA = XMLoadFloat4(&tA.vRotationQuat);
    _vector vQuatB = XMLoadFloat4(&tB.vRotationQuat);

    _vector vPos = XMVectorLerp(vPosA, vPosB, fRatio);
    _vector vRot = XMQuaternionSlerp(vQuatA, vQuatB, fRatio);

    _float fFovy = tA.fFovy + (tB.fFovy - tA.fFovy) * fRatio;

    m_tr.Set_Position(vPos);
    m_tr.Set_Rotation_Quaternion(vRot);
    m_camera.Set_Fovy(fFovy);
}

void CCinematic_System::Process_EventKeys(_float fPrevTime, _float fCurTime)
{
    for (const auto& tEventKey : m_tClip.vecEventKeys)
    {
        if (tEventKey.fTime <= fPrevTime)
            continue;

        if (tEventKey.fTime > fCurTime)
            continue;

        CINEMATIC_EVENT_DATA tEventData{ tEventKey.szEventName };
        OnCinematicEvent.Invoke(tEventData);
    }
}

void CCinematic_System::Apply_Shake(_float fTime)
{
    if (false == m_tr.Is_Valid())
        return;

    const auto& vecShakeKeys = m_tClip.vecShakeKeys;
    if (vecShakeKeys.empty())
        return;

    _vector vBasePos = m_tr.Get_StateXM(STATE::POSITION);
    _float4 vBaseQuatFloat4 = m_tr.Get_Rotation_Quaternion();
    _vector vBaseQuat = XMLoadFloat4(&vBaseQuatFloat4);

    _vector vShakeOffset = XMVectorZero();
    _vector vShakeRotEuler = XMVectorZero();

    for (const auto& tShake : vecShakeKeys)
    {
        if (fTime < tShake.fStartTime || fTime > tShake.fEndTime)
            continue;

        const _float fDuration = tShake.fEndTime - tShake.fStartTime;
        if (fDuration <= 0.f)
            continue;

        const _float fLocalTime = fTime - tShake.fStartTime;
        const _float fNormalized = std::clamp(fLocalTime / fDuration, 0.f, 1.f);

        _float fWeight = 1.f;
        if (fNormalized < 0.5f)
            fWeight = 2.f * fNormalized;
        else
            fWeight = 2.f * (1.f - fNormalized);

        fWeight = std::clamp(fWeight, 0.f, 1.f);

        const _float fWaveTime = fLocalTime * tShake.fFrequency;

        const _float fPosAmp = tShake.fAmplitudePos * fWeight;
        const _float fRotAmp = tShake.fAmplitudeRot * fWeight;

        _float fPosX = sinf(fWaveTime * 1.7f) * fPosAmp;
        _float fPosY = cosf(fWaveTime * 2.3f) * fPosAmp;
        _float fPosZ = sinf(fWaveTime * 1.1f + 1.37f) * fPosAmp;

        _float fRotPitch = sinf(fWaveTime * 2.1f) * fRotAmp;
        _float fRotYaw = cosf(fWaveTime * 1.6f) * fRotAmp;
        _float fRotRoll = sinf(fWaveTime * 2.8f + 0.7f) * fRotAmp;

        vShakeOffset += XMVectorSet(fPosX, fPosY, fPosZ, 0.f);
        vShakeRotEuler += XMVectorSet(
            XMConvertToRadians(fRotPitch),
            XMConvertToRadians(fRotYaw),
            XMConvertToRadians(fRotRoll),
            0.f
        );
    }

    vBasePos += vShakeOffset;

    _vector vShakeQuat = XMQuaternionRotationRollPitchYawFromVector(vShakeRotEuler);
    _vector vFinalQuat = XMQuaternionNormalize(XMQuaternionMultiply(vShakeQuat, vBaseQuat));

    m_tr.Set_Position(vBasePos);
    m_tr.Set_Rotation_Quaternion(vFinalQuat);
}
