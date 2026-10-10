#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)
typedef struct tagPlayerContext PLAYER_CONTEXT;
typedef struct tagBladeDurability BLADE_DURABILITY;

class CUI_BladeController : public IScript
{
public:
    void Start(void* pCtx);

    void Late_Update(void* pCtx, _float fDT);

    void Bind_PlayerContext(const PLAYER_CONTEXT& tContext);

private :
    BLADE_DURABILITY*   m_pBlade{};
    CCanvasRenderer     m_crAtkRemainLeft;
    CCanvasRenderer     m_crAtkRemainRight;
    CCanvasRenderer     m_crRemain[8];

private :
    SCRIPT_OBJECT_REF   m_refAtkRemainLeft;
    SCRIPT_OBJECT_REF   m_refAtkRemainRight;
    SCRIPT_OBJECT_REF   m_refBladeRemain[8];

SCRIPT_FIELDS_BEGIN(CUI_BladeController)
    SCRIPT_FIELD_OBJECT_REF(m_refAtkRemainLeft)
    SCRIPT_FIELD_OBJECT_REF(m_refAtkRemainRight)
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[0])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[1])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[2])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[3])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[4])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[5])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[6])
    SCRIPT_FIELD_OBJECT_REF(m_refBladeRemain[7])
SCRIPT_FIELDS_END(CUI_BladeController)


};

NS_END;
