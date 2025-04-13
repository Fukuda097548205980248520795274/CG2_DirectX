#include "Vector.h"

/// <summary>
/// 加算する
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>加算した値</returns>
Vector3 Add(const Vector3& v1, const Vector3& v2)
{
	// 加算した値
	Vector3 add;
	add.x = v1.x + v2.x;
	add.y = v1.y + v2.y;
	add.z = v1.z + v2.z;

	return add;
}

/// <summary>
/// 減算する
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>減算した値</returns>
Vector3 Subtract(const Vector3& v1, const Vector3& v2)
{
	// 減算した値
	Vector3 subtract;
	subtract.x = v1.x - v2.x;
	subtract.y = v1.y - v2.y;
	subtract.z = v1.z - v2.z;

	return subtract;
}

/// <summary>
/// 内積を求める
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>内積</returns>
float Dot(const Vector3& v1, const Vector3& v2)
{
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return dot;
}

/// <summary>
/// ベクトルの距離を求める
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>距離</returns>
float Length(const Vector3& v)
{
	float length = sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));
	return length;
}

/// <summary>
/// 正規化する
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>正規化したベクトル</returns>
Vector3 Normalize(const Vector3& v)
{
	// 距離
	float length = Length(v);

	// 正規化したベクトル
	Vector3 normalize = { 0.0f , 0.0f , 0.0f };

	if (length != 0.0f)
	{
		normalize.x = v.x / length;
		normalize.y = v.y / length;
		normalize.z = v.z / length;
	}

	return normalize;
}

/// <summary>
/// クロス積を求める
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>クロス積</returns>
Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	// クロス積
	Vector3 cross;

	cross.x = v1.y * v2.z - v1.z * v2.y;
	cross.y = v1.z * v2.x - v1.x * v2.z;
	cross.z = v1.x * v2.y - v1.y * v2.x;

	return cross;
}