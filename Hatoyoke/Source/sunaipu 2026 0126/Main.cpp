#include <Siv3D.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"

Array<Bullet> bullets;
Stopwatch skillCooldown{ StartImmediately::No };

namespace GameConfig
{
	// スキルのクールタイム（秒）
	constexpr double SkillCooldownSeconds = 10.0;

	// クリアまでの時間
	constexpr double ClearTimeSeconds = 30.0;

	// 背景スクロール速度
	constexpr double BackgroundScrollSpeed = 300.0;
}

enum class GameState
{
	HowToPlay,
	SelectDifficulty,
	Playing,
	GameOver,
	Clear
};

void InitEnemies(Array<Enemy>& enemy, int difficulty)
{
	int enemyCount = 6;
	double speedMul = 0.7;

	if (difficulty == 1)
	{
		enemyCount = 7;
		speedMul = 1.0;
	}
	else if (difficulty == 2)
	{
		enemyCount = 8;
		speedMul = 1.1;
	}

	enemy.clear();

	for (int i = 0; i < enemyCount; ++i)
	{
		enemy << Enemy();
	}

	enemy[0].Init(710, 100, -4 * speedMul, 2 * speedMul, U"example/hato.png");
	enemy[1].Init(720, 300, -5 * speedMul, -3 * speedMul, U"example/hato.png");
	enemy[2].Init(730, 500, -6 * speedMul, 1 * speedMul, U"example/hato.png");
	enemy[3].Init(740, 100, -4 * speedMul, -3 * speedMul, U"example/hato.png");
	enemy[4].Init(750, 300, -5 * speedMul, 1 * speedMul, U"example/hato.png");
	enemy[5].Init(710, 100, -6 * speedMul, -2 * speedMul, U"example/hato.png");

	if (enemyCount >= 7)
	{
		enemy[6].Init(720, 500, -1 * speedMul, 1 * speedMul, U"example/Oyahato.png");
		enemy[6].SetSkillImmune(true);
	}

	if (enemyCount >= 8)
	{
		enemy[7].Init(730, 500, -2 * speedMul, -1 * speedMul, U"example/Oyahato.png");
		enemy[7].SetSkillImmune(true);
	}
}

void StartGame(
	Array<Enemy>& enemy,
	Player& player,
	Stopwatch& timer,
	int difficulty)
{
	// 難易度に応じて敵を生成
	InitEnemies(enemy, difficulty);

	// プレイヤーを初期位置に戻す
	player.Reset();

	// ゲームの制限時間を開始
	timer.restart();

	// スキルのクールタイムを初期化
	skillCooldown.reset();

	// 前回の弾を削除
	bullets.clear();
}

void EndGame(
	GameState& gameState,
	GameState result,
	Stopwatch& timer)
{
	// ゲームの状態を変更
	gameState = result;

	// 制限時間を停止
	timer.pause();

	// スキルのクールタイムを停止
	skillCooldown.pause();
}

void ResetGame(
	Player& player,
	Array<Enemy>& enemy,
	Stopwatch& timer)
{
	// プレイヤーを初期位置に戻す
	player.Reset();

	// 敵と弾を削除
	enemy.clear();
	bullets.clear();

	// タイマーを初期化
	timer.reset();

	// スキルのクールタイムを初期化
	skillCooldown.reset();
}

void UpdateSkill(double px, double py)
{
	// Eキーが押されていなければ何もしない
	if (!KeyE.down())
	{
		return;
	}

	// クールタイム中ならスキルを使用できない
	if (skillCooldown.isRunning()
		&& skillCooldown.s() < GameConfig::SkillCooldownSeconds)
	{
		return;
	}

	// 弾を生成
	Bullet bullet;
	bullet.Init(px, py, 8, 0);
	bullets << bullet;

	// クールタイムを開始
	skillCooldown.restart();
}

void UpdateBullets(Array<Enemy>& enemy)
{
	for (auto& bullet : bullets)
	{
		bullet.Update();

		if (!bullet.IsActive())
		{
			continue;
		}

		for (size_t i = 0; i < enemy.size(); ++i)
		{
			// 消えている敵とスキル無効の敵は対象外
			if (!enemy[i].IsActive() || enemy[i].IsSkillImmune())
			{
				continue;
			}

			if (bullet.HasHitEnemy(i))
			{
				continue;
			}

			if (!bullet.GetRect().intersects(enemy[i].GetRect()))
			{
				continue;
			}

			bullet.RegisterHit(i);

			// 敵を5秒間消す
			enemy[i].Deactivate();
		}
	}

	bullets.remove_if([](const Bullet& bullet)
	{
		return !bullet.IsActive();
	});
}

