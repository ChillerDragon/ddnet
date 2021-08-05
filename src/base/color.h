
#ifndef BASE_COLOR_H
#define BASE_COLOR_H

#include "math.h"
#include "vmath.h"

/*
	Title: Color handling
*/
/*
	Function: RgbToHue
		Determines the hue from RGB values
*/
inline float RgbToHue(float r, float g, float b)
{
	float h_min = minimum(r, g, b);
	float h_max = maximum(r, g, b);

	float hue = 0.0f;
	if(h_max != h_min)
	{
		float c = h_max - h_min;
		if(h_max == r)
			hue = (g - b) / c + (g < b ? 6 : 0);
		else if(h_max == g)
			hue = (b - r) / c + 2;
		else
			hue = (r - g) / c + 4;
	}

	return hue / 6.0f;
}

// Curiously Recurring Template Pattern for type safety
template<typename DerivedT>
class color4_base
{
public:
	union
	{
		float x, r, h;
	};
	union
	{
		float y, g, s;
	};
	union
	{
		float z, b, l, v;
	};
	union
	{
		float w, a;
	};

	color4_base() {}

	color4_base(const vec4 &v4)
	{
		x = v4.x;
		y = v4.y;
		z = v4.z;
		a = v4.w;
	}

	color4_base(const vec3 &v3)
	{
		x = v3.x;
		y = v3.y;
		z = v3.z;
		a = 1.0f;
	}

	color4_base(float nx, float ny, float nz, float na)
	{
		x = nx;
		y = ny;
		z = nz;
		a = na;
	}

	color4_base(float nx, float ny, float nz)
	{
		x = nx;
		y = ny;
		z = nz;
		a = 1.0f;
	}

	color4_base(unsigned col, bool alpha = false)
	{
		a = alpha ? ((col >> 24) & 0xFF) / 255.0f : 1.0f;
		x = ((col >> 16) & 0xFF) / 255.0f;
		y = ((col >> 8) & 0xFF) / 255.0f;
		z = ((col >> 0) & 0xFF) / 255.0f;
	}

	vec4 v4() { return vec4(x, y, z, a); };

	unsigned Pack(bool Alpha = true)
	{
		return (Alpha ? ((unsigned)(a * 255.0f) << 24) : 0) + ((unsigned)(x * 255.0f) << 16) + ((unsigned)(y * 255.0f) << 8) + (unsigned)(z * 255.0f);
	}

	DerivedT WithAlpha(float alpha)
	{
		DerivedT col(static_cast<DerivedT &>(*this));
		col.a = alpha;
		return col;
	}
};

class ColorHSLA : public color4_base<ColorHSLA>
{
public:
	using color4_base::color4_base;
	ColorHSLA(){};

	constexpr static const float DARKEST_LGT = 0.5f;

	ColorHSLA UnclampLighting(float Darkest = DARKEST_LGT)
	{
		ColorHSLA col = *this;
		col.l = Darkest + col.l * (1.0f - Darkest);
		return col;
	}

	unsigned Pack(bool Alpha = true)
	{
		return color4_base::Pack(Alpha);
	}

	unsigned Pack(float Darkest, bool Alpha = false)
	{
		ColorHSLA col = *this;
		col.l = (l - Darkest) / (1 - Darkest);
		col.l = clamp(col.l, 0.0f, 1.0f);
		return col.Pack(Alpha);
	}
};

class ColorHSVA : public color4_base<ColorHSVA>
{
public:
	using color4_base::color4_base;
	ColorHSVA(){};
};

class ColorRGBA : public color4_base<ColorRGBA>
{
public:
	using color4_base::color4_base;
	ColorRGBA(){};
};

template<typename T, typename F>
T color_cast(const F &f) = delete;

template<>
inline ColorHSLA color_cast(const ColorRGBA &rgb)
{
	float Min = minimum(rgb.r, rgb.g, rgb.b);
	float Max = maximum(rgb.r, rgb.g, rgb.b);

	float c = Max - Min;
	float h = RgbToHue(rgb.r, rgb.g, rgb.b);
	float l = 0.5f * (Max + Min);
	float s = (Max != 0.0f && Min != 1.0f) ? (c / (1 - (absolute(2 * l - 1)))) : 0;

	return ColorHSLA(h, s, l, rgb.a);
}

