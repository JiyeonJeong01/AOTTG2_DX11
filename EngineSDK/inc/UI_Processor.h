// UI_Processor.h
#pragma once
#include "Component_Pool.h"
#include "Component_Processor.h"
#include "UIButton.h"
#include "UIImage.h"

NS_BEGIN(Engine)

class CCanvasRenderer_Processor;
class CRectTransform_Processor;
class CUIButton;
class CUIImage;
class CUIText;

class ENGINE_DLL CUI_Processor final : public CComponent_Processor
{
public :
    CUI_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CCanvasRenderer_Processor* pCanvasProcessor, CRectTransform_Processor* pRTProcessor);
    ~CUI_Processor() override;

public:
    HRESULT Initialize() override;
    void    Update(_float fDT) override;
    void    LateUpdate(_float fDT) override;
    void    Render();

    COMPONENT_HANDLE Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) override;
    void Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

    template<typename TProxy>
    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
    {
        if constexpr (std::is_same_v<TProxy, CUIButton>) {
            return m_ButtonPool.Get_Proxy(hComponent);
        }
        else if constexpr (std::is_same_v<TProxy, CUIImage>) {
            return m_ImagePool.Get_Proxy(hComponent);
        }

        IF_TRUE_RETURN_MSG_BREAK(true, TProxy{}, "Invalid Proxy Type for this Pool");
    }

private:
    template <typename TProxy>
    COMPONENT_HANDLE Create_Component_Data_Inner(CComponent_Pool<TProxy>& pool, OBJECT_HANDLE hObject)
    {
        COMPONENT_HANDLE hComponent = pool.Allocate();
        auto pData = pool.Get_Data_By_Handle(hComponent);
        pData->hObject = hObject;
        return hComponent;
    }

    template <typename TProxy>
    void Remove_Component_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent)
    {
        pool.Deallocate(hComponent);
    }
    void Sync_Images_To_Canvas();
    void Update_Buttons(_float fDT);
    void Apply_ButtonVisual(const UI_BUTTON_DATA& tData);

    static _bool HitTest_Rect(const RECT& rcScreen, const POINT& ptMouse) noexcept;

    HRESULT Initialize_From_Spec_UIButton(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);
    HRESULT Initialize_From_Spec_UIImage(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);
    //HRESULT Initialize_From_Spec_UIText(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec);

private :
    CCanvasRenderer_Processor*  m_pCanvasProcessor{};
    CRectTransform_Processor*   m_pRectTransformProcessor{};
    CComponent_Pool<CUIButton>  m_ButtonPool;
    CComponent_Pool<CUIImage>   m_ImagePool;

public :
    static std::unique_ptr<CUI_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CCanvasRenderer_Processor* pCanvasProcessor, CRectTransform_Processor* pRTProcessor);
};

NS_END
