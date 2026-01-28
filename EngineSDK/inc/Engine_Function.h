#ifndef Engine_Function_h__
#define Engine_Function_h__


namespace Engine
{
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

}

#endif // Engine_Function_h__
