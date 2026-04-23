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
    m_bPlaying = false;
    m_bPause = false;
    m_fCurrentTime = 0.f;
    m_iPrevPriority = 0;

    m_strCurrentClipName.clear();

    return S_OK;
}

bool CCinematic_System::Load(const std::string& strFileName)
{
    std::filesystem::path pathFile(strFileName);
    std::string strClipName = pathFile.stem().string();

    CINEMATIC_CLIP newClip{};

    if (false == CCinematicIO::Load(strFileName, newClip))
        return false;

    m_umClips.erase(strClipName);
    auto [it, inserted] = m_umClips.insert({strClipName, std::move(newClip)});

    return inserted;
}

bool CCinematic_System::Play(CCamera camera)
{
    if (m_tCurClip.vecCameraKeys.empty())
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

    Evaluate(0.f);
    Apply_Shake(0.f);

    return true;
}

bool CCinematic_System::Play(const std::string& strClipName, CCamera camera)
{
    auto it = m_umClips.find(strClipName);
    if (it == m_umClips.end())
        return false;

    m_strCurrentClipName = strClipName;
    m_tCurClip = it->second;

    return Play(camera);
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

void CCinematic_System::Reset_Cinematic()
{
    Stop();
    m_strCurrentClipName.clear();
}

void CCinematic_System::Set_TestClip(const CINEMATIC_CLIP& tClip)
{
    Stop();
    m_tCurClip = tClip;
    m_strCurrentClipName = tClip.szName;
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

    const _float fClampedTime = std::clamp(fTime, 0.f, m_tCurClip.fDuration);

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

    if (m_tCurClip.vecCameraKeys.empty())
    {
        Stop();
        return;
    }

    m_fPrevTime = m_fCurrentTime;
    m_fCurrentTime += fDT;
    m_fCurrentTime = std::clamp(m_fCurrentTime, 0.f, m_tCurClip.fDuration);

    Evaluate(m_fCurrentTime);
    Apply_Shake(m_fCurrentTime);
    Process_EventKeys(m_fPrevTime, m_fCurrentTime);

    if (m_fCurrentTime >= m_tCurClip.fDuration)
    {
        CINEMATIC_EVENT_DATA tEventData{ "FINISH" };
        Invoke_CinematicEvent_Channel(m_strCurrentClipName, CINEMATIC_EVENT_TYPE::FINISH, tEventData);

        Stop();
    }
}

_bool CCinematic_System::Set_CurClip(const std::string& strClipName)
{
    auto it = m_umClips.find(strClipName);
    if (it == m_umClips.end())
        return false;

    m_strCurrentClipName = strClipName;
    m_tCurClip = it->second;

    return true;
}

void CCinematic_System::Evaluate(_float fTime)
{
    /* Shot이 있으면 Shot이 주도권을 가짐.
       Shot이 하나도 없거나, 적용 가능한 Shot이 없을 때만 CameraKey 기본 보간으로 fallback한다. */
    if (Try_Apply_ShotTrack(fTime))
        return;

    Apply_DefaultCameraFallback(fTime);
}

_bool CCinematic_System::Try_Apply_ShotTrack(_float fTime)
{
    const auto& vecShotKeys = m_tCurClip.vecShotKeys;
    const auto& vecCameraKeys = m_tCurClip.vecCameraKeys;

    if (vecShotKeys.empty())
        return false;

    const int32_t iActiveShotIndex = Find_ActiveShotIndex(fTime);
    if (iActiveShotIndex < 0 || iActiveShotIndex >= To<int32_t>(vecShotKeys.size()))
        return false;

    const auto& tShot = vecShotKeys[iActiveShotIndex];

    if (tShot.iCameraKeyIndex >= vecCameraKeys.size())
        return false;

    CINEMATIC_CAMERA_KEY tEval{};
    const auto& tShotBaseKey = vecCameraKeys[tShot.iCameraKeyIndex];

    /* Orbit Shot */
    if (tShot.bUseOrbit)
    {
        const _float fShotStart = tShot.fTime;
        const _float fShotEnd = Get_ShotEndTime(iActiveShotIndex);
        const _float fShotDuration = std::fmaxf(0.0001f, fShotEnd - fShotStart);

        CINEMATIC_CAMERA_KEY tOrbitStart{};
        Build_OrbitCameraKey(tShot, tShotBaseKey, 0.f, tOrbitStart);
        Apply_ShotLookAt(tShot, tOrbitStart);

        /* CUT Orbit */
        if (tShot.eType == CINEMATIC_SHOT_TYPE::CUT)
        {
            _float fOrbitRatio = (fTime - fShotStart) / fShotDuration;
            fOrbitRatio = Apply_Ease(tShotBaseKey.eEase, fOrbitRatio);

            Build_OrbitCameraKey(tShot, tShotBaseKey, fOrbitRatio, tEval);
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        /* Blend Orbit */
        if (tShot.eType == CINEMATIC_SHOT_TYPE::BLEND)
        {
            const int32_t iPrevShotIndex = Find_PreviousValidShotIndex(iActiveShotIndex);

            /* 이전 Shot이 없는 경우 */
            if (iPrevShotIndex < 0)
            {
                _float fOrbitRatio = (fTime - fShotStart) / fShotDuration;
                fOrbitRatio = Apply_Ease(tShotBaseKey.eEase, fOrbitRatio);

                Build_OrbitCameraKey(tShot, tShotBaseKey, fOrbitRatio, tEval);
                Apply_ShotLookAt(tShot, tEval);
                Apply_CameraKey(tEval);
                return true;
            }

            /* 현재 Shot이 마지막 키인 경우 */
            const auto& tPrevShot = vecShotKeys[iPrevShotIndex];
            if (tPrevShot.iCameraKeyIndex >= vecCameraKeys.size())
            {
                _float fOrbitRatio = (fTime - fShotStart) / fShotDuration;
                fOrbitRatio = Apply_Ease(tShotBaseKey.eEase, fOrbitRatio);

                Build_OrbitCameraKey(tShot, tShotBaseKey, fOrbitRatio, tEval);
                Apply_ShotLookAt(tShot, tEval);
                Apply_CameraKey(tEval);
                return true;
            }

            /* 이전 Cam Key랑 보간 시작 */
            const auto& tPrevKey = vecCameraKeys[tPrevShot.iCameraKeyIndex];

            const _float fBlendDuration = std::fmaxf(0.f, tShot.fBlendDuration);
            const _float fBlendEnd = fShotStart + fBlendDuration;

            if (fBlendDuration > 0.f && fTime < fBlendEnd)
            {
                _float fBlendRatio = (fTime - fShotStart) / fBlendDuration;
                fBlendRatio = Apply_Ease(tPrevKey.eEase, fBlendRatio);

                _vector vPosA = XMLoadFloat3(&tPrevKey.vPosition);
                _vector vPosB = XMLoadFloat3(&tOrbitStart.vPosition);

                _vector vQuatA = XMLoadFloat4(&tPrevKey.vRotationQuat);
                _vector vQuatB = XMLoadFloat4(&tOrbitStart.vRotationQuat);

                _vector vPos = XMVectorLerp(vPosA, vPosB, fBlendRatio);
                _vector vRot = XMQuaternionSlerp(vQuatA, vQuatB, fBlendRatio);

                XMStoreFloat3(&tEval.vPosition, vPos);
                XMStoreFloat4(&tEval.vRotationQuat, XMQuaternionNormalize(vRot));

                tEval.fTime = fTime;
                tEval.fFovy = tPrevKey.fFovy + (tOrbitStart.fFovy - tPrevKey.fFovy) * fBlendRatio;
                tEval.eEase = tOrbitStart.eEase;

                Apply_CameraKey(tEval);
                return true;
            }

            const _float fOrbitStartTime = fBlendEnd;
            const _float fOrbitDuration = std::fmaxf(0.0001f, fShotEnd - fOrbitStartTime);

            _float fOrbitRatio = (fTime - fOrbitStartTime) / fOrbitDuration;
            fOrbitRatio = Apply_Ease(tShotBaseKey.eEase, fOrbitRatio);

            Build_OrbitCameraKey(tShot, tShotBaseKey, fOrbitRatio, tEval);
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;

        }
    }


    /* CUT : 해당 Shot 시점부터 다음 Shot 전까지 지정 CameraKey 유지 */
    if (tShot.eType == CINEMATIC_SHOT_TYPE::CUT)
    {
        tEval = vecCameraKeys[tShot.iCameraKeyIndex];
        Apply_ShotLookAt(tShot, tEval);
        Apply_CameraKey(tEval);
        return true;
    }

    /* BLEND : Shot 시작 ~ 종료 동안 이전 Shot 카메라 -> 현재 Shot 카메라 보간
       종료 후에는 현재 Shot 카메라 유지 */
    if (tShot.eType == CINEMATIC_SHOT_TYPE::BLEND)
    {
        const int32_t iPrevShotIndex = Find_PreviousValidShotIndex(iActiveShotIndex);

        /* 이전 Shot이 없으면 시작점부터 대상 카메라 유지 */
        if (iPrevShotIndex < 0)
        {
            tEval = vecCameraKeys[tShot.iCameraKeyIndex];
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        const auto& tPrevShot = vecShotKeys[iPrevShotIndex];

        if (tPrevShot.iCameraKeyIndex >= vecCameraKeys.size())
        {
            tEval = vecCameraKeys[tShot.iCameraKeyIndex];
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        const auto& tFrom = vecCameraKeys[tPrevShot.iCameraKeyIndex];
        const auto& tTo = vecCameraKeys[tShot.iCameraKeyIndex];

        if (tShot.fBlendDuration <= 0.f)
        {
            tEval = tTo;
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        const _float fBlendStart = tShot.fTime;
        const _float fBlendEnd = tShot.fTime + tShot.fBlendDuration;

        if (fTime <= fBlendStart)
        {
            tEval = tFrom;
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        if (fTime >= fBlendEnd)
        {
            tEval = tTo;
            Apply_ShotLookAt(tShot, tEval);
            Apply_CameraKey(tEval);
            return true;
        }

        _float fRatio = (fTime - fBlendStart) / tShot.fBlendDuration;
        fRatio = std::clamp(fRatio, 0.f, 1.f);

        fRatio = Apply_Ease(tFrom.eEase, fRatio);

        _vector vPosA = XMLoadFloat3(&tFrom.vPosition);
        _vector vPosB = XMLoadFloat3(&tTo.vPosition);

        _vector vQuatA = XMLoadFloat4(&tFrom.vRotationQuat);
        _vector vQuatB = XMLoadFloat4(&tTo.vRotationQuat);

        _vector vPos = XMVectorLerp(vPosA, vPosB, fRatio);
        _vector vRot = XMQuaternionSlerp(vQuatA, vQuatB, fRatio);

        XMStoreFloat3(&tEval.vPosition, vPos);
        XMStoreFloat4(&tEval.vRotationQuat, XMQuaternionNormalize(vRot));

        tEval.fTime = fTime;
        tEval.fFovy = tFrom.fFovy + (tTo.fFovy - tFrom.fFovy) * fRatio;
        tEval.eEase = tTo.eEase;

        Apply_ShotLookAt(tShot, tEval);
        Apply_CameraKey(tEval);
        return true;
    }

    return false;
}

_bool CCinematic_System::Apply_DefaultCameraFallback(_float fTime)
{
    auto& vecKeys = m_tCurClip.vecCameraKeys;

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

        fRatio = Apply_Ease(tA.eEase, fRatio);
        Apply_CameraKey(tA, tB, fRatio);
        return true;
    }

    return false;
}

int32_t CCinematic_System::Find_ActiveShotIndex(_float fTime) const
{
    const auto& vecShotKeys = m_tCurClip.vecShotKeys;

    if (vecShotKeys.empty())
        return -1;

    int32_t iActiveShotIndex = -1;

    /* e.g., fTime = 2.f이고 vecshotkeys[5] = 2.3f라면 index는 4 */
    for (int32_t i = 0; i < To<int32_t>(vecShotKeys.size()); ++i)
    {
        if (vecShotKeys[i].fTime > fTime)
            break;

        iActiveShotIndex = i;
    }

    return iActiveShotIndex;
}

int32_t CCinematic_System::Find_PreviousValidShotIndex(int32_t iShotIndex) const
{
    const auto& vecShotKeys = m_tCurClip.vecShotKeys;

    for (int32_t i = iShotIndex - 1; i >= 0; --i)
    {
        if (vecShotKeys[i].iCameraKeyIndex < m_tCurClip.vecCameraKeys.size())
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

void CCinematic_System::Apply_ShotLookAt(const CINEMATIC_SHOT_KEY& tShot, CINEMATIC_CAMERA_KEY& tInOutKey)
{
    _bool bShouldLookAt = false;
    _float3 vLookAtPos = { 0.f, 0.f, 0.f };
    _float fBlendRatio = 1.f;

    if (tShot.bUseLookAt)
    {
        bShouldLookAt = true;
        vLookAtPos = tShot.vLookAtPosition;
        fBlendRatio = std::clamp(tShot.fLookAtBlendRatio, 0.f, 1.f);
    }
    else if (tShot.bUseOrbit)
    {
        /* Orbit면 기본적으로 중심을 바라보게 */
        bShouldLookAt = true;
        vLookAtPos = tShot.vOrbitCenter;
        fBlendRatio = 1.f;
    }

    if (false == bShouldLookAt)
        return;

    _float4 vLookQuat = Make_LookAt_Quaternion(tInOutKey.vPosition, vLookAtPos);

    _vector vBaseQuat = XMLoadFloat4(&tInOutKey.vRotationQuat);
    _vector vTargetQuat = XMLoadFloat4(&vLookQuat);

    _vector vFinalQuat = XMQuaternionSlerp(vBaseQuat, vTargetQuat, fBlendRatio);
    vFinalQuat = XMQuaternionNormalize(vFinalQuat);

    XMStoreFloat4(&tInOutKey.vRotationQuat, vFinalQuat);
}


_float CCinematic_System::Apply_Ease(CINEMATIC_EASE eEase, _float fRatio) const
{
    fRatio = std::clamp(fRatio, 0.f, 1.f);

    switch (eEase)
    {
    case CINEMATIC_EASE::EASE_IN:
        return fRatio * fRatio;

    case CINEMATIC_EASE::EASE_OUT:
        return 1.f - (1.f - fRatio) * (1.f - fRatio);

    case CINEMATIC_EASE::EASE_IN_OUT:
        if (fRatio < 0.5f)
            return 2.f * fRatio * fRatio;
        else
            return 1.f - powf(-2.f * fRatio + 2.f, 2.f) * 0.5f;

    case CINEMATIC_EASE::LINEAR:
    default:
        return fRatio;
    }
}

_float4 CCinematic_System::Make_LookAt_Quaternion(const _float3& vFromPos, const _float3& vLookAtPos) const
{
    const _vector vEye = XMLoadFloat3(&vFromPos);
    const _vector vAt = XMLoadFloat3(&vLookAtPos);
    const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vDir = XMVector3Normalize(vAt - vEye);

    if (XMVectorGetX(XMVector3LengthSq(vDir)) <= 0.000001f)
    {
        return _float4(0.f, 0.f, 0.f, 1.f);
    }

    _matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);
    _matrix matWorld = XMMatrixInverse(nullptr, matView);

    _vector vQuat = XMQuaternionRotationMatrix(matWorld);
    vQuat = XMQuaternionNormalize(vQuat);

    _float4 vOut{};
    XMStoreFloat4(&vOut, vQuat);
    return vOut;
}

void CCinematic_System::Process_EventKeys(_float fPrevTime, _float fCurTime)
{
    for (const auto& tEventKey : m_tCurClip.vecEventKeys)
    {
        if (tEventKey.fTime <= fPrevTime)
            continue;

        if (tEventKey.fTime > fCurTime)
            continue;

        CINEMATIC_EVENT_DATA tEventData{ tEventKey.szEventName };
        Invoke_CinematicEvent_Channel(m_strCurrentClipName, tEventKey.eType, tEventData);
    }
}

void CCinematic_System::Apply_Shake(_float fTime)
{
    if (false == m_tr.Is_Valid())
        return;

    const auto& vecShakeKeys = m_tCurClip.vecShakeKeys;
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

_float CCinematic_System::Get_ShotEndTime(int32_t iShotIndex) const
{
    const auto& vecShotKeys = m_tCurClip.vecShotKeys;

    if (iShotIndex < 0 || iShotIndex >= To<int32_t>(vecShotKeys.size()))
        return m_tCurClip.fDuration;

    if (iShotIndex + 1 < To<int32_t>(vecShotKeys.size()))
        return vecShotKeys[iShotIndex + 1].fTime;

    return m_tCurClip.fDuration;
}

void CCinematic_System::Build_OrbitCameraKey(
    const CINEMATIC_SHOT_KEY& tShot,
    const CINEMATIC_CAMERA_KEY& tBaseKey,
    _float fOrbitRatio,
    CINEMATIC_CAMERA_KEY& tOutKey) const
{
    tOutKey = tBaseKey;

    fOrbitRatio = std::clamp(fOrbitRatio, 0.f, 1.f);

    const _float fAngleDeg = tShot.fOrbitStartAngleDeg + tShot.fOrbitSweepAngleDeg * fOrbitRatio;
    const _float fAngleRad = XMConvertToRadians(fAngleDeg);

    const _float fHeightOffset =
        tShot.fOrbitStartHeightOffset +
        (tShot.fOrbitEndHeightOffset - tShot.fOrbitStartHeightOffset) * fOrbitRatio;

    tOutKey.vPosition.x = tShot.vOrbitCenter.x + cosf(fAngleRad) * tShot.fOrbitRadius;
    tOutKey.vPosition.y = tShot.vOrbitCenter.y + fHeightOffset;
    tOutKey.vPosition.z = tShot.vOrbitCenter.z + sinf(fAngleRad) * tShot.fOrbitRadius;
}

void CCinematic_System::Invoke_CinematicEvent_Channel(
    const std::string& strClipName,
    CINEMATIC_EVENT_TYPE eType,
    const CINEMATIC_EVENT_DATA& tEventData)
{
    CINEMATIC_EVENT_CHANNEL_KEY tKey{};
    tKey.strClipName = strClipName;
    tKey.eType = eType;

    auto it = m_umCinematicEvents.find(tKey);
    if (it == m_umCinematicEvents.end())
        return;

    it->second.Invoke(tEventData);
}
