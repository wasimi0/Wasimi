#pragma once
#include <Siv3D.hpp>

class Player
{
public:
	void Init(double x_, double y_, s3d::String img_);
	void Update();
	void Draw();
	double x = 0, y = 0;
	double startX = 0, startY = 0;
	Texture texture;
	RectF GetRect() const;
	void Reset();
};
