#include "Scout_Controller.h"

#include "AnimationClip_Player.h"

NS_BEGIN(Client)

void CScout_Controller::Awake(void* pCtx)
{
    Cache_Scouts();

    /* 기본은 꺼둬서 최적화 */
    Enable_Object(true);
}

void CScout_Controller::Start(void* pCtx)
{
}

void CScout_Controller::Priority_Update(void* pCtx, _float fDT)
{
}

void CScout_Controller::Update(void* pCtx, _float fDT)
{
}

void CScout_Controller::Late_Update(void* pCtx, _float fDT)
{
}

void CScout_Controller::Ready_Opening()
{
    Enable_Object(true);
}

void CScout_Controller::Finish_Opening()
{
    Enable_Object(false);
}

void CScout_Controller::Enable_Object(_bool bEnable)
{
    for (auto& tScout : m_vecStagingScouts)
    {
        if (tScout.pObject == nullptr)
            continue;

        tScout.pObject->Set_Enable(bEnable);
    }
}

void CScout_Controller::Play_Salute_Animation()
{
    for (auto& tScout : m_vecStagingScouts)
    {
        if (!tScout.anim.Is_Valid())
            continue;

        tScout.anim.Set_NextAnimationClip(ANIM_PLAYER::EMOTE_SALUTE);
    }
}

void CScout_Controller::Cache_Scouts()
{
    m_vecStagingScouts.clear();
    m_vecStagingScouts.reserve(NUM_SCOUT);

    for (size_t i = 0; i < NUM_SCOUT; ++i)
    {
        CGameObject* pObject = GAME_INSTANCE.Find_GameObject(m_refScouts[i].hObject);
        if (pObject == nullptr)
            continue;

        STAGING_SCOUT tScout{};
        tScout.pObject = pObject;
        tScout.tr = pObject->Get_Component<CTransform>();
        tScout.meshRenderer = pObject->Get_Component<CMeshRenderer>();
        tScout.anim = pObject->Get_Component<CAnimator>();

        m_vecStagingScouts.emplace_back(tScout);
    }
}

NS_END