template<>
inline ColorRGBA color_cast(const ColorHSLA &hsl)
{
	vec3 rgb = vec3(0, 0, 0);

	float h1 = hsl.h * 6;
	float c = (1 - absolute(2 * hsl.l - 1)) * hsl.s;
	float x = c * (1 - absolute(fmod(h1, 2) - 1));

	switch(round_truncate(h1))
	{
	case 0:
		rgb.r = c, rgb.g = x;
		break;
	case 1:
		rgb.r = x, rgb.g = c;
		break;
	case 2:
		rgb.g = c, rgb.b = x;
		break;
	case 3:
		rgb.g = x, rgb.b = c;
		break;
	case 4:
		rgb.r = x, rgb.b = c;
		break;
	case 5:
	case 6:
		rgb.r = c, rgb.b = x;
		break;
	}

	float m = hsl.l - (c / 2);
	return ColorRGBA(rgb.r + m, rgb.g + m, rgb.b + m, hsl.a);
}

template<>
inline ColorHSLA color_cast(const ColorHSVA &hsv)
{
	float l = hsv.v * (1 - hsv.s * 0.5f);
	return ColorHSLA(hsv.h, (l == 0.0f || l == 1.0f) ? 0 : (hsv.v - l) / minimum(l, 1 - l), l);
}

template<>
inline ColorHSVA color_cast(const ColorHSLA &hsl)
{
	float v = hsl.l + hsl.s * minimum(hsl.l, 1 - hsl.l);
	return ColorHSVA(hsl.h, v == 0.0f ? 0 : 2 - (2 * hsl.l / v), v);
}

template<>
inline ColorRGBA color_cast(const ColorHSVA &hsv)
{
	return color_cast<ColorRGBA>(color_cast<ColorHSLA>(hsv));
}

template<>
inline ColorHSVA color_cast(const ColorRGBA &rgb)
{
	return color_cast<ColorHSVA>(color_cast<ColorHSLA>(rgb));
}

template<typename T>
T color_scale(const T &col, float s)
{
	return T(col.x * s, col.y * s, col.z * s, col.a * s);
}

template<typename T>
T color_invert(const T &col)
{
	return T(1.0f - col.x, 1.0f - col.y, 1.0f - col.z, 1.0f - col.a);
}

/* * * * * * * * * * * * * *
 *                         *
 * teeworlds color system  *
 * TODO: replace by ddnet  *
 *                         *
 * * * * * * * * * * * * * */

/*
	Function: HueToRgb
		Converts Hue to RGB
*/
inline float HueToRgb(float v1, float v2, float h)
{
	if(h < 0.0f) h += 1;
	if(h > 1.0f) h -= 1;
	if((6.0f * h) < 1.0f) return v1 + (v2 - v1) * 6.0f * h;
	if((2.0f * h) < 1.0f) return v2;
	if((3.0f * h) < 2.0f) return v1 + (v2 - v1) * ((2.0f/3.0f) - h) * 6.0f;
	return v1;
}

/*
	Function: HslToRgb
		Converts HSL to RGB
*/
inline vec3 HslToRgb(vec3 HSL)
{
	if(HSL.s == 0.0f)
		return vec3(HSL.l, HSL.l, HSL.l);
	else
	{
		float v2 = HSL.l < 0.5f ? HSL.l * (1.0f + HSL.s) : (HSL.l+HSL.s) - (HSL.s*HSL.l);
		float v1 = 2.0f * HSL.l - v2;

		return vec3(HueToRgb(v1, v2, HSL.h + (1.0f/3.0f)), HueToRgb(v1, v2, HSL.h), HueToRgb(v1, v2, HSL.h - (1.0f/3.0f)));
	}
}

