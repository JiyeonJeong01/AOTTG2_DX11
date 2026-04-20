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
    void Stop();
    void Pause(_bool bPause);

    _bool Is_Playing() const { return m_bPlaying; }

    void Set_TestClip(const CINEMATIC_CLIP& tClip);
    _bool Preview(CCamera camera, _float fTime);

private:
private:
    void Evaluate(_float fTime);

    _bool Try_Apply_ShotTrack(_float fTime);
    _bool Apply_DefaultCameraFallback(_float fTime);

    int32_t Find_ActiveShotIndex(_float fTime) const;
    int32_t Find_PreviousValidShotIndex(int32_t iShotIndex) const;

    void Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tKey);
    void Apply_CameraKey(const CINEMATIC_CAMERA_KEY& tA, const CINEMATIC_CAMERA_KEY& tB, _float fRatio);

    void Process_EventKeys(_float fPrevTime, _float fCurTime);
    void Apply_Shake(_float fTime);


private:
    CINEMATIC_CLIP  m_tClip{};

    _bool           m_bLoaded = false;
    _bool           m_bPlaying = false;
    _bool           m_bPause = false;

    _float          m_fPrevTime = 0.f;
    _float          m_fCurrentTime = 0.f;

    CCamera         m_camera;
    CTransform      m_tr;

    uint8_t         m_iPrevPriority = 0;

    /* 이벤트 */

    CEvent<const CINEMATIC_EVENT_DATA&> OnCinematicEvent;
    CEvent<const CINEMATIC_EVENT_DATA&> OnCinematicFinished;
};

NS_END
