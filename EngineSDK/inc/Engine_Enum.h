#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	enum class WINMODE { FULL, WIN };

	enum class LAYER_TYPE		{ NONE, BACKGROUND, PLAYER, FLOOR, PARTICLE, MAX = 16, };

    /* ------ Component ------ */
    enum class COMPONENT_TYPE : uint8_t {   TRANSFORM, COLLIDER, RIGIDBODY, SPRING_JOINT, SCRIPT, MESH_RENDERER, ANIMATOR, CAMERA, LIGHT, AUDIO_LISTENER, AUDIO_SOURCE,
                                            RECT_TRANSFORM, CANVAS_RENDERER, UI_IMAGE, UI_BUTTON, UI_TEXT, END };
    enum class PROCESSOR_ID : uint8_t
    {
        TRANSFORM,             // CTransform
        PHYSICS,               // Collider / Rigidbody / SpringJoint
        SCRIPT,                // Script
        MESH_RENDERER,         // MeshRenderer
        ANIMATION,             // Animator
        ENVIRONMENT,           // Camera / Light
        AUDIO,                 // AudioListener / AudioSource
        RECT_TRANSFORM,        // CRectTransform
        CANVAS_RENDERER,       // CanvasRenderer
        UI,                    // UIImage / UIButton / UIText
        END
    };

    enum                                    { COMPONENT_MAX = static_cast<uint32_t>(COMPONENT_TYPE::END) };
    enum                                    { COMPONENT_PROCESSOR_MAX = static_cast<uint32_t>(PROCESSOR_ID::END) };

    /* --- Render --- */
    enum class DRAW_TYPE    : uint8_t       { MESH = 0, CANVAS, LINE, TEXT };
    enum class RENDER_LAYER : uint8_t       { PRIORITY = 0, NONBLEND, BLEND, UI, END };
    enum RENDER_FLAGS : uint32_t            { RF_NONE = 0, RF_CAST_SHADOW = 1 << 0, RF_RECEIVE_SHADOW = 1 << 1, RF_DISABLE_CULL = 1 << 2, };
    enum CANVAS_FLAGS : uint32_t            { CF_NONE = 0, CF_CLIP_RECT = 1u << 0, CF_PIXEL_SNAP = 1u << 1, };
    enum class VERTEX_DECL : uint8_t        { VTXCOL = 0, VTXTEX, VTXNORTEX, VTXMESH, VTXANIMMESH, END };
    enum class PARAM_TYPE : uint8_t         { FLOAT, FLOAT4, FLOAT4X4, TEXTURE_HANDLE };
    enum class MODEL_TYPE : uint8_t         { NONANIM = 0, ANIM, };
    enum class MESH_MODE : uint8_t          { NONE = 0, PARTS, ATTACH };


    /* --- Physics --- */
	enum class SHAPE : uint8_t              { BOX, SPHERE, PLANE, CAPSULE, END };
	enum class BODY_TYPE : uint8_t          { DYNAMIC, KINEMATIC, END };

    /* --- UI --- */
	enum class UI_TYPE : uint8_t            { BUTTON, IMAGE, PANEL, SLOT, END };
    enum class UI_BTN_STATE : uint8_t       { Normal, Hover, Pressed, Disabled };

    /* --- Asset --- */
    enum class ASSET_TYPE : uint8_t         { UNKNOWN = 0, FOLDER, TEXTURE, MESH, SHADER, MODEL, MATERIAL, SCENE, PROTOTYPE, SCRIPT, FONT };
    enum class ASSET_SRC : uint8_t          { FILE, BUILTIN };

    /* --- Scene --- */
    enum class APP_MODE : uint8_t           { GAME_PLAY = 0, EDITOR_EDIT };
    enum class SCENE_STATE : uint8_t        { PLAY = 0, PAUSE, EDIT };

    /* --- Environment --- */
    enum class LIGHT_TYPE : uint8_t         { DIRECTIONAL = 0, POINT, SPOT };

    /* ------ LOG ------ */
	enum class SEVERITY_TYPE : uint8_t      { INFO, WARN, ERR, ASSERTION, END };
	enum class DOMAIN_TYPE : uint8_t        { ENGINE, CLIENT, EDITOR, END };

    /* ------ INPUT ------ */
    enum class MOUSE_MOVE_AXIS : uint8_t    { HORIZONTAL, VERTICAL, DEPTH, END };
    enum class MOUSE_BUTTON : uint8_t       { LEFT, RIGHT, MIDDLE, END };

    /* --- Utils --- */
    enum class SCRIPT_FIELD_TYPE : uint8_t  { INT = 0, FLOAT, FLOAT2, FLOAT3, FLOAT4, OBJECT_REF };
    enum class STATE : uint8_t { RIGHT, UP, LOOK, POSITION, END };
    enum class SPACE : uint8_t { WORLD, LOCAL, END };
    enum class Geometry : uint8_t { Rect, Circle, Cube, Sphere };
    enum class EVENT_TYPE : uint8_t
    {
        None, GameObject, On_GameObject_Created, On_GameObject_Destroyed, On_Scene_Changed, On_Window_Resize,
        BUTTON
    };
}
#endif // Engine_Enum_h__
