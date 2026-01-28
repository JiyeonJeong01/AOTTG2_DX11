#ifndef Engine_Struct_h__
#define Engine_Struct_h__

#include "Engine_Enum.h"

namespace  Engine
{
	typedef struct tagEngineDesc
	{
		HWND				hWnd;
		WINMODE		eWinMode;
		std::pair<unsigned int, unsigned int> iViewportSize;
	} ENGINE_DESC;

    namespace ComponentConfig
    {
        static constexpr uint32_t INDEX_MASK = 0x000FFFFF;      // 하위 20비트 : 페이지 인덱스
        static constexpr uint32_t VERSION_MASK = 0xFFF00000;    // 상위 12비트 : 
        static constexpr uint32_t GROUP_FLAG = 0x80000000;      // MSB 1비트 : 멀티 여부
        static constexpr uint32_t DATA_MASK = 0x7FFFFFFF;       // 나머지 31비트 : 데이터 추출
        static constexpr uint32_t MAX_VERSION = 0x000007FF;     // 안전한 최대 버전
        static constexpr uint32_t VERSION_SHIFT = 20;
    }


    typedef struct tagComponentHandle
    {
        uint32_t iHandle = 0;

        uint32_t Get_Index() const { return iHandle & ComponentConfig::INDEX_MASK; }
        uint32_t Get_Version() const { return (iHandle & ComponentConfig::VERSION_MASK) >> ComponentConfig::VERSION_SHIFT; }

        static tagComponentHandle Create(uint32_t index, uint32_t version)
        {
            tagComponentHandle h;
            h.iHandle = (index & ComponentConfig::INDEX_MASK) | ((version << ComponentConfig::VERSION_SHIFT) & ComponentConfig::VERSION_MASK);
            return h;
        }

        bool operator==(const tagComponentHandle& other) const { return iHandle == other.iHandle; }
        bool Is_Valid() const { return iHandle != 0; }
    }COMPONENT_HANDLE;

    typedef struct tagComponentGroup
    {
        COMPONENT_HANDLE            tPrimary; 
        vector<COMPONENT_HANDLE>    tExtras;
    }COMPONENT_GROUP;


}

#endif // Engine_Struct_h__