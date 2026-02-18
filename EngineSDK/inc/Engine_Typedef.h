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


    /* 저장용 */
    typedef		XMFLOAT2					_float2;
    typedef		XMFLOAT3					_float3;
    typedef		XMFLOAT4					_float4;
    typedef		XMFLOAT4X4					_float4x4;

    /*  SIMD 연산용 */
    typedef		XMVECTOR					_vector;
    typedef		FXMVECTOR					_fvector;
    typedef		GXMVECTOR					_gvector;
    typedef		HXMVECTOR					_hvector;
    typedef		CXMVECTOR					_cvector;

    typedef		XMMATRIX					_matrix;
    typedef		FXMMATRIX					_fmatrix;
    typedef		CXMMATRIX					_cmatrix;

    namespace Component
    {
        using  COMPONENT_ID = uint32_t;
        static constexpr COMPONENT_ID INDEX_MASK    = 0x000FFFFF;       // 하위 20비트 : 페이지 인덱스
        static constexpr COMPONENT_ID VERSION_MASK  = 0x7FF00000;       // 중간 11비트 (0x80000000 제외)
        static constexpr COMPONENT_ID GROUP_FLAG    = 0x80000000;       // MSB 1비트 : 멀티 여부
        static constexpr COMPONENT_ID DATA_MASK     = 0x7FFFFFFF;       // 나머지 31비트 : 데이터 추출
        static constexpr COMPONENT_ID MAX_VERSION   = 0x000007FF;       // 안전한 최대 버전
        static constexpr uint32_t VERSION_SHIFT     = 20;
        static constexpr COMPONENT_ID INVALID_COMPONENT_SLOT = 0;

        using COMPONENT_MASK = uint32_t;

        constexpr COMPONENT_MASK Component_Bit(COMPONENT_TYPE t) noexcept
        {
            return 1u << SCAST(uint32_t, t);
        }

        static constexpr uint64_t UNIQUE_MASK =
            Component::Component_Bit(COMPONENT_TYPE::TRANSFORM) |
            Component::Component_Bit(COMPONENT_TYPE::RIGIDBODY) |
            Component::Component_Bit(COMPONENT_TYPE::MESH_RENDERER);

        static inline _bool Is_Multi_Allowed(COMPONENT_TYPE eType)
        {
            return (Component::UNIQUE_MASK & Component::Component_Bit(eType)) == 0;
        }

    }

    namespace Layer
    {
        using LAYER_ID = uint8_t;
        using LAYER_MASK = uint32_t;
        constexpr uint32_t MAX_LAYERS = 32;

        constexpr LAYER_ID INVALID_LAYER = 0xff;
        constexpr LAYER_ID DEFAULT_LAYER = 0;
        constexpr LAYER_ID UI_LAYER = MAX_LAYERS - 1;

        constexpr LAYER_MASK To_Bit(LAYER_ID layer)
        {
            return (layer == INVALID_LAYER) ? 0u : (1u << layer);
        }
    }


    /* ------------------------------------------------ */
    /*                      GUIDE                       */   
    /* ------------------------------------------------ */

    /**
     * @brief =========== DirectX Math 타입 가이드 ===========
     *
     *   [저장용] XMFLOATn, XMFLOAT4X4 (멤버 변수용)
     *
     *   [연산용] XMVECTOR, XMMATRIX (지역 변수용)
     *
     *   [매개변수 전달 규칙]
     * - FXMVECTOR : 1~3번째 벡터 인자
     *
     * - GXMVECTOR : 4번째 벡터 인자
     *
     * - HXMVECTOR : 5번째 벡터 인자
     *
     * - CXMVECTOR : 6번째 이후 또는 모든 행렬(XMMATRIX) 인자
     *
     */
    class TYPE_TIP {};




}


#endif// Engine_Typedef_h__
