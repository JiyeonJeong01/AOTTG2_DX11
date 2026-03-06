#ifndef Engine_Function_h__
#define Engine_Function_h__


namespace Engine
{
    template <typename T, typename U>
    constexpr  T To(U&& value) {
        return static_cast<T>(std::forward<U>(value));
    }

	template<typename T>
	void Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete[] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	_uint Safe_AddRef(T& pInstance)
	{
		_uint		iRefCnt = {};
		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->AddRef();
		}

		return iRefCnt;
	}

	template<typename T>
	_uint Safe_Release(T& pInstance)
	{
		_uint	iRefCnt = {};
		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->Release();

			if (0 == iRefCnt)
				pInstance = nullptr;
		}

		return iRefCnt;
	}

    template<typename T>
    requires std::is_enum_v<T>
    constexpr bool Has_Flag(T value, T flag) {
        return (static_cast<std::underlying_type_t<T>>(value) & static_cast<std::underlying_type_t<T>>(flag)) != 0;
    }

}

#endif // Engine_Function_h__
