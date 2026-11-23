#pragma once
#include <Siv3D.hpp>

class Enemy
{
public:
	void Init(double x_, double y_, double vx_, double vy_, s3d::String img_);
	void Update();
	void Draw();
	s3d::RectF GetRect() const;

	void Reset();
	void ResetPos();

	// 弾に当たった敵を5秒間消す
	void Deactivate();

	// 敵が出現中か確認する
	bool IsActive() const;

	// 大きな鳥など、スキルが効かない敵の設定
	void SetSkillImmune(bool immune) { skillImmune = immune; }
	bool IsSkillImmune() const { return skillImmune; }

private:
	double x = 0;
	double y = 0;
	double vecX = 0;
	double vecY = 0;

	double startX = 0;
	double startY = 0;

	bool skillImmune = false;

	s3d::Texture texture;

	// 敵の出現状態
	bool isActive = true;

	// 復活までの残り時間（秒）
	double respawnTimer = 0.0;

	// 復活までの時間
	static constexpr double RespawnTime = 5.0;
};
