#include "Animator.h"

#include "Animator_Processor.h"

NS_BEGIN(Engine)

void CAnimator::Set_Loop(_bool bLoop, const std::string& strAnimClip)
{
    if (!m_pData)
        return;

    if (m_pData->NameToClipIndex.size() == 0 && m_pData->pAnimator_Processor)
        m_pData->pAnimator_Processor->Try_Build_ClipNameMap(m_pData);

    if (m_pData->NameToClipIndex.size() == 0)
        return;

    auto it = m_pData->NameToClipIndex.find(strAnimClip);
    if (it == m_pData->NameToClipIndex.end())
        return;

    uint32_t iClip = it->second;

    Set_Loop(bLoop, iClip);
}

void CAnimator::Set_Loop(_bool bLoop, uint32_t iAnimClip)
{
    if (m_pData->pAnimator_Processor)
        m_pData->pAnimator_Processor->Set_Loop(m_pData, iAnimClip, bLoop);
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
    if (!_Data())
        return;

    const _bool bRewind = (fTrackPosition < m_pData->fTrackPosition);

    m_pData->fTrackPosition = fTrackPosition;

    if (bRewind)
    {
        for (uint32_t& iKeyFrameIndex : m_pData->currentKeyFrameIndices)
            iKeyFrameIndex = 0;
    }
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

    if (m_pData->NameToClipIndex.size() == 0)
        m_pData->pAnimator_Processor->Try_Build_ClipNameMap(m_pData);

    if (m_pData->NameToClipIndex.size() == 0)
        return;

    /* 현재 클립과 같을 때 */
    if (m_pData->iAnimationClip == iNextAnimClip)
    {
        /* 이미 끝난 클립을 다시 재생하려는 경우 */
        if (!m_pData->bPlaying)
        {
            m_pData->iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;
            m_pData->fBlendElapsed = 0.f;
            m_pData->fBlendDuration = 0.f;
            m_pData->fTrackPosition = 0.f;
            m_pData->bPlaying = true;
            cout << RED << "bPlaying false -> INVALID_ANIM_CLIP_INDEX is playing " << RESET << endl;
        }
        else
        {
            /* 그냥 같은 클립 유지 요청이면 예약만 취소 */
            m_pData->iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;
            m_pData->fBlendElapsed = 0.f;
            m_pData->fBlendDuration = 0.f;
        }
        return;
    }

    /* 이미 같은 예약이 걸려 있으면 무시 */
    if (m_pData->iNextAnimationClip == iNextAnimClip)
        return;

    m_pData->fBlendElapsed = 0.f;
    m_pData->iNextAnimationClip = iNextAnimClip;

    m_pData->fBlendDuration = m_pData->pAnimator_Processor->Get_BlendDuration(
        m_pData,
        m_pData->iAnimationClip,
        iNextAnimClip);

    DEBUG_POINT;
}

_uint CAnimator::Get_CurAnimaionClipIdx() const
{
    if (!m_pData)
        return INVALID_ANIM_CLIP_INDEX;

    return m_pData->iAnimationClip;
}

_uint CAnimator::Get_AnimationClipIdx_By_Name(const std::string& strName) const
{
    if (!m_pData)
        return INVALID_ANIM_CLIP_INDEX;

    if (!m_pData->pAnimator_Processor)
        return INVALID_ANIM_CLIP_INDEX;

    if (m_pData->NameToClipIndex.size() == 0)
        m_pData->pAnimator_Processor->Try_Build_ClipNameMap(m_pData);

    if (m_pData->NameToClipIndex.size() == 0)
        return INVALID_ANIM_CLIP_INDEX;

    auto it = m_pData->NameToClipIndex.find(strName);
    if (it == m_pData->NameToClipIndex.end())
        return INVALID_ANIM_CLIP_INDEX;

    return it->second;
    int a = 10;
}

const std::string& CAnimator::Get_Name_By_AnimationCliIdx(_uint iIdx) const
{
    if (!m_pData)
        return NONE_STR;

    for (const auto& pair : m_pData->NameToClipIndex)
    {
        if (pair.second == iIdx)
            return pair.first;
    }
    return NONE_STR;
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
