#include "Sound_System.h"
#include <io.h>

IMPLEMENT_SINGLETON(CSound_System)

CSound_System::CSound_System() {}
CSound_System::~CSound_System() {}

HRESULT CSound_System::Initialize()
{
    FMOD_System_Create(&m_pSystem, FMOD_VERSION);
    FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

    LoadSoundFiles(L"../../Client/Bin/Assets/Sounds/");

    return S_OK;
}

/**
 * \brief 해당 채널에서 이미 재생 중이면 재생 안 함
 */
void CSound_System::PlaySFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    FMOD_BOOL bPlay = FALSE;

    if (m_pChannelArr[eID])
        FMOD_Channel_IsPlaying(m_pChannelArr[eID], &bPlay);

    if (bPlay)
        return;

    FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, FALSE, &m_pChannelArr[eID]);

    if (m_pChannelArr[eID])
        FMOD_Channel_SetVolume(m_pChannelArr[eID], fVolume);

    FMOD_System_Update(m_pSystem);
}

/**
 * \brief 해당 채널에서 뭐가 재생 중이든 새로 재생
 */
void CSound_System::PlayForceSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    if (m_pChannelArr[eID])
        FMOD_Channel_Stop(m_pChannelArr[eID]);

    FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, FALSE, &m_pChannelArr[eID]);

    if (m_pChannelArr[eID])
    {
        FMOD_Channel_SetMode(m_pChannelArr[eID], FMOD_LOOP_OFF);
        FMOD_Channel_SetVolume(m_pChannelArr[eID], fVolume);
    }

    FMOD_System_Update(m_pSystem);
}

/**
 * \brief 해당 채널에서 이미 재생 중이면 루프 재생 안 함
 */
void CSound_System::PlayLoopSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    FMOD_BOOL bPlay = FALSE;

    if (m_pChannelArr[eID])
        FMOD_Channel_IsPlaying(m_pChannelArr[eID], &bPlay);

    if (bPlay)
        return;

    FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, TRUE, &m_pChannelArr[eID]);

    if (!m_pChannelArr[eID])
        return;

    FMOD_Channel_SetMode(m_pChannelArr[eID], FMOD_LOOP_NORMAL);
    FMOD_Channel_SetVolume(m_pChannelArr[eID], fVolume);
    FMOD_Channel_SetPaused(m_pChannelArr[eID], FALSE);

    FMOD_System_Update(m_pSystem);
}

/**
 * \brief 해당 채널을 끊고 루프를 새로 시작
 */
void CSound_System::PlayForceLoopSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    if (m_pChannelArr[eID])
        FMOD_Channel_Stop(m_pChannelArr[eID]);

    FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, TRUE, &m_pChannelArr[eID]);

    if (!m_pChannelArr[eID])
        return;

    FMOD_Channel_SetMode(m_pChannelArr[eID], FMOD_LOOP_NORMAL);
    FMOD_Channel_SetVolume(m_pChannelArr[eID], fVolume);
    FMOD_Channel_SetPaused(m_pChannelArr[eID], FALSE);

    FMOD_System_Update(m_pSystem);
}

void CSound_System::PlayBGM(const wstring& wsSoundKey, float fVolume)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    if (m_pChannelArr[CHANNEL_0])
    {
        FMOD_Channel_Stop(m_pChannelArr[CHANNEL_0]);
        m_pChannelArr[CHANNEL_0] = nullptr;
    }

    FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, TRUE, &m_pChannelArr[CHANNEL_0]);

    FMOD_Channel_SetMode(m_pChannelArr[CHANNEL_0], FMOD_LOOP_NORMAL);
    FMOD_Channel_SetVolume(m_pChannelArr[CHANNEL_0], fVolume);
    FMOD_Channel_SetPaused(m_pChannelArr[CHANNEL_0], FALSE);

    FMOD_System_Update(m_pSystem);
}

void CSound_System::StopSound(CHANNELID eID)
{
    if (!m_pChannelArr[eID])
        return;

    FMOD_Channel_Stop(m_pChannelArr[eID]);
    m_pChannelArr[eID] = nullptr;
}
void CSound_System::StopAll()
{
    for (int i = 0; i < MAXCHANNEL; ++i)
        FMOD_Channel_Stop(m_pChannelArr[i]);
}

