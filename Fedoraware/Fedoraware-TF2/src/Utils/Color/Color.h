#pragma once

#include <string>
#include <format>
#include <vector>

#include "../Math/Math.h"

#define DEVELOPER_BUILD

using byte = unsigned char;

struct Color_t
{
	byte r = 0, g = 0, b = 0, a = 0;

	bool operator==(Color_t other) const
	{
		return (r == other.r && g == other.g && b == other.b && a == other.a);
	}

	bool operator!=(Color_t other) const
	{
		return r != other.r || g != other.g || b != other.b || a != other.a;
	}

	[[nodiscard]] std::string to_hex() const
	{
		return std::format("\x7{:02X}{:02X}{:02X}", r, g, b);
	}

	[[nodiscard]] std::string to_hex_alpha() const
	{
		return std::format("\x8{:02X}{:02X}{:02X}{:02X}", r, g, b, a);
	}

	[[nodiscard]] Color_t lerp(Color_t to, float t) const
	{
		//a + (b - a) * t
		return {
			static_cast<byte>(r + (to.r - r) * t),
			static_cast<byte>(g + (to.g - g) * t),
			static_cast<byte>(b + (to.b - b) * t),
			static_cast<byte>(a + (to.a - a) * t),
		};
	}
};

struct Gradient_t
{
	Color_t StartColor = { 0, 0, 0, 255 };
	Color_t EndColor = { 0, 0, 0, 255 };
};

struct Chams_t
{
	std::vector<std::string>	VisibleMaterial = { "Original" };
	std::vector<std::string>	OccludedMaterial = {};
	Color_t						VisibleColor = { 255, 255, 255, 255 };
	Color_t						OccludedColor = { 255, 255, 255, 255 };
};

struct Glow_t
{
	bool	Stencil = false;
	bool	Blur = false;
	int		StencilScale = 1;
	int		BlurScale = 1;

	bool operator==(const Glow_t& other) const
	{
		return (Stencil == other.Stencil && Blur == other.Blur && StencilScale == other.StencilScale && BlurScale == other.BlurScale);
	}
};

struct DragBox_t
{
	int x = 100;
	int y = 100;
	bool update = true;
};

struct WindowBox_t
{
	int x = 100;
	int y = 100;
	int w = 200;
	int h = 150;
	bool update = true;
};

namespace Color
{
	inline float TOFLOAT(byte x)
	{
		return (static_cast<float>(x) / 255.0f);
	}

	inline Vec3 TOVEC3(const Color_t& x)
	{
		return Vec3(TOFLOAT(x.r), TOFLOAT(x.g), TOFLOAT(x.b));
	}

	inline unsigned long TODWORD(const Color_t& x)
	{
		return (static_cast<unsigned long>(((x.r & 0xFF) << 24) | ((x.g & 0xFF) << 16) | ((x.b & 0xFF) << 8) | (x.a & 0xFF)));
	}
}