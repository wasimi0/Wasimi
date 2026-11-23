#include "Player.hpp"

void Player::Init(double x_, double y_, s3d::String img_)
{
	x = x_;
	y = y_;

	startX = x_;
	startY = y_;

	texture = Texture{ img_ };
}

void Player::Update()
{
	// 前のフレームからの経過時間
	const double deltaTime = Scene::DeltaTime();

	// プレイヤーの移動速度（ピクセル/秒）
	const double moveSpeed = 400.0;

	// キーボードによる移動
	if (KeyRight.pressed() || KeyD.pressed())
		x += moveSpeed * deltaTime;

	if (KeyLeft.pressed() || KeyA.pressed())
		x -= moveSpeed * deltaTime;

	if (KeyUp.pressed() || KeyW.pressed())
		y -= moveSpeed * deltaTime;

	if (KeyDown.pressed() || KeyS.pressed())
		y += moveSpeed * deltaTime;

	// 画面外制限
	x = Clamp(x, 0.0, static_cast<double>(Scene::Width() - texture.width()));

	y = Clamp(y, 0.0, static_cast<double>(Scene::Height() - texture.height()));
}

void Player::Draw()
{
	texture.draw(x, y);
}

s3d::RectF Player::GetRect() const
{
	return texture.region(x, y);
}

void Player::Reset()
{
	x = startX;
	y = startY;
}
