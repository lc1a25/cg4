#pragma once

#include "CollisionPrimitive.h"

///<summary>
///当たり判定ヘルパークラス
///</summary>
class Collision
{
public:
	///<summary>
	///球と平面の当たり判定
	///</summary>
	///<param name = "sphere">球</param>
	/// <param name = "plane">平面</param>
	/// <param name = "inter">交点</param>
	/// <returns>交差しているか否か</param>
	static bool CheckSphere2Plane(const Sphere& sphere, const Plane& plane, XMVECTOR* inter = nullptr);

	///<summary>
	///点と三角形の最近接点を求める
	///</summary>
	///<param name = "point">点</param>
	/// <param name = "triangle">三角形</param>
	/// <param name = "closest">最近接点 (出力用)</param>
	static void ClodestPtPoint2Triangle(const XMVECTOR& point, const Triangle& triangle, XMVECTOR* closest);

	///<summary>
	///球と法線付き三角形の当たり判定
	///</summary>
	///<param name = "sphere">球</param>
	/// <param name = "triangle">三角形</param>
	/// <param name = "inter">交点 (三角形上の最近接点)</param>
	/// <returns>交差しているか否か</param>
	static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, XMVECTOR* inter = nullptr);

	///<summary>
	///レイと平面の当たり判定
	///</summary>
	///<param name = "lay">レイ</param>
	/// <param name = "plane">平面</param>
	/// <param name = "inter">距離 (出力用)</param>
	/// <param name = "inter">交点 (出力用)</param>
	/// <returns>交差しているか否か</param>
	static bool CheckRay2Plane(const Ray& ray, const Plane& plane, float* distance = nullptr, XMVECTOR* inter = nullptr);

	///<summary>
	///レイと法線付き三角形の当たり判定
	///</summary>
	///<param name = "lay">レイ</param>
	/// <param name = "Triangle">三角形</param>
	/// <param name = "inter">距離 (出力用)</param>
	/// <param name = "inter">交点 (出力用)</param>
	/// <returns>交差しているか否か</param>
	static bool CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance = nullptr, XMVECTOR* inter = nullptr);

	///<summary>
	///レイと球の当たり判定
	///</summary>
	///<param name = "lay">レイ</param>
	/// <param name = "sphere">球</param>
	/// <param name = "inter">距離 (出力用)</param>
	/// <param name = "inter">交点 (出力用)</param>
	/// <returns>交差しているか否か</param>
	static bool CheckRay2Sphere(const Ray& ray, const Sphere& sphere, float* distance = nullptr, XMVECTOR* inter = nullptr);
};