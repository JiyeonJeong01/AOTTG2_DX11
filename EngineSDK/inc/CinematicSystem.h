#pragma once
#include "Engine_Define.h"
#include "Cinematic_Define.h"

#include "Camera.h"
#include "Cinematic_Event.h"
#include "Event.h"
#include "Transform.h"

NS_BEGIN(Engine)

class CCamera;
class CTransform;

class ENGINE_DLL CCinematic_System final
{
    DECLARE_SINGLETON(CCinematic_System)

public:
    HRESULT Initialize();
    void Update(_float fDT);

public:
    bool Load(const std::string& strFileName);
    bool Play(CCamera camera);
    bool Play(const std::string& strClipName, CCamera camera);
    void Stop();
    void Pause(_bool bPause);
    void Reset_Cinematic();

    _bool Is_Playing() const { return m_bPlaying; }

    void Set_TestClip(const CINEMATIC_CLIP& tClip);
    _bool Preview(CCamera camera, _float fTime);

    CINEMATIC_CLIP&         Get_CurClip() { return m_tCurClip; }
    const CINEMATIC_CLIP&   Get_CurClip() const { return m_tCurClip; }
    _bool                   Set_CurClip(const std::string& strClipName);

private:
    void Evaluate(_float fTime);

    _bool   Try_Apply_ShotTrack(_float fTime);
    _bool   Apply_DefaultCameraFallback(_float fTime);
    void    Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tKey);
    void    Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tA, const CINEMATIC_CAMERA_KEY& tB, _float fRatio);
    void    Apply_ShotLookAt(const CINEMATIC_SHOT_KEY& tShot, CINEMATIC_CAMERA_KEY& tInOutKey);
    void    Apply_Shake(_float fTime);
    _float  Apply_Ease(CINEMATIC_EASE eEase, _float fRatio) const;

    int32_t Find_ActiveShotIndex(_float fTime) const;
    int32_t Find_PreviousValidShotIndex(int32_t iShotIndex) const;

    void    Process_EventKeys(_float fPrevTime, _float fCurTime);
    _float4 Make_LookAt_Quaternion(const _float3& vFromPos, const _float3& vLookAtPos) const;


    _float Get_ShotEndTime(int32_t iShotIndex) const;
    void Build_OrbitCameraKey(const CINEMATIC_SHOT_KEY& tShot, const CINEMATIC_CAMERA_KEY& tBaseKey, _float fOrbitRatio, CINEMATIC_CAMERA_KEY& tOutKey) const;

    void Invoke_CinematicEvent_Channel(const std::string& strClipName, CINEMATIC_EVENT_TYPE eType, const CINEMATIC_EVENT_DATA& tEventData);
private:
    std::unordered_map<std::string, CINEMATIC_CLIP>  m_umClips{};
    CINEMATIC_CLIP                              m_tCurClip{};

    _bool           m_bPlaying = false;
    _bool           m_bPause = false;

    _float          m_fPrevTime = 0.f;
    _float          m_fCurrentTime = 0.f;

    CCamera         m_camera;
    CTransform      m_tr;

    uint8_t         m_iPrevPriority = 0;

    /* 이벤트 */
    std::unordered_map<
        CINEMATIC_EVENT_CHANNEL_KEY,
        CEvent<const CINEMATIC_EVENT_DATA&>,
        CINEMATIC_EVENT_CHANNEL_KEY_HASH>    m_umCinematicEvents{};

    std::string                              m_strCurrentClipName{};

public :
    template <typename T>
    void Subscribe_CinematicEvent(
        const std::string& strClipName,
        CINEMATIC_EVENT_TYPE eType,
        void (T::* pFunc)(const CINEMATIC_EVENT_DATA&),
        T* pObj)
    {
        CINEMATIC_EVENT_CHANNEL_KEY tKey{};
        tKey.strClipName = strClipName;
        tKey.eType = eType;

        auto& Event = m_umCinematicEvents[tKey];
        Event.Add_Listener(pFunc, pObj);
    }

public:
    void Force_Shake(_float fDuration, _float fPower);
    void Stop_ForceShake();

private:
    _bool   m_bForceShake = false;
    _float  m_fForceShakeTime = 0.f;
    _float  m_fForceShakeDuration = 0.f;
    _float  m_fForceShakePower = 0.f;
};



NS_END
