#pragma once
#include  "Editor_Define.h"

NS_BEGIN(Engine)
struct tagTransformData;
struct tagRectTransformData;
NS_END

NS_BEGIN(Editor)

class CGizmo final
{
public :
    CGizmo();
    ~CGizmo();

public:
    void Set_Mode(GIZMO_MODE eMode);
    GIZMO_MODE Get_Mode() const;
    void Set_Space(GIZMO_SPACE eSpace);
    GIZMO_SPACE Get_Space() const;

    void Render(
        const float* view,
        const float* proj,
        float* world,
        const ImVec2& viewportPos,
        const ImVec2& viewportSize
    );
    static void Apply_World_To_TransformData(const float* world16, Engine::tagTransformData& td);

    //void Render_UI(
    //    const float* view,
    //    const float* proj,
    //    float* world,
    //    const ImVec2& viewportPos,
    //    const ImVec2& viewportSize
    //);
    //static void Apply_World_To_RectTransformData(const float* world16, tagRectTransformData& td, const ImVec2& viewportSize);

private:
    GIZMO_MODE  m_Mode = GIZMO_MODE::TRANSLATE;
    GIZMO_SPACE m_Space = GIZMO_SPACE::LOCAL;

public :
    static std::unique_ptr<CGizmo> Create();
};

NS_END
