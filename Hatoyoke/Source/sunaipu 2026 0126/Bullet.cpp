#include "Bullet.hpp"

void Bullet::Init(double x_, double y_, double vx_, double vy_)
{
	x = x_;
	y = y_;
	vecX = vx_;
	vecY = vy_;
	active = true;
	hitEnemies.clear();
	texture = Texture{ U"example/Bullet.png" };
}

void Bullet::Update()
{
	if (!active) return;

	const double deltaTime = Scene::DeltaTime();

	// 弾の移動速度をFPSに依存しないようにする
	x += vecX * deltaTime * 90.0;
	y += vecY * deltaTime * 90.0;

	// 弾は中心座標で描画しているため、画像の半分の大きさを使う
	const double halfWidth = texture.width() / 2.0;
	const double halfHeight = texture.height() / 2.0;

	// 画像全体が画面外に出たら削除対象にする
	if (x + halfWidth < 0.0
		|| x - halfWidth > Scene::Width()
		|| y + halfHeight < 0.0
		|| y - halfHeight > Scene::Height())
	{
		active = false;
	}
}

void Bullet::Draw()
{
	if (active)
	{
		texture.drawAt(x, y);
	}
}

s3d::RectF Bullet::GetRect() const
{
	return s3d::RectF(
		x - texture.width() / 2.0,
		y - texture.height() / 2.0,
		texture.width(),
		texture.height()
	);
}

bool Bullet::HasHitEnemy(size_t enemyIndex) const
{
	for (const auto& index : hitEnemies)
	{
		if (index == enemyIndex)
		{
			return true;
		}
	}

	return false;
}

void Bullet::RegisterHit(size_t enemyIndex)
{
	if (!HasHitEnemy(enemyIndex))
	{
		hitEnemies << enemyIndex;
	}
}
