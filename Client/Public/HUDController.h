#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)
class CUI_BladeController;
class CUI_GasController;
class CUI_SkillController;
NS_END

NS_BEGIN(Client)

class CHUDController : public IScript
{

private :
    CUI_BladeController*    m_pBlade = nullptr;
    CUI_GasController*      m_pGas = nullptr;
    CUI_SkillController*    m_pSkill = nullptr;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
