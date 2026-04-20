#include "CinematicIO.h"

bool CCinematicIO::Save(const std::string& strFileName, const CINEMATIC_CLIP& tClip)
{
    std::string strPath = "../../Client/Bin/Assets/DataFiles/Cinematics/" + strFileName + ".dat";

    std::ofstream ofs(strPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    CINEMATIC_FILE_HEADER tHeader{};
    tHeader.iCameraKeyCount = To<uint32_t>(tClip.vecCameraKeys.size());
    tHeader.iEventKeyCount = To<uint32_t>(tClip.vecEventKeys.size());
    tHeader.iShakeKeyCount = To<uint32_t>(tClip.vecShakeKeys.size());
    tHeader.iShotKeyCount = To<uint32_t>(tClip.vecShotKeys.size());

    ofs.write(reinterpret_cast<const char*>(&tHeader), sizeof(tHeader));

    ofs.write(reinterpret_cast<const char*>(&tClip.fDuration), sizeof(_float));
    ofs.write(reinterpret_cast<const char*>(tClip.szName), sizeof(tClip.szName));

    if (!tClip.vecCameraKeys.empty())
    {
        ofs.write(reinterpret_cast<const char*>(tClip.vecCameraKeys.data()),
            sizeof(CINEMATIC_CAMERA_KEY) * tClip.vecCameraKeys.size());
    }

    if (!tClip.vecEventKeys.empty())
    {
        ofs.write(reinterpret_cast<const char*>(tClip.vecEventKeys.data()),
            sizeof(CINEMATIC_EVENT_KEY) * tClip.vecEventKeys.size());
    }

    if (!tClip.vecShakeKeys.empty())
    {
        ofs.write(reinterpret_cast<const char*>(tClip.vecShakeKeys.data()),
            sizeof(CINEMATIC_SHAKE_KEY) * tClip.vecShakeKeys.size());
    }

    if (!tClip.vecShotKeys.empty())
    {
        ofs.write(reinterpret_cast<const char*>(tClip.vecShotKeys.data()),
            sizeof(CINEMATIC_SHOT_KEY) * tClip.vecShotKeys.size());
    }

    ofs.close();
    return true;
}

bool CCinematicIO::Load(const std::string& strFileName, CINEMATIC_CLIP& tOutClip)
{
    std::string strPath =
        "../../Client/Bin/Assets/DataFiles/Cinematics/" + strFileName + ".dat";

    std::ifstream ifs(strPath, std::ios_base::binary);
    if (!ifs.is_open())
        return false;

    CINEMATIC_FILE_HEADER tHeader{};
    ifs.read(reinterpret_cast<char*>(&tHeader), sizeof(tHeader));

    if (tHeader.iMagic != 'CMTK')
    {
        ifs.close();
        return false;
    }

    tOutClip = {};

    ifs.read(reinterpret_cast<char*>(&tOutClip.fDuration), sizeof(_float));
    ifs.read(reinterpret_cast<char*>(tOutClip.szName), sizeof(tOutClip.szName));

    tOutClip.vecCameraKeys.resize(tHeader.iCameraKeyCount);
    tOutClip.vecEventKeys.resize(tHeader.iEventKeyCount);
    tOutClip.vecShakeKeys.resize(tHeader.iShakeKeyCount);
    tOutClip.vecShotKeys.resize(tHeader.iShotKeyCount);

    if (!tOutClip.vecCameraKeys.empty())
    {
        ifs.read(reinterpret_cast<char*>(tOutClip.vecCameraKeys.data()),
            sizeof(CINEMATIC_CAMERA_KEY) * tOutClip.vecCameraKeys.size());
    }

    if (!tOutClip.vecEventKeys.empty())
    {
        ifs.read(reinterpret_cast<char*>(tOutClip.vecEventKeys.data()),
            sizeof(CINEMATIC_EVENT_KEY) * tOutClip.vecEventKeys.size());
    }

    if (!tOutClip.vecShakeKeys.empty())
    {
        ifs.read(reinterpret_cast<char*>(tOutClip.vecShakeKeys.data()),
            sizeof(CINEMATIC_SHAKE_KEY) * tOutClip.vecShakeKeys.size());
    }

    if (!tOutClip.vecShotKeys.empty())
    {
        ifs.read(reinterpret_cast<char*>(tOutClip.vecShotKeys.data()),
            sizeof(CINEMATIC_SHOT_KEY) * tOutClip.vecShotKeys.size());
    }

    ifs.close();
    return true;
}
