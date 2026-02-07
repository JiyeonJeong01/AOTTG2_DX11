#ifndef Engine_Enum_h__
#define Engine_Enum_h__



namespace Engine
{
	enum class WINMODE { FULL, WIN };

	enum class LAYER_TYPE		{ NONE, BACKGROUND, PLAYER, FLOOR, PARTICLE, MAX = 16, };
    enum class COMPONENT_TYPE : uint8_t { TEST_A, TEST_B,
                                        TRANSFORM, COLLIDER, RIGIDBODY, SCRIPT, SPRITE_RENDERER, ANIMATOR, CAMERA, AUDIO_LISTENER, AUDIO_SOURCE, END };
	enum                                { COMPONENT_MAX = static_cast<uint32_t>(COMPONENT_TYPE::END) };

	enum class RESOURCE_TYPE : uint8_t  { TEXTURE, MESH, MATERIAL, ANIMATION, AUDIO_CLIP, SHADER, PREFAB, END };
	enum class COLLIDER_TYPE : uint8_t  { SPHERE, BOX, CAPSULE, END };
	enum class BODY_TYPE : uint8_t      { STATIC, KINEMATIC, DYNAMIC, END };
	enum class UI_TYPE : uint8_t        { BUTTON, IMAGE, PANEL, SLOT, END };

	enum class STATE : uint8_t          { RIGHT, UP, LOOK, POSITION, END };

    enum class EVENT_TYPE : uint8_t     { None, On_GameObject_Created, On_GameObject_Destroyed, On_Scene_Changed, On_Window_Resize };

    enum class ASSET_TYPE : uint8_t     { UNKNOWN = 0, FOLDER, TEXTURE, MESH, MODEL, MATERIAL, SCENE, PREFAB, PROTOTYPE, SCRIPT, };

    /* ------ LOG ------ */
	enum class SEVERITY_TYPE : uint8_t  { INFO, WARN, ERR, ASSERTION, END };
	enum class DOMAIN_TYPE : uint8_t    { ENGINE, CLIENT, EDITOR, END };

}
#endif // Engine_Enum_h__
