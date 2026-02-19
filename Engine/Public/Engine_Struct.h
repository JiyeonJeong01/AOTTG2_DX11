#ifndef Engine_Struct_h__
#define Engine_Struct_h__

#include "Engine_Enum.h"
#include "Engine_Typedef.h"

namespace  Engine
{
	typedef struct tagEngineDesc
	{
		HWND				hWnd;
        HINSTANCE           hInst;
		WINMODE		        eWinMode;
		std::pair<unsigned int, unsigned int> iViewportSize;
	} ENGINE_DESC;

    namespace ProjectConfig
    {
        const std::string PATH = "../../Client/Bin/";
        const std::string ROOT = "Assets";
    }


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

    typedef struct tagObjectHandle {
        uint32_t raw = 0;

        static constexpr uint32_t VERSION_SHIFT = 16;
        static constexpr uint32_t UI_SHIFT = 31;

        static constexpr uint32_t INDEX_MASK = 0x0000FFFFu;
        static constexpr uint32_t VERSION_MASK = 0x7FFF0000u;
        static constexpr uint32_t UI_MASK = 1u << UI_SHIFT;

        tagObjectHandle() = default;
        tagObjectHandle(uint32_t idx, uint32_t ver, _bool isUI)
            : raw((idx& INDEX_MASK) |
                (((ver & 0x7FFFu) << VERSION_SHIFT) & VERSION_MASK) |
                (isUI ? UI_MASK : 0u)) {
        }

        uint32_t Index()   const { return raw & INDEX_MASK; }
        uint32_t Version() const { return (raw & VERSION_MASK) >> VERSION_SHIFT; }

        _bool     Is_UI()    const { return (raw & UI_MASK) != 0; }
        _bool     Is_Valid() const { return (raw & INDEX_MASK) != 0; }

        _bool operator==(const tagObjectHandle& other) const { return raw == other.raw; }
        _bool operator!=(const tagObjectHandle& other) const { return raw != other.raw; }
    }OBJECT_HANDLE;

    typedef struct tagUIGlobal
    {
        _float4x4 matView{};
        _float4x4 matProj{};
        _float2   vViewport{};
        tagUIGlobal()
        {
            XMStoreFloat4x4(&matView, XMMatrixIdentity());
            XMStoreFloat4x4(&matProj, XMMatrixIdentity());
        }
    }UI_GLOBAL;

    typedef struct ENGINE_DLL tagEventData
    {
        tagEventData(EVENT_TYPE type) : eType(type) {}
        virtual ~tagEventData() = default;
        const EVENT_TYPE eType;
    }EVENT_DATA;
}

#endif // Engine_Struct_h__
