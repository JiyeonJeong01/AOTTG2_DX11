#pragma once
#include <random>
#include <type_traits>
#include <limits>
#include <algorithm>

#ifdef max
#undef max
#endif

class CRandomUtil final
{
public:
    static int Get_Int(int iMin, int iMax)
    {
        if (iMin > iMax)
            std::swap(iMin, iMax);

        std::uniform_int_distribution<int> dist(iMin, iMax);
        return dist(Get_Engine());
    }

    static float Get_Float(float fMin, float fMax)
    {
        if (fMin > fMax)
            std::swap(fMin, fMax);

        std::uniform_real_distribution<float> dist(
            fMin,
            std::nextafter(fMax, std::numeric_limits<float>::max())
        );

        float fValue = dist(Get_Engine());

        return std::clamp(fValue, fMin, fMax);
    }

    template <typename T>
    static T Get(T tMin, T tMax)
    {
        if constexpr (std::is_same_v<T, int>)
        {
            return Get_Int(tMin, tMax);
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return Get_Float(tMin, tMax);
        }
        else
        {
            static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>,
                "CRandomUtil::Get only supports int and float.");
        }
    }

private:
    static std::mt19937& Get_Engine()
    {
        static std::random_device rd;
        static std::mt19937 engine(rd());
        return engine;
    }
};

#define max(a,b) (((a) > (b)) ? (a) : (b))
