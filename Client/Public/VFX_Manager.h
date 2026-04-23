#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

enum class COMBAT_VFX : uint32_t
{
    HIT_01,
    HIT_02,
    HIT_03,
    HIT_04,
    HIT_05,
    END
};

enum class PARTICLE_VFX : uint32_t
{
    RAIN,
    SLIDE_SPARK,
    FOOT_DUST,
    SIGNAL_FLARE_SMOKE,
    END
};

class CVFX_Manager final : public IScript
{
public:
    CVFX_Manager();
    ~CVFX_Manager() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;


public:
    void Play_CombatEffect(COMBAT_VFX eType, const _float3& vWorldPos);

public:
    VFX_OBJECT* Start_Particle(PARTICLE_VFX eType, const _float3& vWorldPos);
    VFX_OBJECT* Start_Particle(PARTICLE_VFX eType, const _float3& vWorldPos, const _float3& vPivot);

    void Update_Particle(VFX_OBJECT* pVFX, const _vector vWorldPos);
    void Update_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos);
    void Update_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos, const _float3& vPivot);

    void Finish_Particle(VFX_OBJECT* pVFX);

public:
    void Play_ParticleBurst(PARTICLE_VFX eType, const _float3& vWorldPos);
    void Play_ParticleBurst(PARTICLE_VFX eType, const _float3& vWorldPos, const _float3& vPivot);

    void Set_Rain_Enable(_bool bEnable);

private:
    void Cache_Object(SCRIPT_OBJECT_REF& refObj, std::vector<VFX_OBJECT>& vecPool);

    VFX_OBJECT* Find_Available_Combat(COMBAT_VFX eType);
    VFX_OBJECT* Find_Available_Particle(PARTICLE_VFX eType);

    std::vector<VFX_OBJECT>* Get_ParticlePool(PARTICLE_VFX eType);

    void Play_Sprite(VFX_OBJECT* pVFX, const _float3& vWorldPos);
    void Play_Particle(VFX_OBJECT* pVFX, const _float3& vWorldPos, const _float3* pPivot);

private:
    CGameObject* m_goOwner = nullptr;
    VFX_OBJECT      m_vfxRain{};

private:
    /* rain */
    SCRIPT_OBJECT_REF m_refRainObject;

private:
    /* combat */
    SCRIPT_OBJECT_REF m_refHit01_0;
    SCRIPT_OBJECT_REF m_refHit01_1;

    SCRIPT_OBJECT_REF m_refHit02_0;
    SCRIPT_OBJECT_REF m_refHit02_1;

    SCRIPT_OBJECT_REF m_refHit03_0;
    SCRIPT_OBJECT_REF m_refHit03_1;

    SCRIPT_OBJECT_REF m_refHit04_0;
    SCRIPT_OBJECT_REF m_refHit04_1;

    SCRIPT_OBJECT_REF m_refHit05_0;
    SCRIPT_OBJECT_REF m_refHit05_1;

