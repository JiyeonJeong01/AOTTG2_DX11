#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class CScene;

typedef struct ENGINE_DLL tagSceneChangeData : public EVENT_DATA
{
public:
    tagSceneChangeData(EVENT_TYPE eType, CScene* pNewScene, CScene* pOldScene = nullptr)
        : EVENT_DATA(EVENT_TYPE::On_Scene_Changed), m_pNewScene(pNewScene), m_pOldScene(pOldScene) {
    };
    ~tagSceneChangeData() override = default;

    CScene* m_pNewScene{};
    CScene* m_pOldScene{};
}SCENECHANGE_EVENT_DATA;

NS_END
