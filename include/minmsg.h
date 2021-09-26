#pragma once
#include "bbuf.h"
#include <utility>

namespace minmsg {
	namespace detail {
		template<typename T, typename Enabled = void, typename Enabled2 = void>
		struct is_packable : std::false_type {};

		template<typename T>
		struct is_packable
			<T,
			std::enable_if_t<std::is_member_function_pointer_v<decltype(&T::minmsg_pack)>,
			std::enable_if_t<std::is_member_function_pointer_v<decltype(&T::minmsg_unpack)>
			>
			>
			> {
			static constexpr bool value = T::minmsg_packable() == 0xF00F;
		};

		template<typename T>
		struct base {
			using type = std::remove_reference_t<std::remove_pointer_t<T>>;
		};

		template<typename T>
		using base_t = typename base<T>::type;
	}

	class packer {
		static void pack(bbuf_write*) {}
		static void unpack(bbuf_read*) {}
	public:
		template<typename T, typename ...Args>
		constexpr static void pack(bbuf_write* buf, const T& x, Args&& ...args) {
			constexpr bool is_packable = detail::is_packable<detail::base_t<T>>::value;

			if constexpr (is_packable)
				x->minmsg_pack(buf);
			else
				buf->write(*x);

			pack(buf, std::forward<Args>(args)...);
		}

		template<typename T, typename ...Args>
		constexpr static void unpack(bbuf_read* buf, const T& x, Args&& ...args) {
			constexpr bool is_packable = detail::is_packable<detail::base_t<T>>::value;
			
			if constexpr (is_packable)
				x->minmsg_unpack(buf);
			else
				*x = buf->read<detail::base_t<T>>();

			unpack(buf, std::forward<Args>(args)...);
		}
	};

	template<typename T>
	constexpr void pack(bbuf_write* buf, T* object) {
		object->minmsg_pack(buf);
	}

	template<typename T>
	constexpr void unpack(bbuf_read* buf, T* object) {
		object->minmsg_unpack(buf);
	}
}

#define MINMSG_ADD_FIELDS(...)								\
constexpr static int minmsg_packable() { return 0xF00F; }	\
void minmsg_pack(minmsg::bbuf_write* buf) {					\
	minmsg::packer::pack(buf, __VA_ARGS__);					\
}															\
void minmsg_unpack(minmsg::bbuf_read* buf) {				\
	minmsg::packer::unpack(buf, __VA_ARGS__);				\
}															