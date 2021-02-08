#pragma once

#include <string_view>
#include <tuple>
#include <typeinfo>

namespace mh
{
#define MH_NOINLINE __declspec(noinline)

	template<typename TStruct, typename TMember>
	struct struct_member_info
	{
		std::string_view name;
		TStruct& obj;
		TMember& value;
		TMember TStruct::* pointer;
		size_t offset;
	};

	namespace detail::reflection::struct_hpp
	{
		template<typename T> struct impl;
		//{
			//template<typename T, typename TFunc, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, value_type>>>
		//	static constexpr void for_each(T&& obj, TFunc&& func);
		//};

		template<typename T>
		struct make_dependent
		{
			using type = T;
		};

#if 0
		template<typename T, typename TFunc>
		constexpr void for_each_member(T& obj, TFunc&& func)
		{
			impl<typename make_dependent<T>::type>::for_each(std::forward<typename make_dependent<T>::type>(obj), std::forward<TFunc>(func));
		}

#endif
		template<typename T>
		concept HasBaseTypes = requires
		{
			typename T::mh_struct_reflect_bases_t;
		};

		template<typename TObj, typename TFunc, size_t I>
		MH_NOINLINE constexpr void handle_bases_impl(TObj&& obj, TFunc&& func)
		{
			using tuple_t = typename TObj::mh_struct_reflect_bases_t;
			if constexpr (I >= std::tuple_size_v<tuple_t>)
			{
				return;
			}
			else
			{
				using current_t = std::tuple_element_t<I, tuple_t>;
				impl<current_t>::for_each(std::forward<current_t>(obj), std::forward<TFunc>(func));
				return handle_bases_impl<TObj, TFunc, I + 1>(std::forward<TObj>(obj), std::forward<TFunc>(func));
			}
		}

		template<HasBaseTypes TObj, typename TFunc>
		MH_NOINLINE constexpr void handle_bases(typename make_dependent<TObj>::type&& obj, TFunc&& func, int)
		{
			using bases_t = typename TObj::mh_struct_reflect_bases_t;
			handle_bases_impl<TObj, TFunc, 0>(std::forward<TObj>(obj), std::forward<TFunc>(func));
			//std::apply([&](auto&& baseValue)
			//	{
			//		// TODO
			//	}, std::declval<typename TStruct::mh_struct_reflect_bases_t>());
		}

#if 1
		template<typename TObj, typename TFunc>
		MH_NOINLINE constexpr auto handle_bases(TObj&&, TFunc&&, void*)
		{
			// No mh_struct_reflect_bases_t, so nothing to do.
		}
#endif

		template<typename T, typename TFunc>
		MH_NOINLINE constexpr void for_each_member(typename detail::reflection::struct_hpp::make_dependent<T>::type&& obj, TFunc&& func)
		{
			using decayed = std::decay_t<T>;
			detail::reflection::struct_hpp::impl<decayed>::for_each(std::forward<T>(obj), std::forward<TFunc>(func));
		}
	}

	template<typename T, typename TFunc>
	MH_NOINLINE constexpr void for_each_member(T&& obj, TFunc&& func)
	{
		detail::reflection::struct_hpp::for_each_member<T, TFunc>(std::forward<T>(obj), std::forward<TFunc>(func));
	}
}

#define MH_STRUCT_REFLECT_BASES(...) \
	using mh_struct_reflect_bases_t = std::tuple<__VA_ARGS__>;

#define MH_STRUCT_REFLECT_BEGIN(...) \
	template<> \
	struct mh::detail::reflection::struct_hpp::impl<__VA_ARGS__> \
	{ \
		using value_type = __VA_ARGS__; \
		\
		template<typename TObj, typename TFunc, typename = std::enable_if_t<std::is_same_v<std::decay_t<TObj>, value_type>>> \
		static constexpr void for_each(TObj&& obj, TFunc&& func) \
		{ \
			mh::detail::reflection::struct_hpp::handle_bases<TObj, TFunc>(std::forward<value_type>(obj), std::forward<TFunc>(func), 0);

//#define MH_STRUCT_REFLECT_BASE(...) \
//			mh::detail::reflection::struct_hpp::impl< __VA_ARGS__ >::for_each(std::forward<value_type>(obj), std::forward<TFunc>(func));

#define MH_STRUCT_REFLECT_MEMBER(member) \
			func(mh::struct_member_info<value_type, decltype(obj.member)>{ #member, obj, obj.member, &value_type::member, offsetof(value_type, member) });

#define MH_STRUCT_REFLECT_END() \
		} \
	};