private:
    /* particle */
    SCRIPT_OBJECT_REF m_refSlideSpark_0;
    SCRIPT_OBJECT_REF m_refSlideSpark_1;
    SCRIPT_OBJECT_REF m_refSlideSpark_2;
    SCRIPT_OBJECT_REF m_refSlideSpark_3;
    SCRIPT_OBJECT_REF m_refSlideSpark_4;
    SCRIPT_OBJECT_REF m_refSlideSpark_5;
    SCRIPT_OBJECT_REF m_refSlideSpark_6;
    SCRIPT_OBJECT_REF m_refSlideSpark_7;
    SCRIPT_OBJECT_REF m_refSlideSpark_8;
    SCRIPT_OBJECT_REF m_refSlideSpark_9;
    SCRIPT_OBJECT_REF m_refSlideSpark_10;
    SCRIPT_OBJECT_REF m_refSlideSpark_11;
    SCRIPT_OBJECT_REF m_refSlideSpark_12;
    SCRIPT_OBJECT_REF m_refSlideSpark_13;
    SCRIPT_OBJECT_REF m_refSlideSpark_14;
    SCRIPT_OBJECT_REF m_refSlideSpark_15;
    SCRIPT_OBJECT_REF m_refSlideSpark_16;
    SCRIPT_OBJECT_REF m_refSlideSpark_17;
    SCRIPT_OBJECT_REF m_refSlideSpark_18;
    SCRIPT_OBJECT_REF m_refSlideSpark_19;
    SCRIPT_OBJECT_REF m_refSlideSpark_20;
    SCRIPT_OBJECT_REF m_refSlideSpark_21;
    SCRIPT_OBJECT_REF m_refSlideSpark_22;
    SCRIPT_OBJECT_REF m_refSlideSpark_23;
    SCRIPT_OBJECT_REF m_refSlideSpark_24;
    SCRIPT_OBJECT_REF m_refSlideSpark_25;
    SCRIPT_OBJECT_REF m_refSlideSpark_26;
    SCRIPT_OBJECT_REF m_refSlideSpark_27;
    SCRIPT_OBJECT_REF m_refSlideSpark_28;
    SCRIPT_OBJECT_REF m_refSlideSpark_29;
    SCRIPT_OBJECT_REF m_refSlideSpark_30;
    SCRIPT_OBJECT_REF m_refSlideSpark_31;
    SCRIPT_OBJECT_REF m_refSlideSpark_32;
    SCRIPT_OBJECT_REF m_refSlideSpark_33;
    SCRIPT_OBJECT_REF m_refSlideSpark_34;
    SCRIPT_OBJECT_REF m_refSlideSpark_35;
    SCRIPT_OBJECT_REF m_refSlideSpark_36;
    SCRIPT_OBJECT_REF m_refSlideSpark_37;
    SCRIPT_OBJECT_REF m_refSlideSpark_38;
    SCRIPT_OBJECT_REF m_refSlideSpark_39;
    SCRIPT_OBJECT_REF m_refSlideSpark_40;
    SCRIPT_OBJECT_REF m_refSlideSpark_41;
    SCRIPT_OBJECT_REF m_refSlideSpark_42;
    SCRIPT_OBJECT_REF m_refSlideSpark_43;
    SCRIPT_OBJECT_REF m_refSlideSpark_44;

    SCRIPT_OBJECT_REF m_refFootDust_0;
    SCRIPT_OBJECT_REF m_refFootDust_1;
    SCRIPT_OBJECT_REF m_refFootDust_2;
    SCRIPT_OBJECT_REF m_refFootDust_3;
    SCRIPT_OBJECT_REF m_refFootDust_4;
    SCRIPT_OBJECT_REF m_refFootDust_5;
    SCRIPT_OBJECT_REF m_refFootDust_6;
    SCRIPT_OBJECT_REF m_refFootDust_7;
    SCRIPT_OBJECT_REF m_refFootDust_8;
    SCRIPT_OBJECT_REF m_refFootDust_9;
    SCRIPT_OBJECT_REF m_refFootDust_10;
    SCRIPT_OBJECT_REF m_refFootDust_11;
    SCRIPT_OBJECT_REF m_refFootDust_12;
    SCRIPT_OBJECT_REF m_refFootDust_13;
    SCRIPT_OBJECT_REF m_refFootDust_14;
    SCRIPT_OBJECT_REF m_refFootDust_15;
    SCRIPT_OBJECT_REF m_refFootDust_16;
    SCRIPT_OBJECT_REF m_refFootDust_17;
    SCRIPT_OBJECT_REF m_refFootDust_18;
    SCRIPT_OBJECT_REF m_refFootDust_19;
    SCRIPT_OBJECT_REF m_refFootDust_20;
    SCRIPT_OBJECT_REF m_refFootDust_21;
    SCRIPT_OBJECT_REF m_refFootDust_22;
    SCRIPT_OBJECT_REF m_refFootDust_23;
    SCRIPT_OBJECT_REF m_refFootDust_24;
    SCRIPT_OBJECT_REF m_refFootDust_25;
    SCRIPT_OBJECT_REF m_refFootDust_26;
    SCRIPT_OBJECT_REF m_refFootDust_27;
    SCRIPT_OBJECT_REF m_refFootDust_28;
    SCRIPT_OBJECT_REF m_refFootDust_29;
    SCRIPT_OBJECT_REF m_refFootDust_30;

    SCRIPT_OBJECT_REF m_refSignalFlare_0;
    SCRIPT_OBJECT_REF m_refSignalFlare_1;
    SCRIPT_OBJECT_REF m_refSignalFlare_2;
    SCRIPT_OBJECT_REF m_refSignalFlare_3;
    SCRIPT_OBJECT_REF m_refSignalFlare_4;
    SCRIPT_OBJECT_REF m_refSignalFlare_5;
    SCRIPT_OBJECT_REF m_refSignalFlare_6;
    SCRIPT_OBJECT_REF m_refSignalFlare_7;
    SCRIPT_OBJECT_REF m_refSignalFlare_8;
    SCRIPT_OBJECT_REF m_refSignalFlare_9;
    SCRIPT_OBJECT_REF m_refSignalFlare_10;
    SCRIPT_OBJECT_REF m_refSignalFlare_11;
    SCRIPT_OBJECT_REF m_refSignalFlare_12;
    SCRIPT_OBJECT_REF m_refSignalFlare_13;
    SCRIPT_OBJECT_REF m_refSignalFlare_14;
    SCRIPT_OBJECT_REF m_refSignalFlare_15;
    SCRIPT_OBJECT_REF m_refSignalFlare_16;
    SCRIPT_OBJECT_REF m_refSignalFlare_17;
    SCRIPT_OBJECT_REF m_refSignalFlare_18;
    SCRIPT_OBJECT_REF m_refSignalFlare_19;
    SCRIPT_OBJECT_REF m_refSignalFlare_20;
    SCRIPT_OBJECT_REF m_refSignalFlare_21;
    SCRIPT_OBJECT_REF m_refSignalFlare_22;
    SCRIPT_OBJECT_REF m_refSignalFlare_23;
    SCRIPT_OBJECT_REF m_refSignalFlare_24;
    SCRIPT_OBJECT_REF m_refSignalFlare_25;

