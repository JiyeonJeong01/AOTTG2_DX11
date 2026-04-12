#pragma once
#include "Titan_Struct.h"

NS_BEGIN(Client)

class CTitan_Scriptable_Object : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    const TITAN_SCRIPTABLE_OBJECT& Get_Data() const;
    CGameObject* Get_TitanObject() const;

private:
    void Sync_To_SO();

private:
    SCRIPT_OBJECT_REF           m_refTitan;
    SCRIPT_OBJECT_REF           m_refEren;
    TITAN_SCRIPTABLE_OBJECT     m_tSO{};

    /* editor expose fields */
    _float          m_fCurSpeed{};
    _float          m_fMaxSpeed{};

    _char           m_szMoveAnim[32]{};
    _char           m_szIdleAnim[32]{};

    _float          m_fMaxIdleTime{};
    _float          m_fMaxMoveTime{};

    CGameObject*    m_goEren{};

SCRIPT_FIELDS_BEGIN(CTitan_Scriptable_Object)
    SCRIPT_FIELD_OBJECT_REF(m_refTitan);
    SCRIPT_FIELD_OBJECT_REF(m_refEren);

    SCRIPT_FIELD_FLOAT(m_fCurSpeed);
    SCRIPT_FIELD_FLOAT(m_fMaxSpeed);

    SCRIPT_FIELD_CHAR(m_szMoveAnim);
    SCRIPT_FIELD_CHAR(m_szIdleAnim);

    SCRIPT_FIELD_FLOAT(m_fMaxIdleTime);
    SCRIPT_FIELD_FLOAT(m_fMaxMoveTime);
SCRIPT_FIELDS_END(CTitan_Scriptable_Object)
};

NS_END
