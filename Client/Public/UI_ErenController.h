#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CErenTitan;

class CUI_ErenController : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

    void On_ErenDamaged(_float fCurLife);

private:
    SCRIPT_OBJECT_REF   m_refEren{};
    SCRIPT_OBJECT_REF   m_refLifeFill{};
    SCRIPT_OBJECT_REF   m_refLifeDelayFill{};

    CErenTitan* m_pErenTitan = nullptr;

    CCanvasRenderer     m_crLifeFill;
    CCanvasRenderer     m_crLifeDelayFill;

private:
    _float              m_fMaxLife = 100.f;                 /* 에렌 최대 체력 */
    _float              m_fTargetLife = 100.f;              /* 실제 체력값 */
    _float              m_fCurrentLife = 100.f;             /* 즉시 반영 바 */
    _float              m_fDelayLife = 100.f;               /* 지연 연출 바 */

    _float              m_fTotalSizePx = 361.f;             /* 전체 길이, 이후 인스펙터에서 조절 */
    _float              m_fCurClipRightPx = 0.f;            /* 현재 체력 바 clip right */
    _float              m_fDelayClipRightPx = 0.f;          /* 지연 체력 바 clip right */

    _float              m_fDelayWaitTime = 0.2f;            /* 데미지 후 지연 바 대기 시간 */
    _float              m_fDelayElapsed = 0.f;              /* 대기 타이머 */
    _float              m_fDelayLerpSpeed = 2.5f;           /* 지연 바 추적 속도 */

private:
    void Apply_LifeClipRect();
    _float Ease_OutCubic(_float t) const;

SCRIPT_FIELDS_BEGIN(CUI_ErenController)
    SCRIPT_FIELD_OBJECT_REF(m_refEren)
    SCRIPT_FIELD_OBJECT_REF(m_refLifeFill)
    SCRIPT_FIELD_OBJECT_REF(m_refLifeDelayFill)

    SCRIPT_FIELD_FLOAT(m_fMaxLife)
    SCRIPT_FIELD_FLOAT(m_fTotalSizePx)
    SCRIPT_FIELD_FLOAT(m_fDelayWaitTime)
    SCRIPT_FIELD_FLOAT(m_fDelayLerpSpeed)
SCRIPT_FIELDS_END(CUI_ErenController)
};

NS_END;
