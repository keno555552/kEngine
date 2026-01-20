#pragma once

struct Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4 operator/= (const float& target);
	Vector4 operator+(const Vector4& target);
	Vector4 operator-(const Vector4& target);
	Vector4 operator=(const Vector4& target);

	bool operator==(const Vector4& target)const;
	bool operator!=(const Vector4& target)const;

	void ColorBy255();
	void ColorBy1();
};