#ifndef CHOCOBO1_CRC_64_XZ_H
#define CHOCOBO1_CRC_64_XZ_H

#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#if (__cplusplus > 201703L)
#include <version>
#endif

#ifndef USE_STD_SPAN_CHOCOBO1_HASH
#if (__cpp_lib_span >= 202002L)
#define USE_STD_SPAN_CHOCOBO1_HASH 1
#else
#define USE_STD_SPAN_CHOCOBO1_HASH 0
#endif
#endif

#if (USE_STD_SPAN_CHOCOBO1_HASH == 1)
#include <span>
#else
#include "gsl/span"
#endif


namespace Chocobo1
{
namespace Hash
{
#ifndef CONSTEXPR_CPP17_CHOCOBO1_HASH
#if __cplusplus >= 201703L
#define CONSTEXPR_CPP17_CHOCOBO1_HASH constexpr
#else
#define CONSTEXPR_CPP17_CHOCOBO1_HASH
#endif
#endif

#ifndef CHOCOBO1_HASH_ROR_IMPL
#define CHOCOBO1_HASH_ROR_IMPL
	template <typename R, typename T>
	constexpr R ror(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<R>::value, "");
		static_assert(std::is_unsigned<T>::value, "");
		return static_cast<R>(x >> s);
	}
#endif


namespace CRC_64_XZ_NS
{
	class CRC_64_XZ
	{
		public:
			using Byte = uint8_t;
			using ResultArrayType = std::array<Byte, 8>;

#if (USE_STD_SPAN_CHOCOBO1_HASH == 1)
			template <typename T, std::size_t Extent = std::dynamic_extent>
			using Span = std::span<T, Extent>;
#else
			template <typename T, std::size_t Extent = gsl::dynamic_extent>
			using Span = gsl::span<T, Extent>;
#endif

			constexpr CRC_64_XZ();

			constexpr void reset();
			constexpr CRC_64_XZ& finalize();

			std::string toString() const;
			std::vector<Byte> toVector() const;
			CONSTEXPR_CPP17_CHOCOBO1_HASH ResultArrayType toArray() const;
			template <typename T>
			CONSTEXPR_CPP17_CHOCOBO1_HASH operator T() const noexcept;

			constexpr CRC_64_XZ& addData(Span<const Byte> inData);
			constexpr CRC_64_XZ& addData(const void *ptr, std::size_t length);
			template <std::size_t N>
			constexpr CRC_64_XZ& addData(const Byte (&array)[N]);
			template <typename T, std::size_t N>
			CRC_64_XZ& addData(const T (&array)[N]);
			template <typename T>
			CRC_64_XZ& addData(Span<T> inSpan);

			friend constexpr bool operator==(const CRC_64_XZ &left, const CRC_64_XZ &right)
			{
				return (left.m_h == right.m_h);
			}

			friend constexpr bool operator!=(const CRC_64_XZ &left, const CRC_64_XZ &right)
			{
				return !(left == right);
			}

		private:
			constexpr void addDataImpl(Span<const Byte> data);

			uint64_t m_h = 0;
	};


	constexpr CRC_64_XZ::CRC_64_XZ()
	{
		static_assert((CHAR_BIT == 8), "Sorry, we don't support exotic CPUs");
		reset();
	}

	constexpr void CRC_64_XZ::reset()
	{
		m_h = UINT64_MAX;
	}

	constexpr CRC_64_XZ& CRC_64_XZ::finalize()
	{
		m_h ^= UINT64_MAX;
		return (*this);
	}

	inline std::string CRC_64_XZ::toString() const
	{
		const auto digest = toArray();
		std::string ret;
		ret.resize(2 * digest.size());

		auto *retPtr = &ret.front();
		for (const auto c : digest)
		{
			const Byte upper = ror<Byte>(c, 4);
			*(retPtr++) = static_cast<char>((upper < 10) ? (upper + '0') : (upper - 10 + 'a'));

			const Byte lower = c & 0xf;
			*(retPtr++) = static_cast<char>((lower < 10) ? (lower + '0') : (lower - 10 + 'a'));
		}

		return ret;
	}

	inline std::vector<CRC_64_XZ::Byte> CRC_64_XZ::toVector() const
	{
		const auto digest = toArray();
		return {digest.begin(), digest.end()};
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH inline CRC_64_XZ::ResultArrayType CRC_64_XZ::toArray() const
	{
		const int dataSize = sizeof(m_h);

		ResultArrayType ret {};
		auto *retPtr = ret.data();
		for (int j = (dataSize - 1); j >= 0; --j)
			*(retPtr++) = ror<Byte>(m_h, (j * 8));

		return ret;
	}

	template <typename T>
	CONSTEXPR_CPP17_CHOCOBO1_HASH CRC_64_XZ::operator T() const noexcept
	{
		static_assert(std::is_unsigned<T>::value, "");

		const auto digest = toArray();
		T ret = 0;
		for (int i = 0, iMax = static_cast<int>(std::min(sizeof(T), digest.size())); i < iMax; ++i)
		{
			ret <<= 8;
			ret |= digest[i];
		}
		return ret;
	}

	constexpr CRC_64_XZ& CRC_64_XZ::addData(const Span<const Byte> inData)
	{
		addDataImpl(inData);
		return (*this);
	}

	constexpr CRC_64_XZ& CRC_64_XZ::addData(const void *ptr, const std::size_t length)
	{
		return addData({static_cast<const Byte*>(ptr), length});
	}

	template <std::size_t N>
	constexpr CRC_64_XZ& CRC_64_XZ::addData(const Byte (&array)[N])
	{
		return addData({array, N});
	}

	template <typename T, std::size_t N>
	CRC_64_XZ& CRC_64_XZ::addData(const T (&array)[N])
	{
		return addData({reinterpret_cast<const Byte*>(array), (sizeof(T) * N)});
	}

	template <typename T>
	CRC_64_XZ& CRC_64_XZ::addData(const Span<T> inSpan)
	{
		return addData({reinterpret_cast<const Byte*>(inSpan.data()), inSpan.size_bytes()});
	}

	constexpr void CRC_64_XZ::addDataImpl(const Span<const Byte> data)
	{
		constexpr uint64_t polynomial = UINT64_C(0xC96C5795D7870F42);

		for (std::size_t i = 0; i < static_cast<std::size_t>(data.size()); ++i)
		{
			m_h ^= data[i];
			for (int bit = 0; bit < 8; ++bit)
				m_h = (m_h >> 1) ^ ((m_h & 1) ? polynomial : 0);
		}
	}
}
}

	using CRC_64_XZ = Hash::CRC_64_XZ_NS::CRC_64_XZ;
}

namespace std
{
	template <>
	struct hash<Chocobo1::CRC_64_XZ>
	{
		CONSTEXPR_CPP17_CHOCOBO1_HASH size_t operator()(const Chocobo1::CRC_64_XZ &hash) const noexcept
		{
			return hash;
		}
	};
}

#endif  // CHOCOBO1_CRC_64_XZ_H
