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
	
	
#define NO_COPY(CLASSNAME)										\
			private:											\
			CLASSNAME(const CLASSNAME&) = delete;				\
			CLASSNAME& operator = (const CLASSNAME&) = delete;		

#define DECLARE_SINGLETON(CLASSNAME)							\
			NO_COPY(CLASSNAME)									\
			private:											\
			static CLASSNAME*	m_pInstance;					\
			public:												\
			static CLASSNAME*	GetInstance( void );			\
			static unsigned int DestroyInstance( void );			

#define IMPLEMENT_SINGLETON(CLASSNAME)							\
			CLASSNAME*	CLASSNAME::m_pInstance = nullptr;		\
			CLASSNAME*	CLASSNAME::GetInstance( void )	{		\
				if(nullptr == m_pInstance) {					\
					m_pInstance = new CLASSNAME;				\
				}												\
				return m_pInstance;								\
			}													\
			unsigned int CLASSNAME::DestroyInstance( void ) {	\
				unsigned int iRefCnt = {};						\
				if(nullptr != m_pInstance)	{					\
					iRefCnt = m_pInstance->Release();			\
					if(0 == iRefCnt)							\
						m_pInstance = nullptr;					\
				}												\
				return iRefCnt;									\
			}

#define GET_INSTANCE(CLASSNAME) CLASSNAME::GetInstance()
#define SYS_CORE				GET_INSTANCE(Engine::CCore_System)

#define SYS_LOG		            GET_INSTANCE(Engine::CLogger)
#define SYS_COMPONENT           GET_INSTANCE(Engine::CComponent_System)
#define SYS_GAMEOBJECT          GET_INSTANCE(Engine::CGameObject_System)

//#define INPUT		GET_INSTANCE(CInputSystem)
//#define TIME		GET_INSTANCE(CTimeManager)
//#define DT				   
//#define RESOURCES	GET_INSTANCE(CResourceManager)
//#define SCENE		GET_INSTANCE(CSceneManager)
//#define SOUND		GET_INSTANCE(CSoundManager)
//#define EVENT		GET_INSTANCE(CEvent_Manager)

}
#endif // Engine_Macro_h__
