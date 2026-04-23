#include "VFX_Manager.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CVFX_Manager::CVFX_Manager()
{
}

CVFX_Manager::~CVFX_Manager()
{
}

void CVFX_Manager::Awake(void* pCtx)
{
    m_goOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
}

void CVFX_Manager::Start(void* pCtx)
{
    CGameObject* pObject = GAME_INSTANCE.Find_GameObject(m_refRainObject.hObject);

    if (pObject != nullptr)
    {
        m_tRain.pObject = pObject;
        m_tRain.transform = m_tRain.pObject->Get_Component<CTransform>();
        m_tRain.meshRenderer = m_tRain.pObject->Get_Component<CMeshRenderer>();
        m_tRain.spriteEffect = m_tRain.pObject->Get_Component<CSpriteEffect>();
    }

    {
        Cache_Object(m_refHit01_0, m_vecHit01Pool);
        Cache_Object(m_refHit01_1, m_vecHit01Pool);

        Cache_Object(m_refHit02_0, m_vecHit02Pool);
        Cache_Object(m_refHit02_1, m_vecHit02Pool);

        Cache_Object(m_refHit03_0, m_vecHit03Pool);
        Cache_Object(m_refHit03_1, m_vecHit03Pool);

        Cache_Object(m_refHit04_0, m_vecHit04Pool);
        Cache_Object(m_refHit04_1, m_vecHit04Pool);

        Cache_Object(m_refHit05_0, m_vecHit05Pool);
        Cache_Object(m_refHit05_1, m_vecHit05Pool);
    }

    {
        Cache_Object(m_refSlideSpark_0, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_1, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_2, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_3, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_4, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_5, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_6, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_7, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_8, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_9, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_10, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_11, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_12, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_13, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_14, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_15, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_16, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_17, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_18, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_19, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_20, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_21, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_22, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_23, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_24, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_25, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_26, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_27, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_28, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_29, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_30, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_31, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_32, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_33, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_34, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_35, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_36, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_37, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_38, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_39, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_40, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_41, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_42, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_43, m_vecSlideSparkPool);
        Cache_Object(m_refSlideSpark_44, m_vecSlideSparkPool);
    }

    {
        Cache_Object(m_refFootDust_0, m_vecFootDustPool);
        Cache_Object(m_refFootDust_1, m_vecFootDustPool);
        Cache_Object(m_refFootDust_2, m_vecFootDustPool);
        Cache_Object(m_refFootDust_3, m_vecFootDustPool);
        Cache_Object(m_refFootDust_4, m_vecFootDustPool);
        Cache_Object(m_refFootDust_5, m_vecFootDustPool);
        Cache_Object(m_refFootDust_6, m_vecFootDustPool);
        Cache_Object(m_refFootDust_7, m_vecFootDustPool);
        Cache_Object(m_refFootDust_8, m_vecFootDustPool);
    }

    {
        Cache_Object(m_refSignalFlare_0, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_1, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_2, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_3, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_4, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_5, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_6, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_7, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_8, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_9, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_10, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_11, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_12, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_13, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_14, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_15, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_16, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_17, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_18, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_19, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_20, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_21, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_22, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_23, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_24, m_vecSignalFlarePool);
        Cache_Object(m_refSignalFlare_25, m_vecSignalFlarePool);
    }

    Set_Rain_Enable(m_bRainEnable);
}

void CVFX_Manager::Priority_Update(void* pCtx, _float fDT)
{
}

void CVFX_Manager::Update(void* pCtx, _float fDT)
{
}

void CVFX_Manager::Late_Update(void* pCtx, _float fDT)
{
}

