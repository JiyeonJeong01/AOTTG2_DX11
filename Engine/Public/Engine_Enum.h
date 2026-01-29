#ifndef Engine_Enum_h__
#define Engine_Enum_h__



namespace Engine
{
	enum class WINMODE { FULL, WIN };

	enum class LAYER_TYPE		{ NONE, BACKGROUND, PLAYER, FLOOR, PARTICLE, MAX = 16, };
	enum class COMPONENT_TYPE	{
		TEST_A, TEST_B, TRANSFORM, COLLIDER, RIGIDBODY, SCRIPT, SPRITE_RENDERER, ANIMATOR, CAMERA, AUDIO_LISTENER, AUDIO_SOURCE, END };

	enum { MAX_COMPONENT = static_cast<uint32_t>(COMPONENT_TYPE::END) };

	enum class RESOURCE_TYPE	{ TEXTURE, MESH, MATERIAL, ANIMATION, AUDIO_CLIP, SHADER, PREFAB, END };
	enum class COLLIDER_TYPE	{ SPHERE, BOX, CAPSULE, END };
	enum class BODY_TYPE		{ STATIC, KINEMATIC, DYNAMIC, END };
	enum class UI_TYPE			{ BUTTON, IMAGE, PANEL, SLOT, END };

	enum class AXIS_TYPE		{ X, Y, Z, END };

    enum class EVENT_TYPE { GameObject_Created, GameObject_Destroyed, Window_Resize };

    /* ------ LOG ------ */
	enum class SEVERITY_TYPE : uint8_t { INFO, WARN, ERR, ASSERTION };
	enum class DOMAIN_TYPE : uint8_t { ENGINE, CLIENT, EDITOR };

}
#endif // Engine_Enum_h__
