#pragma once
#include "Client_Define.h"
#include "Cinematic_Event.h"
#include "Script.h"

NS_BEGIN(Client)

class CCinematicCamera_Director : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CCamera m_camCinematic{};
    _bool   m_bPlay = false;

private :
    SCRIPT_OBJECT_REF   m_refCinematicCam{};

    SCRIPT_FIELDS_BEGIN(CCinematicCamera_Director)
        SCRIPT_FIELD_OBJECT_REF(m_refCinematicCam)
    SCRIPT_FIELDS_END(CCinematicCamera_Director)

};

NS_END;
