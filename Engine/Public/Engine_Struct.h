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

}

#endif // Engine_Struct_h__
