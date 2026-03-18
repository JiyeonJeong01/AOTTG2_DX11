#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

typedef struct tagRopeAmplitude
{
    _float fGain = 0.f;
    _float fDamping = 0.f;
    _float fTarget = 0.f;
    _float fVelocity = 0.f;
    _float fValue = 0.f;

    _float Get_Value(_float fTimeDelta)
    {
        const _float fDiff = fTarget - fValue;
        const _float fSign = (fDiff >= 0.f) ? 1.f : -1.f;
        const _float fForce = fabsf(fDiff) * fGain;

        fVelocity += (fSign * fForce - fVelocity * fDamping) * fTimeDelta;
        fValue += fVelocity * fTimeDelta;

        return fValue;
    }

    void Reset()
    {
        fVelocity = 0.f;
        fValue = 0.f;
    }
} AMPLITUDE_VALUE;


NS_END
