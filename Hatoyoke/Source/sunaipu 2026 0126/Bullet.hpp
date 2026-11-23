#pragma once
#include <Siv3D.hpp>

class Bullet
{
public:
	void Init(double x_, double y_, double vx_, double vy_);
	void Update();
	void Draw();
	s3d::RectF GetRect() const;

	void Deactivate() { active = false; }
	bool IsActive() const { return active; }

	// 敵に命中済みか確認
	bool HasHitEnemy(size_t enemyIndex) const;

	// 命中した敵を記録
	void RegisterHit(size_t enemyIndex);

private:
	double x = 0;
	double y = 0;
	double vecX = 0;
	double vecY = 0;
	bool active = true;
	s3d::Texture texture;

	// 命中済みの敵番号を保存
	s3d::Array<size_t> hitEnemies;
};