void CSound_System::SetChannelVolume(CHANNELID eID, float fVolume)
{
    FMOD_Channel_SetVolume(m_pChannelArr[eID], fVolume);

    FMOD_System_Update(m_pSystem);
}

void CSound_System::LoadSoundFile(const wstring& wsPath)
{
    std::wstring wsBasePath = wsPath;
    if (!wsBasePath.empty())
    {
        wchar_t back = wsBasePath.back();
        if (back != L'/' && back != L'\\')
            wsBasePath += L'/';
    }

    // 검색 패턴 (*.mp3 / *.wav / *.ogg)
    std::wstring wsSearchPath = wsBasePath + L"*.wav";

    _wfinddata_t fd;
    intptr_t hFind = _wfindfirst(wsSearchPath.c_str(), &fd);
    if (hFind == -1)
    {
        MSG_BOX("There is no Sound File");
        return;
    }

    do
    {
        // 전체 경로(wchar_t)
        std::wstring wsFullPath = wsBasePath + fd.name;

        // FMOD 는 char* 경로를 받으므로 멀티바이트로 변환
        char szFullPath[MAX_PATH] = {};
        WideCharToMultiByte(CP_ACP, 0,
            wsFullPath.c_str(), -1,
            szFullPath, MAX_PATH,
            nullptr, nullptr);

        FMOD_SOUND* pSound = nullptr;
        FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem,
            szFullPath,
            FMOD_DEFAULT,
            0,
            &pSound);

        if (eRes == FMOD_OK)
        {
            // player_attack.wav < 파일 지정자 포함
            std::wstring wsKey = fd.name;
            m_umapSound.emplace(wsKey, pSound);
        }

    } while (_wfindnext(hFind, &fd) != -1);

    _findclose(hFind);
    FMOD_System_Update(m_pSystem);
}

void CSound_System::PlaySoundLoopSection(
    const wstring& wsSoundKey,
    CHANNELID eID,
    float fVolume,
    unsigned int loopStartMs,
    unsigned int loopEndMs,
    bool bPlayIntro)
{
    auto iter = m_umapSound.find(wsSoundKey);
    if (iter == m_umapSound.end())
        return;

    FMOD_SOUND* pSound = iter->second;

    // 길이 클램프 (ms)
    unsigned int lenMs = 0;
    FMOD_Sound_GetLength(pSound, &lenMs, FMOD_TIMEUNIT_MS);

    if (lenMs == 0) return;
    if (loopStartMs > lenMs) loopStartMs = lenMs;
    if (loopEndMs > lenMs) loopEndMs = lenMs;
    if (loopEndMs <= loopStartMs + 1) return;

    // 기존 재생 중이면 정지
    if (m_pChannelArr[eID])
        FMOD_Channel_Stop(m_pChannelArr[eID]);

    // 일단 PAUSED로 틀어놓고 세팅 후 풀기 (첫 프레임 튐 방지)
    FMOD_System_PlaySound(m_pSystem, pSound, nullptr, TRUE, &m_pChannelArr[eID]);

    FMOD_CHANNEL* ch = m_pChannelArr[eID];
    if (!ch) return;

    // “구간 루프”의 핵심
    FMOD_Channel_SetMode(ch, FMOD_LOOP_NORMAL);
    FMOD_Channel_SetLoopPoints(ch,
        loopStartMs, FMOD_TIMEUNIT_MS,
        loopEndMs, FMOD_TIMEUNIT_MS);

    // 인트로 없이 구간만 돌리고 싶으면 시작 위치를 loopStart로 점프
    if (!bPlayIntro)
        FMOD_Channel_SetPosition(ch, loopStartMs, FMOD_TIMEUNIT_MS);

    FMOD_Channel_SetVolume(ch, fVolume);
    FMOD_Channel_SetPaused(ch, FALSE);

    FMOD_System_Update(m_pSystem);
}

