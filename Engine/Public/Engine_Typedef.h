#ifndef Engine_Typedef_h__
#define Engine_Typedef_h__

namespace Engine
{
	typedef		bool						_bool;

	typedef		signed char					_byte;
	typedef		unsigned char				_ubyte;
	typedef		char						_char;

	typedef		wchar_t						_tchar;
	typedef		wstring						_wstring;

	typedef		signed short				_short;
	typedef		unsigned short				_ushort;

	typedef		signed int					_int;
	typedef		unsigned int				_uint;

	typedef		signed long					_long;
	typedef		unsigned long				_ulong;

	typedef		float						_float;
	typedef		double						_double;

	typedef		XMFLOAT2					_float2;
	typedef		XMFLOAT3					_float3;
	typedef		XMFLOAT4					_float4;
	typedef		XMFLOAT4X4					_float4x4;

	typedef		XMVECTOR					_vector;
	typedef		XMMATRIX					_matrix;

    namespace Component
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
        constexpr LAYER_ID DEFAULT_LAYER = 0;
        constexpr uint32_t MAX_LAYERS = 32;

        constexpr LAYER_MASK To_Bit(LAYER_ID layer)
        {
            return (layer == INVALID_LAYER) ? 0u : (1u << layer);
        }
    }

}


#endif// Engine_Typedef_h__
