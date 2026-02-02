#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <ctime>
#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <deque>
#include <mutex>
#include <new>


using namespace std;

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"
#include "Core_Struct.h"

#include <type_traits>

#define DIRECTINPUT_VERSION		0x8000
#include <dinput.h>

#pragma warning(disable : 4251)

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif // DBG_NEW
#endif // _DEBUG

using namespace Engine;

#endif // Engine_Define_h__