/*
	Function: HexToRgba
		Converts Hex to Rgba

	Remarks: Hex should be RGBA8
*/
inline vec4 HexToRgba(int hex)
{
	vec4 c;
	c.r = ((hex >> 24) & 0xFF) / 255.0f;
	c.g = ((hex >> 16) & 0xFF) / 255.0f;
	c.b = ((hex >> 8) & 0xFF) / 255.0f;
	c.a = (hex & 0xFF) / 255.0f;

	return c;
}

/*
	Function: HsvToRgb
		Converts Hsv to Rgb
*/
inline vec3 HsvToRgb(vec3 hsv)
{
	int h = int(hsv.x * 6.0f);
	float f = hsv.x * 6.0f - h;
	float p = hsv.z * (1.0f - hsv.y);
	float q = hsv.z * (1.0f - hsv.y * f);
	float t = hsv.z * (1.0f - hsv.y * (1.0f - f));

	vec3 rgb = vec3(0.0f, 0.0f, 0.0f);

	switch(h % 6)
	{
	case 0:
		rgb.r = hsv.z;
		rgb.g = t;
		rgb.b = p;
		break;

	case 1:
		rgb.r = q;
		rgb.g = hsv.z;
		rgb.b = p;
		break;

	case 2:
		rgb.r = p;
		rgb.g = hsv.z;
		rgb.b = t;
		break;

	case 3:
		rgb.r = p;
		rgb.g = q;
		rgb.b = hsv.z;
		break;

	case 4:
		rgb.r = t;
		rgb.g = p;
		rgb.b = hsv.z;
		break;

	case 5:
		rgb.r = hsv.z;
		rgb.g = p;
		rgb.b = q;
		break;
	}

	return rgb;
}

/*
	Function: RgbToHsv
		Converts Rgb to Hsv
*/
inline vec3 RgbToHsv(vec3 rgb)
{
	float h_min = minimum(minimum(rgb.r, rgb.g), rgb.b);
	float h_max = maximum(maximum(rgb.r, rgb.g), rgb.b);

	// hue
	float hue = 0.0f;

	if(h_max == h_min)
		hue = 0.0f;
	else if(h_max == rgb.r)
		hue = (rgb.g-rgb.b) / (h_max-h_min);
	else if(h_max == rgb.g)
		hue = 2.0f + (rgb.b-rgb.r) / (h_max-h_min);
	else
		hue = 4.0f + (rgb.r-rgb.g) / (h_max-h_min);

	hue /= 6.0f;

	if(hue < 0.0f)
		hue += 1.0f;

	// saturation
	float s = 0.0f;
	if(h_max != 0.0f)
		s = (h_max - h_min)/h_max;

	// value
	float v = h_max;

	return vec3(hue, s, v);
}

inline vec3 RgbToLab(vec3 rgb)
{
	vec3 adapt(0.950467f, 1, 1.088969f);
	vec3 xyz(
		0.412424f * rgb.r + 0.357579f * rgb.g + 0.180464f * rgb.b,
		0.212656f * rgb.r + 0.715158f * rgb.g + 0.0721856f * rgb.b,
		0.0193324f * rgb.r + 0.119193f * rgb.g + 0.950444f * rgb.b
	);

#define RGB_TO_LAB_H(VAL) ((VAL > 0.008856f) ? powf(VAL, 0.333333f) : (7.787f*VAL + 0.137931f))

	return vec3(
		116 * RGB_TO_LAB_H( xyz.y / adapt.y) - 16,
		500 * (RGB_TO_LAB_H(xyz.x / adapt.x) - RGB_TO_LAB_H(xyz.y / adapt.y)),
		200 * (RGB_TO_LAB_H(xyz.y / adapt.y) - RGB_TO_LAB_H(xyz.z / adapt.z))
	);

#undef RGB_TO_LAB_H
}

inline float LabDistance(vec3 labA, vec3 labB)
{
	float ld = labA.x - labB.x;
	float ad = labA.y - labB.y;
	float bd = labA.z - labB.z;
	return sqrtf(ld*ld + ad*ad + bd*bd);
}

#endif