void CVFX_Manager::Cache_Object(SCRIPT_OBJECT_REF& refObj, std::vector<VFX_OBJECT>& vecPool)
{
    CGameObject* pObject = GAME_INSTANCE.Find_GameObject(refObj.hObject);
    if (!pObject)
        return;

    VFX_OBJECT tObj{};
    tObj.pObject = pObject;
    tObj.transform = tObj.pObject->Get_Component<CTransform>();
    tObj.meshRenderer = tObj.pObject->Get_Component<CMeshRenderer>();
    tObj.spriteEffect = tObj.pObject->Get_Component<CSpriteEffect>();

    vecPool.push_back(tObj);
}

std::vector<VFX_OBJECT>* CVFX_Manager::Get_ParticlePool(PARTICLE_VFX eType)
{
    switch (eType)
    {
    case PARTICLE_VFX::SLIDE_SPARK:
        return &m_vecSlideSparkPool;

    case PARTICLE_VFX::FOOT_DUST:
        return &m_vecFootDustPool;
    case PARTICLE_VFX::SIGNAL_FLARE_SMOKE:
        return &m_vecSignalFlarePool;

    default:
        return nullptr;
    }
}

VFX_OBJECT* CVFX_Manager::Find_Available_Combat(COMBAT_VFX eType)
{
    std::vector<VFX_OBJECT>* pPool = nullptr;

    switch (eType)
    {
    case COMBAT_VFX::HIT_01: pPool = &m_vecHit01Pool; break;
    case COMBAT_VFX::HIT_02: pPool = &m_vecHit02Pool; break;
    case COMBAT_VFX::HIT_03: pPool = &m_vecHit03Pool; break;
    case COMBAT_VFX::HIT_04: pPool = &m_vecHit04Pool; break;
    case COMBAT_VFX::HIT_05: pPool = &m_vecHit05Pool; break;
    default: return nullptr;
    }

    for (auto& tObj : *pPool)
    {
        if (!tObj.spriteEffect.Is_Valid())
            continue;

        if (tObj.spriteEffect.Is_Finished() || !tObj.spriteEffect._Data()->bPlay)
            return &tObj;
    }

    if (pPool->empty())
        return nullptr;

    return &(*pPool)[0];
}

VFX_OBJECT* CVFX_Manager::Find_Available_Particle(PARTICLE_VFX eType)
{
    std::vector<VFX_OBJECT>* pPool = Get_ParticlePool(eType);
    if (pPool == nullptr)
        return nullptr;

    for (auto& tObj : *pPool)
    {
        if (!tObj.meshRenderer.Is_Valid())
            continue;

        if (!tObj.meshRenderer.Get_ParticlePlaying() || tObj.meshRenderer.Is_ParticleFinished())
            return &tObj;
    }

    if (pPool->empty())
        return nullptr;

    return &(*pPool)[0];
}

void CVFX_Manager::Play_Sprite(VFX_OBJECT* pVFX, const _float3& vWorldPos)
{
    if (pVFX == nullptr)
        return;

    if (!pVFX->transform.Is_Valid())
        return;

    if (!pVFX->spriteEffect.Is_Valid())
        return;

    pVFX->pObject->Set_Enable(true);
    pVFX->transform.Set_Position(XMLoadFloat3(&vWorldPos));

    pVFX->spriteEffect.Set_Loop(false);
    pVFX->spriteEffect.Play_From_Start();
}

void CVFX_Manager::Play_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos, const _float3* pPivot)
{
    if (pVFX == nullptr)
        return;

    if (!pVFX->transform.Is_Valid())
        return;

    if (!pVFX->meshRenderer.Is_Valid())
        return;

    pVFX->pObject->Set_Enable(true);
    pVFX->transform.Set_Position(XMLoadFloat3(&vWorldPos));

    if (pPivot != nullptr)
    {
        _float3 vLocalPivot =
        {
            pPivot->x - vWorldPos.x,
            pPivot->y - vWorldPos.y,
            pPivot->z - vWorldPos.z
        };

        pVFX->meshRenderer.Set_ParticlePivot(vLocalPivot);
    }
    else
    {
        pVFX->meshRenderer.Set_ParticlePivot(_float3(0.f, 0.f, 0.f));
    }

    pVFX->meshRenderer.Reset_Particle();
    pVFX->meshRenderer.Set_ParticlePlaying(true);
}

