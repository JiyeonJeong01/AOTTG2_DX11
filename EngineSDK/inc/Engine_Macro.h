#ifndef Engine_Macro_h__
#define Engine_Macro_h__

namespace Engine
{
	#ifndef				MSG_BOX
	#define			MSG_BOX(_message)				MessageBox(nullptr, TEXT(_message), L"System Message", MB_OK);
	#endif				// MSG_BOX
	
	#define			NS_BEGIN(NAMESPACE)		namespace NAMESPACE {
	#define			NS_END									}
	
	#define			USING(NAMESPACE)				using namespace NAMESPACE;
	
	#ifdef				ENGINE_EXPORTS
	#define			ENGINE_DLL							_declspec(dllexport)
	#else
	#define			ENGINE_DLL							_declspec(dllimport)
	#endif				 // ENGINE_EXPORTS

	#define SCAST(type, value) static_cast<type>(value)

	#define NULL_CHECK( _ptr)	\
			{if( _ptr == 0){ return;}}
	
	#define NULL_CHECK_RETURN( _ptr, _return)	\
			{if( _ptr == 0){return _return;}}
	
	#define NULL_CHECK_MSG( _ptr, _message )		\
			{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);}}
	
	#define NULL_CHECK_RETURN_MSG( _ptr, _return, _message )	\
			{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);return _return;}}
	
	#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
			{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK);  return E_FAIL;}
	
	#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )		\
			{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK);  return _return;}
	
	#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )	\
			{ MessageBoxW(NULL, _message, L"System Message",MB_OK); return E_FAIL;}
	
	#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
			{ MessageBoxW(NULL, _message, L"System Message",MB_OK); return _return;}
	
	
#define NO_COPY(CLASSNAME)                                  \
    CLASSNAME(const CLASSNAME&) = delete;                   \
    CLASSNAME& operator=(const CLASSNAME&) = delete;

#define NO_MOVE(CLASSNAME)                                  \
    CLASSNAME(CLASSNAME&&) = delete;                        \
    CLASSNAME& operator=(CLASSNAME&&) = delete;

#define DECLARE_SINGLETON(CLASSNAME)                        \
private:                                                    \
    CLASSNAME(const CLASSNAME&) = delete;                   \
    CLASSNAME& operator=(const CLASSNAME&) = delete;        \
    CLASSNAME(CLASSNAME&&) = delete;                        \
    CLASSNAME& operator=(CLASSNAME&&) = delete;             \
    static std::unique_ptr<CLASSNAME> m_pInstance;          \
public:                                                     \
    static CLASSNAME& GetInstance();                        \
    static CLASSNAME* GetInstancePtr();                     \
    static void DestroyInstance();                          \
    virtual ~CLASSNAME();                                   \
private:                                                    \
    CLASSNAME();

#define IMPLEMENT_SINGLETON(CLASSNAME)                          \
    std::unique_ptr<CLASSNAME> CLASSNAME::m_pInstance = nullptr; \
    CLASSNAME& CLASSNAME::GetInstance() {                       \
        if (!m_pInstance) {                                     \
            m_pInstance = std::unique_ptr<CLASSNAME>(new CLASSNAME()); \
        }                                                       \
        return *m_pInstance;                                    \
    }                                                           \
    CLASSNAME* CLASSNAME::GetInstancePtr() {                    \
        return m_pInstance.get();                               \
    }                                                           \
    void CLASSNAME::DestroyInstance() {                         \
        m_pInstance.reset();                                    \
    }

#define ENUM_BIT_OPERATORS(ENUM_NAME)                           \
    constexpr ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) {   \
        return static_cast<ENUM_NAME>(                          \
            static_cast<std::underlying_type_t<ENUM_NAME>>(a) | \
            static_cast<std::underlying_type_t<ENUM_NAME>>(b)   \
        );                                                      \
    }                                                           \
    constexpr ENUM_NAME operator&(ENUM_NAME a, ENUM_NAME b) {   \
        return static_cast<ENUM_NAME>(                          \
            static_cast<std::underlying_type_t<ENUM_NAME>>(a) & \
            static_cast<std::underlying_type_t<ENUM_NAME>>(b)   \
        );                                                      \
    }                                                           \

#define GET_INSTANCE(CLASSNAME) CLASSNAME::GetInstance()
#define SYS_CORE				GET_INSTANCE(Engine::CCore_System)

#define SYS_LOG		            GET_INSTANCE(Engine::CLogger)
#define SYS_COMPONENT           GET_INSTANCE(Engine::CComponent_System)
#define SYS_GAMEOBJECT          GET_INSTANCE(Engine::CGameObject_System)
#define SYS_ASSET               GET_INSTANCE(Engine::CAsset_Registry)
#define SYS_RESOURCE            GET_INSTANCE(Engine::CResource_System)
#define SYS_INPUT		        GET_INSTANCE(Engine::CInput_System)
#define SYS_RENDER		        GET_INSTANCE(Engine::CRender_System)
#define SYS_EVENT		        GET_INSTANCE(Engine::CEvent_System)
#define SYS_CINEMATIC           GET_INSTANCE(Engine::CCinematic_System)

#define SYS_EDITOR		        GET_INSTANCE(Engine::CEditor_System)

#define GAME_INSTANCE		    GET_INSTANCE(Engine::CGameInstance)


//#define RESOURCES	GET_INSTANCE(CResourceManager)
//#define SCENE		GET_INSTANCE(CScene_Handler)
//#define SOUND		GET_INSTANCE(CSoundManager)

}
#endif // Engine_Macro_h__
