#include "Tester.h"
#include "Engine_Log.h"
#include "MainApp.h"
#include "Event.h"
#include "Camera.h"
#include "Component_System.h"
#include "Environment_Processor.h"
#include "GameObject.h"
#include "GameObject_System.h"
#include "Transform.h"
#include "Transform_Processor.h"
#include <algorithm>

NS_BEGIN(Client)

void Tester::Initialize_Tester(CMainApp* pMainapp)
{
}

void Tester::Run_ComponentPoolPageTest()
{
    LOG_INFO("[PoolPageTest] begin");

    struct HandleStats
    {
        uint32_t iMinIndex = 0;
        uint32_t iMaxIndex = 0;
        uint32_t iMaxPage = 0;
        uint32_t iUniquePageCount = 0;
        bool bHasDuplicateIndex = false;
    };

    auto BuildStats = [](const std::vector<COMPONENT_HANDLE>& handles, uint32_t iPageSize)
        {
            HandleStats stats{};
            if (handles.empty())
                return stats;

            stats.iMinIndex = handles.front().Get_Index();
            stats.iMaxIndex = stats.iMinIndex;

            std::vector<uint32_t> indices;
            indices.reserve(handles.size());

            for (COMPONENT_HANDLE hComponent : handles)
            {
                const uint32_t iIndex = hComponent.Get_Index();
                if (iIndex < stats.iMinIndex)
                    stats.iMinIndex = iIndex;
                if (iIndex > stats.iMaxIndex)
                    stats.iMaxIndex = iIndex;
                indices.push_back(iIndex);
            }

            stats.iMaxPage = stats.iMaxIndex / iPageSize;

            std::sort(indices.begin(), indices.end());
            stats.bHasDuplicateIndex = std::adjacent_find(indices.begin(), indices.end()) != indices.end();

            uint32_t iPrevPage = UINT32_MAX;
            for (uint32_t iIndex : indices)
            {
                const uint32_t iPage = iIndex / iPageSize;
                if (iPage != iPrevPage)
                {
                    ++stats.iUniquePageCount;
                    iPrevPage = iPage;
                }
            }

            return stats;
        };

    auto RunCameraCase = [&](uint32_t iCount)
        {
            std::vector<CGameObject*> objects;
            std::vector<COMPONENT_HANDLE> cameraHandles;
            objects.reserve(iCount);
            cameraHandles.reserve(iCount);

            bool bOk = true;
            for (uint32_t i = 0; i < iCount; ++i)
            {
                CGameObject* pObj = SYS_GAMEOBJECT.Create_GameObject(Layer::DEFAULT_LAYER, "PoolPageTest_Camera");
                if (!pObj)
                {
                    bOk = false;
                    break;
                }

                CCamera camera = pObj->Add_Component<CCamera>();
                if (!camera.Is_Valid())
                {
                    bOk = false;
                    SYS_GAMEOBJECT.Destroy_Object(pObj);
                    break;
                }

                objects.push_back(pObj);
                cameraHandles.push_back(camera.Get_Handle());
            }

            constexpr uint32_t iCameraPageSize = 64;
            HandleStats stats = BuildStats(cameraHandles, iCameraPageSize);
            const bool bCreatedAll = cameraHandles.size() == iCount;
            const bool bCrossedExpectedPage = (iCount <= iCameraPageSize) || (stats.iMaxPage >= 1);
            bOk = bOk && bCreatedAll && !stats.bHasDuplicateIndex && bCrossedExpectedPage;

            LOG_INFO(
                "[PoolPageTest][Camera64] requested=%u created=%zu ok=%d minIndex=%u maxIndex=%u maxPage=%u uniquePages=%u duplicateIndex=%d",
                iCount,
                objects.size(),
                bOk ? 1 : 0,
                stats.iMinIndex,
                stats.iMaxIndex,
                stats.iMaxPage,
                stats.iUniquePageCount,
                stats.bHasDuplicateIndex ? 1 : 0);

            for (CGameObject* pObj : objects)
            {
                if (pObj && pObj->Is_Valid())
                    SYS_GAMEOBJECT.Destroy_Object(pObj);
            }
            SYS_GAMEOBJECT.Flush_PendingDestroy();

            CEnvironment_Processor* pEnvironmentProcessor = SYS_COMPONENT.Bind_Processor<CEnvironment_Processor>();
            bool bDestroyedHandlesRejected = pEnvironmentProcessor != nullptr;
            for (COMPONENT_HANDLE hCamera : cameraHandles)
            {
                if (pEnvironmentProcessor == nullptr ||
                    pEnvironmentProcessor->Get_DataPtr(COMPONENT_TYPE::CAMERA, hCamera) != nullptr)
                {
                    bDestroyedHandlesRejected = false;
                    break;
                }
            }

            LOG_INFO(
                "[PoolPageTest][Camera64Cleanup] requested=%u destroyedRejected=%d",
                iCount,
                bDestroyedHandlesRejected ? 1 : 0);
        };

    RunCameraCase(1);
    RunCameraCase(4);
    RunCameraCase(64);
    RunCameraCase(65);

    std::vector<COMPONENT_HANDLE> transformHandles;
    transformHandles.reserve(1025);

    bool bTransformOk = true;
    for (uint32_t i = 0; i < 1025; ++i)
    {
        COMPONENT_HANDLE hTransform = SYS_COMPONENT.Create_Component_By_Type(COMPONENT_TYPE::TRANSFORM, OBJECT_HANDLE{});
        if (!hTransform.Is_Valid())
        {
            bTransformOk = false;
            break;
        }

        transformHandles.push_back(hTransform);
    }

    constexpr uint32_t iTransformPageSize = 1024;
    HandleStats transformStats = BuildStats(transformHandles, iTransformPageSize);
    bTransformOk = bTransformOk &&
        transformHandles.size() == 1025 &&
        !transformStats.bHasDuplicateIndex &&
        transformStats.iMaxPage >= 1;

    LOG_INFO(
        "[PoolPageTest][Transform1024] requested=1025 created=%zu ok=%d minIndex=%u maxIndex=%u maxPage=%u uniquePages=%u duplicateIndex=%d",
        transformHandles.size(),
        bTransformOk ? 1 : 0,
        transformStats.iMinIndex,
        transformStats.iMaxIndex,
        transformStats.iMaxPage,
        transformStats.iUniquePageCount,
        transformStats.bHasDuplicateIndex ? 1 : 0);

    COMPONENT_HANDLE hOldTransform{};
    COMPONENT_HANDLE hNewTransform{};
    bool bOldHandleRejected = false;

    if (!transformHandles.empty())
    {
        hOldTransform = transformHandles.back();
        SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hOldTransform);

        CTransform_Processor* pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
        bOldHandleRejected =
            (pTransformProcessor != nullptr) &&
            (pTransformProcessor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, hOldTransform) == nullptr);

        hNewTransform = SYS_COMPONENT.Create_Component_By_Type(COMPONENT_TYPE::TRANSFORM, OBJECT_HANDLE{});

        if (hNewTransform.Is_Valid())
            SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hNewTransform);

        transformHandles.pop_back();
    }

    for (COMPONENT_HANDLE hTransform : transformHandles)
        SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hTransform);

    CTransform_Processor* pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    bool bDestroyedTransformRejected = pTransformProcessor != nullptr;
    for (COMPONENT_HANDLE hTransform : transformHandles)
    {
        if (pTransformProcessor == nullptr ||
            pTransformProcessor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, hTransform) != nullptr)
        {
            bDestroyedTransformRejected = false;
            break;
        }
    }

    LOG_INFO(
        "[PoolPageTest][TransformReuse] oldHandle=%u newHandle=%u oldRejected=%d sameIndex=%d versionChanged=%d",
        hOldTransform.iHandle,
        hNewTransform.iHandle,
        bOldHandleRejected ? 1 : 0,
        hOldTransform.Get_Index() == hNewTransform.Get_Index() ? 1 : 0,
        hOldTransform.Get_Version() != hNewTransform.Get_Version() ? 1 : 0);

    LOG_INFO(
        "[PoolPageTest][TransformCleanup] destroyedRejected=%d",
        bDestroyedTransformRejected ? 1 : 0);

    LOG_INFO("[PoolPageTest] end");
}

void Tester::Test_Public_Func_void()
{
}

void Tester::Test_Public_Func_int(_int iTest)
{
}

void Tester::Test_Public_Func_float_int(_int iTest, _float fTest)
{
}

void Tester::Test_Private_Func_void()
{
}

void Tester::Test_Private_Func_int(_int iTest)
{
}

void Client::Tester::Test_Private_Func_int_float(_int iTest, _float fTest)
{
}

Tester* Client::Tester::Create()
{
    return new Tester;
}

void Tester::Free()
{
    CBase::Free();
}

NS_END
