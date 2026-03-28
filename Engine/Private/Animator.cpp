#include "Animator.h"

#include "Animator_Processor.h"

NS_BEGIN(Engine)

void CAnimator::Set_Loop(_bool bLoop)
{
    if (!m_pData)
        return;

    m_pData->bLoop = bLoop;
}

_bool CAnimator::Is_Playing() const
{
    if (!m_pData)
        return false;

    return m_pData->bPlaying;
}

void CAnimator::Set_Playing(_bool bPlaying)
{
    if (!m_pData)
        return;

    m_pData->bPlaying = bPlaying;
}

void CAnimator::Play()
{
    if (!m_pData)
        return;

    m_pData->bPlaying = true;
}

void CAnimator::Pause()
{
    if (!m_pData)
        return;

    m_pData->bPlaying = false;
}

void CAnimator::Stop()
{
    if (!m_pData)
        return;

    int a = 1;

    m_pData->bPlaying = false;
    m_pData->fTrackPosition = 0.f;
    Reset_CurrentKeyFrameIndices();
}

_float CAnimator::Get_TrackPosition() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fTrackPosition;
}

void CAnimator::Set_TrackPosition(_float fTrackPosition)
{
    if (!m_pData)
        return;

    if (fTrackPosition < 0.f)
        fTrackPosition = 0.f;

    m_pData->fTrackPosition = fTrackPosition;
}

void CAnimator::Add_TrackPosition(_float fDeltaTrackPosition)
{
    if (!m_pData)
        return;

    m_pData->fTrackPosition += fDeltaTrackPosition;

    if (m_pData->fTrackPosition < 0.f)
        m_pData->fTrackPosition = 0.f;
}

_float CAnimator::Get_PlaySpeed() const
{
    if (!m_pData)
        return 1.f;

    return m_pData->fPlaySpeed;
}

void CAnimator::Set_PlaySpeed(_float fPlaySpeed)
{
    if (!m_pData)
        return;

    if (fPlaySpeed < 0.f)
        fPlaySpeed = 0.f;

    m_pData->fPlaySpeed = fPlaySpeed;
}

void CAnimator::Set_NextAnimationClip(const std::string& strNextAnimClip)
{
    if (!m_pData)
        return;

    if (!m_pData->pAnimator_Processor)
        return;

    if (m_pData->NameToClipIndex.size() == 0)
        m_pData->pAnimator_Processor->Try_Build_ClipNameMap(m_pData);

    if (m_pData->NameToClipIndex.size() == 0)
        return;

    auto it = m_pData->NameToClipIndex.find(strNextAnimClip);
    if (it == m_pData->NameToClipIndex.end())
        return ;

    uint32_t iNextAnimClip = it->second;

    Set_NextAnimationClip(iNextAnimClip);
}

void CAnimator::Set_NextAnimationClip(uint32_t iNextAnimClip)
{
    if (!m_pData)
        return;

    if (!m_pData->pAnimator_Processor)
        return;

    if (m_pData->iAnimationClip == iNextAnimClip)
        return;

    if (m_pData->NameToClipIndex.size() == 0)
        m_pData->pAnimator_Processor->Try_Build_ClipNameMap(m_pData);

    if (m_pData->NameToClipIndex.size() == 0)
        return;

    m_pData->fBlendElapsed = 0.f;
    m_pData->iNextAnimationClip = iNextAnimClip;
    m_pData->bIsBlending = true;

    m_pData->fBlendDuration = m_pData->pAnimator_Processor->Get_BlendDuration(
        m_pData,
        m_pData->iAnimationClip,
        iNextAnimClip);
}

void CAnimator::Reset_CurrentKeyFrameIndices()
{
    DataType* m_pData = _Data();
    if (!m_pData)
        return;

    for (uint32_t& iCurrentKeyFrameIndex : m_pData->currentKeyFrameIndices)
    {
        iCurrentKeyFrameIndex = 0;
    }
}

void CAnimator::Reset_CurrentKeyFrameIndices(size_t iChannelCount)
{
    DataType* m_pData = _Data();
    if (!m_pData)
        return;

    m_pData->currentKeyFrameIndices.assign(iChannelCount, 0);
}


NS_END
