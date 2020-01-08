#pragma once

#include <cmath>
#include <type_traits>

namespace mh
{
	namespace detail::interpolation_hpp
	{
		template<typename T>
		constexpr T round(T in)
		{
			if constexpr (std::is_integral_v<T>)
			{
				return in;
			}
#if __cpp_lib_is_constant_evaluated >= 201811
			else if (!std::is_constant_evaluated())
			{
				return std::round(in);
			}
#endif
			else
			{
				if (in >= 0)
					return in + 0.5f;
				else
					return in - 0.5f;
			}
		}

		template<typename TIn, typename TOut>
		constexpr TOut clamp(TIn in, TOut out_min, TOut out_max)
		{
			if (in <= static_cast<TIn>(out_min))
				return out_min;
			if (in >= static_cast<TIn>(out_max))
				return out_max;

			if constexpr (std::is_floating_point_v<TIn> && std::is_integral_v<TOut>)
				in = round(in);

			return static_cast<TOut>(in);
		}

		template<typename T>
		constexpr bool will_overflow_mul(T a, T b)
		{
			if (a == 0)
				return false;

			return T(T(a * b) / a) != b;
		}

		template<typename T>
		constexpr bool will_overflow_add(T a, T b)
		{
			return T(a + b) < a;
		}
	}

	template<typename TIn, typename TOut>
	constexpr TOut lerp_slow(TIn in_01, TOut out_min, TOut out_max)
	{
		static_assert(std::is_floating_point_v<TIn>);
		return (out_min * (1 - in_01)) + (out_max * in_01);
	}

	template<typename TIn, typename TOut>
	constexpr TOut lerp(TIn in_01, TOut out_min, TOut out_max)
	{
		static_assert(std::is_floating_point_v<TIn>);
		return out_min + (out_max - out_min) * in_01;
	}

	template<typename TIn, typename TOut>
	constexpr TOut lerp_clamped(TIn in_01, TOut out_min, TOut out_max)
	{
		using ct = std::common_type_t<TIn, TOut>;

		return detail::interpolation_hpp::clamp(
			lerp<TIn, ct>(in_01, out_min, out_max),
			out_min, out_max);
	}

	template<typename TIn, typename TOut>
	constexpr TOut lerp_slow_clamped(TIn in_01, TOut out_min, TOut out_max)
	{
		using ct = std::common_type_t<TIn, TOut>;

		return detail::interpolation_hpp::clamp(
			lerp_slow<TIn, ct>(in_01, out_min, out_max),
			out_min, out_max);
	}

	template<typename TIn, typename TOut = float>
	constexpr TIn remap_to_01(TIn in, TIn in_min, TIn in_max)
	{
		static_assert(std::is_floating_point_v<TIn>);
		return TOut(TOut(in) - TOut(in_min)) / TOut(TOut(in_max) - TOut(in_min));
	}

	template<typename TIn, typename TOut>
	constexpr TOut remap(TIn in, TIn in_min, TIn in_max, TOut out_min, TOut out_max)
	{
		return lerp(remap_to_01(in, in_min, in_max), out_min, out_max);
	}

	template<typename TIn, typename TOut>
	constexpr TOut remap_clamped(TIn in, TIn in_min, TIn in_max, TOut out_min, TOut out_max)
	{
		return lerp_clamped(remap_to_01(in, in_min, in_max), out_min, out_max);
	}
}
