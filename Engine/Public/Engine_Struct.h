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
        static constexpr uint32_t INVALID_COMPONENT_SLOT = 0;
        static constexpr uint32_t FIRST_COMPONENT = 0;

    }

    namespace Layer
    {
        using LAYER_ID = uint8_t;
        using LAYER_MASK = uint32_t;
        constexpr LAYER_ID INVALID_LAYER = 0xff;
        constexpr uint32_t DEFAULT_LAYER = 0;
        constexpr uint32_t MAX_LAYERS = 32;

        constexpr LAYER_MASK To_Bit(LAYER_ID layer)
        {
            return (layer == INVALID_LAYER) ? 0u : (1u << layer);
        }
    }

    namespace ProjectConfig
    {
        const std::string PATH = "../../Client/Bin/";
        const std::string ROOT = "Assets";
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

    /* TODO : ===============================================================================*/
    typedef struct ENGINE_DLL tagLabel
    {
    private:
        std::string label{};
    public:
        tagLabel() = default;
        tagLabel(std::string str)
            : label(std::move(str)) {}

        void Set_Label(std::string_view str)
        {
            label.assign(str);
        }
        std::string_view Get_Label() const
        {
            return label;
        }

    }LABEL;

    typedef struct tagGameObjectHandle
    {
        uint32_t    iIndex = 0;
        uint32_t    iVersion = 0;

        bool IsValid() const
        {
            return iIndex != 0;
        }
        bool operator==(const tagGameObjectHandle& other) const
        {
            return iIndex == other.iIndex && iVersion == other.iVersion;
        }
        bool operator!=(const tagGameObjectHandle& other) const
        {
            return !(*this == other);
        }

    }GAMEOBJECT_HANDLE;

    typedef struct tagGameObjectData
    {
        uint32_t    iVersion = 0;       /* slot's current version */
        bool        bActive = false;

        /* For layer access in O(1) */
        Layer::LAYER_ID layer = Layer::INVALID_LAYER;
        uint32_t iIndexInLayer = 0;

        /* Components */
        uint32_t iComponentSlots[COMPONENT_MAX] = { 0, };

        /* Hierarchy */
        GAMEOBJECT_HANDLE           hParent{};
        std::vector<GAMEOBJECT_HANDLE>   hChildren;

        /* Reset helper */
        void Reset()
        {
            bActive = false;
            layer = Layer::INVALID_LAYER;
            iIndexInLayer = 0;
            std::fill(std::begin(iComponentSlots), std::end(iComponentSlots), 0);
            hParent = {};
            hChildren.clear();
        }

    }GAMEOBJECT_DATA;

#define COMPONENT_SPEC_TYPE(_TYPE)                                      \
    static constexpr COMPONENT_TYPE TYPE = _TYPE;                       \
    COMPONENT_TYPE Get_Type() const noexcept override { return TYPE; }

    struct COMPONENT_SPEC_BASE
    {
        virtual ~COMPONENT_SPEC_BASE() = default;
        virtual COMPONENT_TYPE Get_Type() const noexcept = 0;
    };

    typedef struct ENGINE_DLL tagComponentSpecBundle
    {
        std::vector<COMPONENT_SPEC_BASE*> components;
        template<typename TSpec>
        const TSpec* Find_One() const
        {
            for (const auto& pSpec : components)
            {
                if (pSpec->Get_Type() == TSpec::TYPE)
                    return SCAST(const TSpec*, pSpec);
            }

            return nullptr;
        }

    } COMPONENT_SPEC_BUNDLE;

    using COMPONENT_MASK = uint32_t;
    using PROTOTYPE_KEY = std::string;

    typedef struct ENGINE_DLL tagPrototypeSpec
    {
        PROTOTYPE_KEY key;
        std::string  strName;

        COMPONENT_SPEC_BUNDLE tComponentBundle;

        std::vector<tagPrototypeSpec> children;

        Layer::LAYER_ID layer = Layer::INVALID_LAYER;
    }PROTOTYPE_SPEC;



}

#endif // Engine_Struct_h__