private:
    VFX_OBJECT m_tRain{};

    std::vector<VFX_OBJECT> m_vecHit01Pool;
    std::vector<VFX_OBJECT> m_vecHit02Pool;
    std::vector<VFX_OBJECT> m_vecHit03Pool;
    std::vector<VFX_OBJECT> m_vecHit04Pool;
    std::vector<VFX_OBJECT> m_vecHit05Pool;

    std::vector<VFX_OBJECT> m_vecSlideSparkPool;
    std::vector<VFX_OBJECT> m_vecFootDustPool;
    std::vector<VFX_OBJECT> m_vecSignalFlarePool;

private:
    _bool m_bRainEnable = true;

private:
    SCRIPT_FIELDS_BEGIN(CVFX_Manager)

        SCRIPT_FIELD_OBJECT_REF(m_refRainObject)

        SCRIPT_FIELD_OBJECT_REF(m_refHit01_0)
        SCRIPT_FIELD_OBJECT_REF(m_refHit01_1)

        SCRIPT_FIELD_OBJECT_REF(m_refHit02_0)
        SCRIPT_FIELD_OBJECT_REF(m_refHit02_1)

        SCRIPT_FIELD_OBJECT_REF(m_refHit03_0)
        SCRIPT_FIELD_OBJECT_REF(m_refHit03_1)

        SCRIPT_FIELD_OBJECT_REF(m_refHit04_0)
        SCRIPT_FIELD_OBJECT_REF(m_refHit04_1)

        SCRIPT_FIELD_OBJECT_REF(m_refHit05_0)
        SCRIPT_FIELD_OBJECT_REF(m_refHit05_1)

        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_0)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_1)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_2)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_3)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_4)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_5)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_6)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_7)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_8)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_9)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_10)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_11)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_12)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_13)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_14)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_15)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_16)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_17)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_18)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_19)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_20)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_21)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_22)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_23)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_24)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_25)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_26)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_27)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_28)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_29)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_30)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_31)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_32)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_33)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_34)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_35)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_36)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_37)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_38)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_39)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_40)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_41)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_42)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_43)
        SCRIPT_FIELD_OBJECT_REF(m_refSlideSpark_44)

        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_0)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_1)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_2)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_3)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_4)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_5)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_6)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_7)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_8)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_9)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_10)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_11)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_12)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_13)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_14)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_15)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_16)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_17)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_18)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_19)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_20)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_21)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_22)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_23)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_24)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_25)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_26)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_27)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_28)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_29)
        SCRIPT_FIELD_OBJECT_REF(m_refFootDust_30)

        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_0)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_1)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_2)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_3)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_4)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_5)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_6)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_7)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_8)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_9)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_10)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_11)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_12)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_13)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_14)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_15)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_16)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_17)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_18)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_19)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_20)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_21)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_22)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_23)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_24)
        SCRIPT_FIELD_OBJECT_REF(m_refSignalFlare_25)


        SCRIPT_FIELDS_END(CVFX_Manager)
};

NS_END