void CVFX_Manager::Play_CombatEffect(COMBAT_VFX eType, const _float3& vWorldPos)
{
    VFX_OBJECT* pVFX = Find_Available_Combat(eType);
    if (pVFX == nullptr)
        return;

    Play_Sprite(pVFX, vWorldPos);
}

VFX_OBJECT* CVFX_Manager::Start_Particle(PARTICLE_VFX eType, const _float3& vWorldPos)
{
    VFX_OBJECT* pVFX = Find_Available_Particle(eType);
    if (pVFX == nullptr)
        return nullptr;

    Play_Particle(pVFX, vWorldPos, nullptr);
    return pVFX;
}

VFX_OBJECT* CVFX_Manager::Start_Particle(PARTICLE_VFX eType, const _float3& vWorldPos, const _float3& vPivot)
{
    VFX_OBJECT* pVFX = Find_Available_Particle(eType);
    if (pVFX == nullptr)
        return nullptr;

    Play_Particle(pVFX, vWorldPos, &vPivot);
    return pVFX;
}

void CVFX_Manager::Update_Particle(VFX_OBJECT* pVFX, const _vector vWorldPos)
{
    _float3 vStartPos;
    XMStoreFloat3(&vStartPos, vWorldPos);
    Update_Particle(pVFX, vStartPos);
}

void CVFX_Manager::Update_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos)
{
    if (pVFX == nullptr)
        return;

    if (!pVFX->transform.Is_Valid())
        return;

    if (!pVFX->meshRenderer.Is_Valid())
        return;

    pVFX->transform.Set_Position(XMLoadFloat3(&vWorldPos));
    pVFX->meshRenderer.Set_ParticlePivot(_float3(0.f, 0.f, 0.f));
}

void CVFX_Manager::Update_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos, const _float3& vPivot)
{
    if (pVFX == nullptr)
        return;

    if (!pVFX->transform.Is_Valid())
        return;

    if (!pVFX->meshRenderer.Is_Valid())
        return;

    pVFX->transform.Set_Position(XMLoadFloat3(&vWorldPos));

    _float3 vLocalPivot =
    {
        vPivot.x - vWorldPos.x,
        vPivot.y - vWorldPos.y,
        vPivot.z - vWorldPos.z
    };

    pVFX->meshRenderer.Set_ParticlePivot(vLocalPivot);
    if (pVFX->meshRenderer.Is_ParticleFinished())
        pVFX->meshRenderer.Stop_Particle();
}

void CVFX_Manager::Finish_Particle(VFX_OBJECT* pVFX)
{
    if (pVFX == nullptr)
        return;

    if (!pVFX->meshRenderer.Is_Valid())
        return;

    pVFX->meshRenderer.Stop_Particle();
}

void CVFX_Manager::Play_ParticleBurst(PARTICLE_VFX eType, const _float3& vWorldPos)
{
    VFX_OBJECT* pVFX = Start_Particle(eType, vWorldPos);
    if (pVFX == nullptr)
        return;
}

void CVFX_Manager::Play_ParticleBurst(PARTICLE_VFX eType, const _float3& vWorldPos, const _float3& vPivot)
{
    VFX_OBJECT* pVFX = Start_Particle(eType, vWorldPos, vPivot);
    if (pVFX == nullptr)
        return;
}

void CVFX_Manager::Set_Rain_Enable(_bool bEnable)
{
    m_bRainEnable = bEnable;

    if (m_tRain.pObject == nullptr)
        return;

    m_tRain.pObject->Set_Enable(bEnable);

    if (m_tRain.meshRenderer.Is_Valid())
        m_tRain.meshRenderer.Set_ParticlePlaying(bEnable);
}

NS_END
