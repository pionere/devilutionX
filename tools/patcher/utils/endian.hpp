#pragma once

#include <cstdint>

DEVILUTION_BEGIN_NAMESPACE

template <typename T>
constexpr std::uint16_t LoadLE16(const T* b)
{
	static_assert(sizeof(T) == 1, "invalid argument");
	// NOLINTNEXTLINE(readability-magic-numbers)
	return (static_cast<std::uint16_t>(b[1]) << 8) | static_cast<std::uint16_t>(b[0]);
}

template <typename T>
constexpr std::uint32_t LoadLE32(const T* b)
{
	static_assert(sizeof(T) == 1, "invalid argument");
	// NOLINTNEXTLINE(readability-magic-numbers)
	return (static_cast<std::uint32_t>(b[3]) << 24) | (static_cast<std::uint32_t>(b[2]) << 16) | (static_cast<std::uint32_t>(b[1]) << 8) | static_cast<std::uint32_t>(b[0]);
}

template <typename T>
constexpr std::uint32_t LoadBE32(const T* b)
{
	static_assert(sizeof(T) == 1, "invalid argument");
	// NOLINTNEXTLINE(readability-magic-numbers)
	return (static_cast<std::uint32_t>(b[0]) << 24) | (static_cast<std::uint32_t>(b[1]) << 16) | (static_cast<std::uint32_t>(b[2]) << 8) | static_cast<std::uint32_t>(b[3]);
}

DEVILUTION_END_NAMESPACE
