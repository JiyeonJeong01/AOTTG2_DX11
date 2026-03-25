#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3dcompiler.h>

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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>

#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"

#include <DirectXTK/VertexTypes.h>
#include <DirectXTK/PrimitiveBatch.h>
#include <DirectXTK/Effects.h>
#include "DirectXCollision.h"
#include "DirectXColors.h"
#include <DirectXTK/SpriteBatch.h>
#include <DirectXTK/SpriteFont.h>

#include "Fx11/d3dx11effect.h"

#include "../ThirdParty/json.hpp"
using json = nlohmann::json;

using namespace std;

#pragma warning(disable : 4251)

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

#include <type_traits>

#define DIRECTINPUT_VERSION		0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")

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