void DrawBullets()
{
	for (auto& b : bullets)
	{
		b.Draw();
	}
}

double GetSkillCooldown()
{
	// まだ一度も使っていなければ使用可能
	if (!skillCooldown.isStarted())
	{
		return 0.0;
	}

	// 停止中も、保持されている経過時間から残りを計算する
	return Max(
		0.0,
		GameConfig::SkillCooldownSeconds - skillCooldown.s()
	);
}

void Main()
{
	int difficulty = 1;  // 0=Easy  1=Normal  2=Hard

	Window::Resize(800, 600);

	Font titleFont{ 36 };
	Font textFont{ 20 };
	Font gameOverFont{ 40 };

	Stopwatch timer{ StartImmediately::No };

	// プレイヤー
	Player player;
	player.Init(100, 300, U"example/Player.png");

	// 敵
	Array<Enemy> enemy;

	// リトライボタン
	Rect retryButton{ 350, 350, 100, 40 };

	GameState gameState = GameState::HowToPlay;
	double howToPlayElapsed = 0.0;
	bool howToPlayInputReady = false;

	Texture bg{ U"example/Haikei.png" };
	double bgX = 0;

	while (System::Update())
	{
		// ゲーム中のみ背景を動かす
		if (gameState == GameState::Playing)
		{
			const double deltaTime = Scene::DeltaTime();

			bgX -= GameConfig::BackgroundScrollSpeed * deltaTime;

			if (bgX <= -800.0)
			{
				bgX += 800.0;
			}
		}

		bg.draw(bgX, 0);
		bg.draw(bgX + 800, 0);

		// 操作説明画面
		if (gameState == GameState::HowToPlay)
		{
			// 説明文の後ろに半透明の黒いパネルを描画
			Rect{ 60, 70, 680, 440 }.draw(ColorF{ 0.0, 0.7 });

			titleFont(U"操作説明").drawAt(400, 120);
			textFont(U"WASD / 矢印キー：移動").drawAt(400, 220);
			textFont(U"敵に当たるとゲームオーバー").drawAt(400, 260);
			textFont(U"30秒耐えるとクリア").drawAt(400, 300);
			textFont(U"スキルはEで前方に放てます　クールタイムは10秒です").drawAt(400, 340);
			textFont(U"小さな鳥はスキルで5秒間消えます").drawAt(400, 380);
			textFont(U"大きな鳥にはスキルが効きません").drawAt(400, 415);
			Rect nextButton{ 280, 450, 240, 45 };
			nextButton.draw(Palette::Skyblue);
			textFont(U"難易度選択へ").drawAt(nextButton.center(), Palette::Black);

			// 起動・画面切り替え時のクリックを持ち越さない
			howToPlayElapsed += Scene::DeltaTime();
			if (!howToPlayInputReady)
			{
				if (howToPlayElapsed >= 0.5 && !MouseL.pressed())
				{
					howToPlayInputReady = true;
				}
			}
			else if (nextButton.leftClicked())
			{
				gameState = GameState::SelectDifficulty;
			}

			continue;
		}
		if (gameState == GameState::Playing)
		{
			player.Update();

			const RectF playerRect = player.GetRect();

			// プレイヤーの右端・上下中央から発射する
			UpdateSkill(
				playerRect.x + playerRect.w,
				playerRect.y + playerRect.h / 2.0
			);

			// 弾との当たり判定より先に、敵を移動する
			for (auto& e : enemy)
			{
				e.Update();
			}

			// 弾を移動し、移動後の敵との当たり判定を行う
			UpdateBullets(enemy);

			bool isGameOver = false;

			// 敵との衝突判定

			for (auto& e : enemy)
			{
				// 消えている敵は当たり判定を行わない
				if (!e.IsActive())
				{
					continue;
				}

				if (player.GetRect().intersects(e.GetRect()))
				{
					isGameOver = true;
					break;
				}
			}

			// ゲームオーバーを優先して判定
			if (isGameOver)
			{
				EndGame(gameState, GameState::GameOver, timer);
			}
			// 敵に当たっていない場合だけクリア判定
			else if (timer.s() >= GameConfig::ClearTimeSeconds)
			{
				EndGame(gameState, GameState::Clear, timer);
			}
		}

		// プレイ中・ゲームオーバー・クリア時に描画
		if (gameState == GameState::Playing ||
			gameState == GameState::GameOver ||
			gameState == GameState::Clear)
		{
			player.Draw();
			DrawBullets();

			for (auto& e : enemy)
			{
				e.Draw();
			}
		}

		// プレイ中と結果画面だけ、時間・スキルを表示
		if (gameState == GameState::Playing
			|| gameState == GameState::GameOver
			|| gameState == GameState::Clear)
		{
			const String difficultyLabel =
				(difficulty == 0) ? U"EASY" :
				(difficulty == 1) ? U"NORMAL" :
				                    U"HARD";

			// 左上に難易度を表示
			Rect{ 10, 5, 180, 40 }.draw(ColorF{ 0.0, 0.7 });
			textFont(U"難易度：{}"_fmt(difficultyLabel)).draw(20, 10);

			const int remainingTime = Max(
				0,
				static_cast<int>(Math::Ceil(GameConfig::ClearTimeSeconds - timer.s()))
			);

			Rect{ 510, 5, 280, 40 }.draw(ColorF{ 0.0, 0.7 });

			const double cooldown = GetSkillCooldown();

			const String skillText = (cooldown > 0.0)
				? Format(static_cast<int>(Math::Ceil(cooldown)))
				: U"READY";

			textFont(U"Time: {}  Skill: {}"_fmt(
				remainingTime,
				skillText
			)).draw(520, 10);
		}

		// 難易度選択
		if (gameState == GameState::SelectDifficulty)
		{
			Rect{ 230, 85, 340, 70 }.draw(ColorF{ 0.0, 0.7 });
			titleFont(U"難易度を選択").drawAt(400, 120);

			Rect easyBtn{ 300, 220, 200, 50 };
			Rect normalBtn{ 300, 300, 200, 50 };
			Rect hardBtn{ 300, 380, 200, 50 };

			easyBtn.draw(Palette::Lightgreen);
			normalBtn.draw(Palette::Skyblue);
			hardBtn.draw(Palette::Orange);

			textFont(U"EASY").drawAt(easyBtn.center(), Palette::Black);
			textFont(U"NORMAL").drawAt(normalBtn.center(), Palette::Black);
			textFont(U"HARD").drawAt(hardBtn.center(), Palette::Black);

			// 操作説明へ戻る
			Rect backButton{ 280, 480, 240, 45 };
			backButton.draw(ColorF{ 0.15, 0.85 });
			textFont(U"操作説明に戻る").drawAt(backButton.center(), Palette::White);

			if (backButton.leftClicked())
			{
				gameState = GameState::HowToPlay;
				howToPlayElapsed = 0.0;
				howToPlayInputReady = false;
				continue;
			}

			int selectedDifficulty = -1;

			if (easyBtn.leftClicked())
			{
				selectedDifficulty = 0;
			}
			else if (normalBtn.leftClicked())
			{
				selectedDifficulty = 1;
			}
			else if (hardBtn.leftClicked())
			{
				selectedDifficulty = 2;
			}

			if (selectedDifficulty != -1)
			{
				difficulty = selectedDifficulty;

				StartGame(enemy, player, timer, difficulty);

				// 背景を初期位置に戻す
				bgX = 0.0;

				// プレイ画面へ移行
				gameState = GameState::Playing;
			}

			continue;
		}

		if (gameState == GameState::GameOver ||
			gameState == GameState::Clear)
		{
			// 結果の文字とボタンを読みやすくする背景
			Rect{ 200, 210, 400, 200 }.draw(ColorF{ 0.0, 0.7 });

			// ゲームオーバーかクリアかで表示を変更
			if (gameState == GameState::GameOver)
			{
				gameOverFont(U"GAME OVER")
					.drawAt(400, 260, Palette::Red);

				textFont(U"生存時間：{:.1f} 秒"_fmt(timer.sF()))
					.drawAt(400, 310);
			}
			else
			{
				gameOverFont(U"CLEAR!")
					.drawAt(400, 260, Palette::Yellow);

				const String difficultyName =
					(difficulty == 0) ? U"EASY" :
					(difficulty == 1) ? U"NORMAL" :
					U"HARD";

				textFont(U"{} クリア！"_fmt(difficultyName))
					.drawAt(400, 310);
			}

			// リトライボタン
			retryButton.draw(Palette::Gray);

			textFont(U"Retry")
				.drawAt(retryButton.center(), Palette::White);

			// リトライ処理
			if (retryButton.leftClicked())
			{
				// ゲームをリセット
				ResetGame(player, enemy, timer);

				// 難易度選択画面に戻る
				gameState = GameState::SelectDifficulty;
			}
		}
	}
}
