#include "Enemy.hpp"

void Enemy::Init(double x_, double y_, double vx_, double vy_, s3d::String img_)
{
	x = x_;
	y = y_;
	vecX = vx_;
	vecY = vy_;
	texture = Texture{ img_ };

	startX = x_;
	startY = y_;

	// 出現状態とスキル耐性を初期化
	skillImmune = false;
	isActive = true;
	respawnTimer = 0.0;
}

void Enemy::Update()
{
	const double deltaTime = Scene::DeltaTime();

	// 消えている間は復活時間だけ更新
	if (!isActive)
	{
		respawnTimer -= deltaTime;

		if (respawnTimer <= 0.0)
		{
			ResetPos();
			isActive = true;
			respawnTimer = 0.0;
		}

		return;
	}

	// 通常の移動処理
	x += vecX * deltaTime * 100.0;
	y += vecY * deltaTime * 100.0;

	// 左右：画像全体が画面外に出たら、反対側へ移動
	if (x < -static_cast<double>(texture.width()))
	{
		x = Scene::Width();
	}
	else if (x > Scene::Width())
	{
		x = -static_cast<double>(texture.width());
	}

	// 上下：画像全体が画面外に出たら、反対側へ移動
	if (y < -static_cast<double>(texture.height()))
	{
		y = Scene::Height();
	}
	else if (y > Scene::Height())
	{
		y = -static_cast<double>(texture.height());
	}
}

void Enemy::Draw()
{
	if (isActive)
	{
		texture.draw(x, y);
	}
}

s3d::RectF Enemy::GetRect() const
{
	return s3d::RectF(x, y, texture.size());
}

void Enemy::Reset()
{
	x = startX;
	y = startY;
	isActive = true;
	respawnTimer = 0.0;
}

void Enemy::ResetPos()
{
	x = startX;
	y = startY;
}

// 敵を5秒間消す
void Enemy::Deactivate()
{
	if (skillImmune)
	{
		return;
	}

	isActive = false;
	respawnTimer = RespawnTime;
}

// 敵が出現中か確認
bool Enemy::IsActive() const
{
	return isActive;
}
