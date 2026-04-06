#include "Hello.h"
#include "GameObject.h"
#include "GameObject_System.h"
#include "Transform.h"
#include "Input_System.h"
#include "Rigidbody.h"
#include "Logger.h"
#include "Resource_System.h"

NS_BEGIN(Client)

void CHello::Awake(void* pCtx)
{
}

void CHello::Start(void* pCtx)
{
    Engine::CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    if (!pObject)
    {
        __debugbreak();
        return;
    }

    m_Transform = pObject->Get_Component<CTransform>();
    m_mr = pObject->Get_Component<CMeshRenderer>();

    m_hPerObjBlock = SYS_RESOURCE.Alloc_PerObjectParamBlock();

    auto* pBlock = SYS_RESOURCE.Get_PerObjectParamBlock(m_hPerObjBlock);
    if (!pBlock)
    {
        __debugbreak();
        return;
    }

    m_hTexture = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_MaskTExture);
    if (m_hTexture == INVALID_HANDLE_UINT)
    {
        __debugbreak();
        return;
    }

    pBlock->block.Set_Texture("g_CuttedMask", m_hTexture);
    pBlock->block.Set_Float4("g_CutFlag", { 0.f, 0.f, 0.f, 0.f });

    cout << "CHello::Start Set_Texture = [" << "g_CuttedMask" << "]" << endl;
    cout << "CHello::Start Set_Float4 = [" << "g_CutFlag" << "]" << endl;

    /* 이 부분은 MeshRenderer 쪽에 per-object param handle을 연결하는 용도 */
    m_mr->hPerObjectParams = m_hPerObjBlock;
}

void CHello::Priority_Update(void* pCtx, _float fDT)
{
}

void CHello::Update(void* pCtx, _float fDT)
{
    auto* flag = SYS_RESOURCE.Get_PerObjectParamBlock(m_hPerObjBlock);
    if (!flag)
        return;

    if (SYS_INPUT.Get_Key(VK_UP))

    {
        flag->block.Set_Float4("g_CutFlag", { 1.f, 0.f, 0.f, 0.f });
    }

    if (SYS_INPUT.Get_Key(VK_DOWN))
    {
        flag->block.Set_Float4("g_CutFlag", { 0.f, 1.f, 0.f, 0.f });
    }

    if (SYS_INPUT.Get_Key(VK_RIGHT))
    {
        flag->block.Set_Float4("g_CutFlag", { 0.f, 0.f, 1.f, 0.f });
    }

    if (SYS_INPUT.Get_Key(VK_LEFT))
    {
        flag->block.Set_Float4("g_CutFlag", { 0.f, 0.f, 0.f, 1.f });
    }

    if (SYS_INPUT.Get_KeyDown(VK_LBUTTON))
    {
        flag->block.Set_Float4("g_CutFlag", { 1.f, 1.f, 0.f, 0.f });
    }

    if (SYS_INPUT.Get_KeyDown(VK_RBUTTON))
    {
        flag->block.Set_Float4("g_CutFlag", { 0.f, 0.f, 0.f, 0.f });
    }
}

void CHello::Late_Update(void* pCtx, _float fDT)
{
}

void CHello::Move(_float fDT)
{
}

NS_END;
