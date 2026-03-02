#pragma once
#include <filesystem>
#include "Engine_SDK.h"

#ifdef new
#undef new
#endif

#include "scene.h"
#include "Importer.hpp"
#include "postprocess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Converter_Struct.h"

namespace Converter {};

using namespace Converter;
