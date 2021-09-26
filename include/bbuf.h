#pragma once
#include <cassert>
#include <type_traits>
#include <stdexcept>
#include <utility>

#include <string.h>

#ifdef __BIG_ENDIAN__
#error Big endian not supported
#endif

namespace minmsg {
	namespace detail {
		class bbuf {
			char* m_data = nullptr;
			size_t m_data_size = 0;
			size_t m_index = 0;
		protected:
			bbuf(char* data, size_t data_size) :
				m_data{data},
				m_data_size{data_size}
			{}

			template<typename T>
			constexpr T get(bool peek = false) {
				static_assert(std::is_pointer_v<T> == false, "T must not be a pointer");
				static_assert(std::is_fundamental_v<T>, "T must be a fundamental type");

				check_bounds_critical(sizeof(T));

				// Storing T temporarily could actually be more expensive than just storing the index
				size_t old_index = m_index;
				if (!peek)
					m_index += sizeof(T);

				return *(T*)((char*)m_data + old_index);
			}

			template<typename T>
			constexpr T* get_ptr(bool peek = false) {
				static_assert(std::is_pointer_v<T> == false, "T must not be a pointer");
				static_assert(std::is_fundamental_v<T>, "T must be a fundamental type");

				check_bounds_critical(sizeof(T));

				size_t old_index = m_index;
				if(!peek)
					m_index += sizeof(T);
				return (T*)((char*)m_data + old_index);
			}

			constexpr void* get_head() {
				return (void*)((char*)m_data + m_index);
			}

			constexpr void advance(size_t amount) {
				m_index += amount;
			}

			constexpr void check_bounds_critical(size_t size) {
				if(m_index + size > m_data_size)
					error("bitbuffer overflow");
			}

			constexpr bool check_bounds(size_t size) {
				return m_index + size < m_data_size;
			}

			inline void error(const char* msg) {
				throw std::runtime_error(msg);
			}
		public:
			bool copy_to(void* buf, size_t buf_size) {
				if (buf_size < m_index)
					return false;

				memcpy(buf, m_data, m_index);
			}

			void clear() {
				memset(m_data, 0, m_index);
			}

			void seek(size_t pos) {
				m_index = pos;
			}

			const size_t size() {
				return m_index;
			}

			const void* data() {
				return m_data;
			}
		};
	}

	class bbuf_read : public detail::bbuf {
	public:
		bbuf_read(char* data, size_t data_size)
			: detail::bbuf(data, data_size)
		{}

		template<int Size>
		bbuf_read(char(&Buffer)[Size])
			: detail::bbuf(Buffer, Size)
		{}

		template<typename T>
		constexpr T read(bool peek = false) {
			if constexpr (std::is_same_v<std::remove_all_extents_t<T>, std::string>)
				return read_string();
			else
				return get<T>(peek);
		}

		inline std::string read_string(uint16_t max_size = 0) {
			const uint16_t string_size = get<uint16_t>();

			if (max_size != 0 && string_size > max_size)
				error("string too large");

			check_bounds_critical(string_size);

			const void* begin = get_head();

			std::string out(string_size, 0);

			// Copy data from head into string
			memcpy(out.data(), begin, string_size);

			// Advance index by size of string
			advance(string_size);
			return out;
		}

		inline bool read_bytes(void* buf, uint16_t buffer_size, uint16_t* read_bytes = nullptr) {
			const uint16_t size = get<uint16_t>();

			if (size > buffer_size)
				return false;
			
			if (!check_bounds(size))
				return false;

			const void* begin = get_head();

			// Copy data from head into buffer
			memcpy(buf, begin, size);

			if (read_bytes)
				*read_bytes = size;

			// Advance index by number of bytes
			advance(size);
			return true;
		}
	};

	class bbuf_write : public detail::bbuf {
	public:
		bbuf_write(char* data, size_t data_size)
			: detail::bbuf(data, data_size)
		{}

		template<int Size>
		bbuf_write(char(&Buffer)[Size])
			: detail::bbuf(Buffer, Size)
		{}

		template<typename T>
		constexpr void write(T x) {
			if constexpr (std::is_same_v<T, std::string>)
				write_string(x);
			else
				*get_ptr<T>() = x;
		}

		inline void write_string(const std::string& str) {
			check_bounds_critical(str.size());

			// Write string size
			write<uint16_t>(str.size());

			// Write in string data
			memcpy((void*)get_head(), str.data(), str.size());

			// Advance index by string size
			advance(str.size());
		}

		inline void write_bytes(void* buf, uint16_t size) {
			check_bounds_critical(size);

			// Write buffer size
			write<uint16_t>(size);

			// Write in data
			memcpy((void*)get_head(), buf, size);

			// Advance index by size
			advance(size);
		}
	};
}