#pragma once
#include <math.h>
#include "../../Struct.h"

/// <summary>
/// 加算する
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>加算した値</returns>
Vector3 Add(const Vector3& v1, const Vector3& v2);

/// <summary>
/// 減算する
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>減算した値</returns>
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

/// <summary>
/// 内積を求める
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>内積</returns>
float Dot(const Vector3& v1, const Vector3& v2);

/// <summary>
/// ベクトルの距離を求める
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>距離</returns>
float Length(const Vector3& v);

/// <summary>
/// 正規化する
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>正規化したベクトル</returns>
Vector3 Normalize(const Vector3& v);

/// <summary>
/// クロス積を求める
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>クロス積</returns>
Vector3 Cross(const Vector3& v1, const Vector3& v2);