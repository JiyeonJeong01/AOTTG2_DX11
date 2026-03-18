#pragma once

#include "Client_Define.h"
#include "Line.h"
#include "Equipment_Struct.h"
#include "Event.h"

NS_BEGIN(Client)

class CRope final
{
public:
    enum class ROPE_STATE : uint8_t { IDLE, EXTENDING, ANCHORED, RETURNING };

public:
    void Initialize();
    void Update(_float fTimeDelta);
    void Render();

public:
    void Set_StartPoint(const _float3& vStartPoint);
    void Set_EndPoint(const _float3& vEndPoint);

    void Start_Extending(const _float3& vStartPoint, const _float3& vAnchorPoint);
    void Set_Anchored(const _float3& vStartPoint, const _float3& vAnchorPoint);
    void Start_Returning(const _float3& vStartPoint);
    void Stop();

    ROPE_STATE Get_State() const { return m_State; }

    void Set_RopeAmplitueInfo(const AMPLITUDE_VALUE& tInfo);

    template <typename T>
    ListenerID Subscribe_On_RopeState_Changed(void(T::* func)(ROPE_STATE), T* pInstance)
    {
        return m_OnChanged_RopeState.Add_Listener(func, pInstance);
    }

private:
    unique_ptr<Engine::CLine>       m_upLine;
    _float3                         m_vStartPoint{};
    _float3                         m_vEndPoint{};
    _float3                         m_vCurDynamicPos{};
    _float3                         m_vTrialDir{};
    ROPE_STATE                      m_State = ROPE_STATE::IDLE;

    std::vector<_float3>            m_RopePoints;
    AMPLITUDE_VALUE                 m_tDynamicValue{};

    /* Rope Line 설정 값 */
    _float                          m_vWaveHeight = 200.f;
    _int                            m_iNumWave = 2;
    _int                            m_iNumPoints = 30;
    _float                          m_fExtendVel = 200.f;

    CEvent<ROPE_STATE>              m_OnChanged_RopeState;
    

private:
    void Process_Extending(_float fTimeDelta);
    void Process_Anchored();
    void Process_Retuning(_float fTimeDelta);

    void Calc_RopeShape(const _float3& vCurTip, _float fTimeDelta);
    _bool Calc_RopeComplete(const _float3& vCurTip);
    void Upload_Line();

public:
    static std::unique_ptr<CRope> Create();
};

NS_END