void CSound_System::LoadSoundFiles(const wstring& wsPath)
{
    std::wstring wsBasePath = wsPath;
    if (!wsBasePath.empty())
    {
        wchar_t back = wsBasePath.back();
        if (back != L'/' && back != L'\\')
            wsBasePath += L'/';
    }

    std::wstring wsSearchPath = wsBasePath + L"*.*";

    _wfinddata_t fd;
    intptr_t hFind = _wfindfirst(wsSearchPath.c_str(), &fd);
    if (hFind == -1)
    {
        MSG_BOX("There is no Sound File");
        return;
    }

    do
    {
        if (fd.attrib & _A_SUBDIR)
            continue;

        std::wstring wsFileName = fd.name;

        size_t iDotPos = wsFileName.find_last_of(L'.');
        if (iDotPos == std::wstring::npos)
            continue;

        std::wstring wsExt = wsFileName.substr(iDotPos);

        for (auto& ch : wsExt)
            ch = towlower(ch);

        if (wsExt != L".wav" &&
            wsExt != L".mp3" &&
            wsExt != L".ogg")
            continue;

        std::wstring wsFullPath = wsBasePath + wsFileName;

        char szFullPath[MAX_PATH] = {};
        WideCharToMultiByte(CP_ACP, 0,
            wsFullPath.c_str(), -1,
            szFullPath, MAX_PATH,
            nullptr, nullptr);

        FMOD_SOUND* pSound = nullptr;
        FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem,
            szFullPath,
            FMOD_DEFAULT,
            0,
            &pSound);

        if (eRes == FMOD_OK)
        {
            std::wstring wsKey = wsFileName.substr(0, iDotPos);

            auto iter = m_umapSound.find(wsKey);
            if (iter != m_umapSound.end())
            {
                FMOD_Sound_Release(iter->second);
                iter->second = pSound;
            }
            else
            {
                m_umapSound.emplace(wsKey, pSound);
            }
        }

    } while (_wfindnext(hFind, &fd) != -1);

    _findclose(hFind);
    FMOD_System_Update(m_pSystem);
}

void CSound_System::LoadMp3SoundFile(const wstring& wsPath)
{
    std::wstring wsBasePath = wsPath;
    if (!wsBasePath.empty())
    {
        wchar_t back = wsBasePath.back();
        if (back != L'/' && back != L'\\')
            wsBasePath += L'/';
    }

    std::wstring wsSearchPath = wsBasePath + L"*.mp3";

    _wfinddata_t fd;
    intptr_t hFind = _wfindfirst(wsSearchPath.c_str(), &fd);
    if (hFind == -1)
    {
        MSG_BOX("There is no Sound File");
        return;
    }

    do
    {
        // 전체 경로(wchar_t)
        std::wstring wsFullPath = wsBasePath + fd.name;

        // FMOD 는 char* 경로를 받으므로 멀티바이트로 변환
        char szFullPath[MAX_PATH] = {};
        WideCharToMultiByte(CP_ACP, 0,
            wsFullPath.c_str(), -1,
            szFullPath, MAX_PATH,
            nullptr, nullptr);

        FMOD_SOUND* pSound = nullptr;
        FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem,
            szFullPath,
            FMOD_DEFAULT,
            0,
            &pSound);

        if (eRes == FMOD_OK)
        {
            // player_attack.wav < 파일 지정자 포함
            std::wstring wsKey = fd.name;
            m_umapSound.emplace(wsKey, pSound);
        }

    } while (_wfindnext(hFind, &fd) != -1);

    _findclose(hFind);
    FMOD_System_Update(m_pSystem);
}

void CSound_System::LoadOggSoundFile(const wstring& wsPath)
{
    std::wstring wsBasePath = wsPath;
    if (!wsBasePath.empty())
    {
        wchar_t back = wsBasePath.back();
        if (back != L'/' && back != L'\\')
            wsBasePath += L'/';
    }

    std::wstring wsSearchPath = wsBasePath + L"*.ogg";

    _wfinddata_t fd;
    intptr_t hFind = _wfindfirst(wsSearchPath.c_str(), &fd);
    if (hFind == -1)
    {
        MSG_BOX("There is no Sound File");
        return;
    }

    do
    {
        std::wstring wsFullPath = wsBasePath + fd.name;

        char szFullPath[MAX_PATH] = {};
        WideCharToMultiByte(CP_ACP, 0,
            wsFullPath.c_str(), -1,
            szFullPath, MAX_PATH,
            nullptr, nullptr);

        FMOD_SOUND* pSound = nullptr;
        FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem,
            szFullPath,
            FMOD_DEFAULT,
            0,
            &pSound);

        if (eRes == FMOD_OK)
        {
            std::wstring wsKey = fd.name;
            m_umapSound.emplace(wsKey, pSound);
        }

    } while (_wfindnext(hFind, &fd) != -1);

    _findclose(hFind);
    FMOD_System_Update(m_pSystem);
}
