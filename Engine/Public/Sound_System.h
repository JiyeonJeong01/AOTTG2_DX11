#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CSound_System final
{
    DECLARE_SINGLETON(CSound_System)
public:
    HRESULT     Initialize();

    void        PlaySFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume);
    void        PlayForceSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume);
    void        PlayLoopSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume);
    void        PlayForceLoopSFX(const wstring& wsSoundKey, CHANNELID eID, float fVolume);

    void        PlayBGM(const wstring& wsSoundKey, float fVolume);
    void        StopSound(CHANNELID eID);
    void        StopAll();
    void        SetChannelVolume(CHANNELID eID, float fVolume);
    void        LoadSoundFile(const wstring& wsPath);
    void        PlaySoundLoopSection(const wstring& wsSoundKey,
        CHANNELID eID,
        float fVolume,
        unsigned int loopStartMs,
        unsigned int loopEndMs,
        bool bPlayIntro);
public:
    void        LoadSoundFiles(const wstring& wsPath);
    void        LoadMp3SoundFile(const wstring& wsPath);
    void        LoadOggSoundFile(const wstring& wsPath);

private:
    unordered_map<wstring, FMOD_SOUND*> m_umapSound;
    FMOD_CHANNEL* m_pChannelArr[Engine::MAXCHANNEL];
    FMOD_SYSTEM* m_pSystem;

};

NS_END
