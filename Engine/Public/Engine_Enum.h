#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	enum class WINMODE { FULL, WIN };

	enum class LAYER_TYPE		{ NONE, BACKGROUND, PLAYER, FLOOR, PARTICLE, MAX = 16, };
    enum class COMPONENT_TYPE : uint8_t { TEST_A, TEST_B, TEXTURE, 
                                        TRANSFORM, COLLIDER, RIGIDBODY, SCRIPT, MESH_RENDERER, ANIMATOR, CAMERA, AUDIO_LISTENER, AUDIO_SOURCE, SHADER, END };
	enum                                { COMPONENT_MAX = static_cast<uint32_t>(COMPONENT_TYPE::END) };
    enum class RENDER_LAYER : uint8_t   { PRIORITY = 0, NONBLEND, BLEND, UI, END };
    enum RENDER_FLAGS : uint32_t        { RF_NONE = 0, RF_CAST_SHADOW = 1 << 0, RF_RECEIVE_SHADOW = 1 << 1, RF_DISABLE_CULL = 1 << 2, };
    enum class VERTEX_DECL : uint8_t    { VTXCOL = 0, VTXTEX, };


	enum class COLLIDER_TYPE : uint8_t  { SPHERE, BOX, CAPSULE, END };
	enum class BODY_TYPE : uint8_t      { STATIC, KINEMATIC, DYNAMIC, END };
	enum class UI_TYPE : uint8_t        { BUTTON, IMAGE, PANEL, SLOT, END };

	enum class STATE : uint8_t          { RIGHT, UP, LOOK, POSITION, END };
    enum class SPACE : uint8_t          { WORLD, LOCAL, END };

    enum class EVENT_TYPE : uint8_t     { None, On_GameObject_Created, On_GameObject_Destroyed, On_Scene_Changed, On_Window_Resize };

    enum class ASSET_TYPE : uint8_t     { UNKNOWN = 0, FOLDER, TEXTURE, MESH, SHADER, MODEL, MATERIAL, SCENE, PREFAB, PROTOTYPE, SCRIPT, };
    enum class ASSET_SRC : uint8_t      { FILE, BUILTIN };

    enum class Geometry : uint8_t       { Rect, Circle, Cube, Sphere };

    /* ------ LOG ------ */
	enum class SEVERITY_TYPE : uint8_t  { INFO, WARN, ERR, ASSERTION, END };
	enum class DOMAIN_TYPE : uint8_t    { ENGINE, CLIENT, EDITOR, END };

    /* ------ INPUT ------ */
    enum class MOUSE_MOVE_AXIS : uint8_t{ HORIZONTAL, VERTICAL, DEPTH, END };
    enum class MOUSE_BUTTON : uint8_t   { LEFT, RIGHT, MIDDLE, END };
}
#endif // Engine_Enum_h__
