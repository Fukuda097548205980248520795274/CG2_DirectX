#pragma once

// 2次元ベクトル
typedef struct Vector2
{
	float x;
	float y;
}Vector2;

// 3次元ベクトル
typedef struct Vector3
{
	float x;
	float y;
	float z;
}Vector3;

// 3+1次元ベクトル
typedef struct Vector4
{
	float x;
	float y;
	float z;
	float w;
}Vector4;

// 4x4行列
typedef struct Matrix4x4
{
	float m[4][4];
}Matrix4x4;