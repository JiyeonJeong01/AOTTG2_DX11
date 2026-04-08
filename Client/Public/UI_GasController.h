#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)
class CODM_Gear;
typedef struct tagPlayerContext PLAYER_CONTEXT;
NS_END

NS_BEGIN(Client)
class CUI_GasController : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public :
    void Bind_PlayerContext(const PLAYER_CONTEXT& tContext);

private :
    CCanvasRenderer    m_crLeftGasFill;
    CCanvasRenderer    m_crRightGasFill;

    CODM_Gear*          m_pGear{};

    _float              m_fLeftCurSizePx{};
    _float              m_fRightCurSizePx{};
    _float              m_fTotalSizePx = 184.f;

private :
    void                Update_GasState();

private :
    SCRIPT_OBJECT_REF   m_refLeftGasFill;
    SCRIPT_OBJECT_REF   m_refRightGasFill;

SCRIPT_FIELDS_BEGIN(CUI_GasController)
    SCRIPT_FIELD_OBJECT_REF(m_refLeftGasFill)
    SCRIPT_FIELD_OBJECT_REF(m_refRightGasFill)
SCRIPT_FIELDS_END(CUI_GasController)
};

NS_END;
