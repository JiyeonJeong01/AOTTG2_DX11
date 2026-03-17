#include "Animator.h"

NS_BEGIN(Engine)

void CAnimator::Set_Loop(_bool bLoop)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->bLoop = bLoop;
}

_bool CAnimator::Is_Playing() const
{
    const DataType* pData = _Data();
    if (!pData)
        return false;

    return pData->bPlaying;
}

void CAnimator::Set_Playing(_bool bPlaying)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->bPlaying = bPlaying;
}

void CAnimator::Play()
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->bPlaying = true;
}

void CAnimator::Pause()
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->bPlaying = false;
}

void CAnimator::Stop()
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->bPlaying = false;
    pData->fTrackPosition = 0.f;
    Reset_CurrentKeyFrameIndices();
}

_float CAnimator::Get_TrackPosition() const
{
    const DataType* pData = _Data();
    if (!pData)
        return 0.f;

    return pData->fTrackPosition;
}

void CAnimator::Set_TrackPosition(_float fTrackPosition)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    if (fTrackPosition < 0.f)
        fTrackPosition = 0.f;

    pData->fTrackPosition = fTrackPosition;
}

void CAnimator::Add_TrackPosition(_float fDeltaTrackPosition)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->fTrackPosition += fDeltaTrackPosition;

    if (pData->fTrackPosition < 0.f)
        pData->fTrackPosition = 0.f;
}

_float CAnimator::Get_PlaySpeed() const
{
    const DataType* pData = _Data();
    if (!pData)
        return 1.f;

    return pData->fPlaySpeed;
}

void CAnimator::Set_PlaySpeed(_float fPlaySpeed)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    if (fPlaySpeed < 0.f)
        fPlaySpeed = 0.f;

    pData->fPlaySpeed = fPlaySpeed;
}

void CAnimator::Reset_CurrentKeyFrameIndices()
{
    DataType* pData = _Data();
    if (!pData)
        return;

    for (uint32_t& iCurrentKeyFrameIndex : pData->currentKeyFrameIndices)
    {
        iCurrentKeyFrameIndex = 0;
    }
}

void CAnimator::Reset_CurrentKeyFrameIndices(size_t iChannelCount)
{
    DataType* pData = _Data();
    if (!pData)
        return;

    pData->currentKeyFrameIndices.assign(iChannelCount, 0);
}


NS_END
