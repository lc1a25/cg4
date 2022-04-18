#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
//#include <DirectXMath.h>
#include <vector>
#include <string>
#include <math.h>
#include <d3dcompiler.h>
#include <DirectXTex.h>
#include "Input.h"
#include <wrl.h> // 7-1
#include <d3dx12.h>//7-3
#include<xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include<fstream>

#include "Object3d.h"

#include "Win.h"
#include "DirectXCommon.h"
#include "CollisionPrimitive.h"
#include "Collision.h"

#include "Sprite.h"
#include "SpriteCommon.h"
#include <algorithm>

#include "DebugText.h"

#include "Audio.h"

#include "fbxsdk.h"
#include "FbxLoader.h"



#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "DirectXTex.lib")

using namespace Microsoft::WRL;

using namespace DirectX;


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{


// WindowsAPI初期化処理
	
	
	Win* win = nullptr;
	Input* input = nullptr;
	DirectXCommon* dxcommon = nullptr;
	Audio* audio = nullptr;

	win = new Win();
	win->WinCreate();


	dxcommon = new DirectXCommon();
	dxcommon->Init(win);

	SpriteCommon* spriteCommon = new SpriteCommon;
	spriteCommon->Init(dxcommon->GetDev(), dxcommon->GetCmdlist(), win->window_width, win->window_height);

	DebugText* debugtext_minute = nullptr;
	debugtext_minute = new DebugText();

	Object3d::StaticInitialize(dxcommon->GetDev(), win->window_width, win->window_height);

	audio = new Audio();
	audio->Init();

	input = new Input();
	input->Initialize(win);

	FbxManager* fbxManager = FbxManager::Create();

	FbxLoader::GetInstance()->Init(dxcommon->GetDev());

	// DirectX初期化処理　ここまで

	//描画初期化処理　ここから
	spriteCommon->LoadTexture(0, L"Resource/GE3_2.png");

	Sprite *sprite = Sprite::Create(spriteCommon,0);

	const int debugTextTexNumber3 = 4;

	spriteCommon->LoadTexture(debugTextTexNumber3, L"Resource/ASC_White.png");

	debugtext_minute->debugTextInit(spriteCommon, debugTextTexNumber3);

	DebugText* debugtext_minute2 = nullptr;
	debugtext_minute2 = new DebugText();

	const int debugTextTexNumber4 = 5;

	spriteCommon->LoadTexture(debugTextTexNumber4, L"Resource/ASC_White.png");
	debugtext_minute2->debugTextInit(spriteCommon, debugTextTexNumber4);


	//音声読み込み
	audio->LoadWave("powerdown07.wav");

	


#pragma region model
	

	Object3d* object3d_camera = nullptr;

	Model* model1 = Model::LoadFromOBJ("car1");
	Model* model3 = Model::LoadFromOBJ("home3-0");
	Model* model4 = Model::LoadFromOBJ("ene-0");
	Model* model5 = Model::LoadFromOBJ("road");
	Model* model7 = Model::LoadFromOBJ("home_white");
	Model* model8 = Model::LoadFromOBJ("home_long");
	Model* model9 = Model::LoadFromOBJ("glass");



	//自機
	Object3d* object3d_player = object3d_player->Create();

	object3d_player->SetModel(model1);

	object3d_player->SetPosition({ 0,-10,-20 });
	object3d_player->rotation.y = -90;
	//道
	Object3d* object3d_roadCenter = object3d_roadCenter->Create();
	Object3d* object3d_roadCenter2 = object3d_roadCenter2->Create();
	Object3d* object3d_roadCenter3 = object3d_roadCenter3->Create();

	object3d_roadCenter->SetModel(model5);
	object3d_roadCenter2->SetModel(model5);
	object3d_roadCenter3->SetModel(model5);

	object3d_roadCenter->SetPosition({ 0,-30,0 });
	object3d_roadCenter->scale.z = 200;
	object3d_roadCenter->scale.x = 20;

	object3d_roadCenter2->SetPosition({ 0,-30,+800 });//z 400
	object3d_roadCenter2->scale.z = 200;
	object3d_roadCenter2->scale.x = 20;

	object3d_roadCenter3->SetPosition({ 0,-30,+1600 });//z 400
	object3d_roadCenter3->scale.z = 200;
	object3d_roadCenter3->scale.x = 20;
	//家
	Object3d* object3d_home = object3d_home->Create();
	Object3d* object3d_homeWhite = object3d_homeWhite->Create();
	Object3d* object3d_homeLong = object3d_homeLong->Create();

	object3d_home->SetModel(model3);
	object3d_homeWhite->SetModel(model7);
	object3d_homeLong->SetModel(model8);

	object3d_home->SetPosition({ -70,-10,0 });
	object3d_home->rotation.y = 270;
	object3d_homeWhite->SetPosition({ 90,-30,30 });
	object3d_homeWhite->rotation.y = 90;
	object3d_homeLong->SetPosition({ -90,-30,60 });
	object3d_homeLong->rotation.y = 270;

	//敵
	Object3d* object3d_eneCenter = object3d_eneCenter->Create();

	object3d_eneCenter->SetModel(model4);

	object3d_eneCenter->SetPosition({ 0,-15,+200 });

	Object3d* object3d_eneLeft = object3d_eneLeft->Create();

	object3d_eneLeft->SetModel(model4);

	object3d_eneLeft->SetPosition({ 0,-15,+50 });

	Object3d* object3d_eneRight = object3d_eneRight->Create();

	object3d_eneRight->SetModel(model4);

	object3d_eneRight->SetPosition({ 35,-15,+400 });

	//背景
	Object3d* object3d_glass = object3d_glass->Create();
	Object3d* object3d_glass2 = object3d_glass2->Create();

	object3d_glass->SetModel(model9);
	object3d_glass2->SetModel(model9);

	object3d_glass->SetPosition({ -50,-30,0 });
	object3d_glass->scale.x = 200;
	object3d_glass->scale.z = 200;

	object3d_glass2->SetPosition({ -50,-30,780 });
	object3d_glass2->scale.x = 200;
	object3d_glass2->scale.z = 200;

	//当たり判定　球
	Sphere sphere;

	//当たり判定　三角形
	Triangle triangle_ene2;

	//球の初期値を設定
	sphere.center = XMVectorSet(0, -10, -5, 1);//中心点座標
	sphere.radius = 10.0f;//半径


	FbxLoader::GetInstance()->LoadModelFile("cube");

#pragma endregion
	//描画初期化処理　ここまで



#pragma region シェーダーの色,初期化


	float xmfloat4_r = 1.0f;
	float xmfloat4_b = 1.0f;
	float xmfloat4_g = 1.0f;
	float xmfloat4_a = 1.0f;

	float ene_x2 = object3d_eneLeft->position.x;
	float ene_y2 = object3d_eneLeft->position.y;
	float ene_z2 = object3d_eneLeft->position.z;


	char moji[64];
	char moji2[64];
	char moji3[64];

	int secound_x = 150;
	int secound_y = 0;
	int secound2_x = 100;
	int secound2_y = 0;


	int timer = 60;
	int secound = 0;
	int secound2 = 0;
	int minute = 0;

	int centerCount = 0;
	int leftCount = 0;
	int rightCount = 0;

	int gameScene = 1;

	sprite->SetPosition({ 1040.0f,130.0f,0.0f });
	sprite->SetTexsize({440.0f,250.0f });

	sprite->TransVertexBuffer();
	
#pragma endregion


	while (true)  // ゲームループ
	{
		//// メッセージがある？
		

		//// ?ボタンで終了メッセージが来たらゲームループを抜ける

		if (win->ProcessMessage()) {
			break;
		}

		// DirectX毎フレーム処理　ここから

		//キーボード情報の取得開始
		//全キーの入力状態を取得する
		input->Update();

		sprite->Update();

		
		if (gameScene == 1)
		{

			XMVECTOR moveZ = XMVectorSet(0, 0, 1.0f, 0);//z speed

			XMVECTOR moveX2 = XMVectorSet(1.0f, 0, 0, 0);//debug

			if (input->isKey(DIK_W))
			{
				if (object3d_player->position.z >= 120.0f)
				{
					object3d_player->position.z -= 1.0f;
					sphere.center -= moveZ;
				}
				else
				{
					object3d_player->position.z += 1.0f;
					sphere.center += moveZ;
				}

			}
			if (input->isKey(DIK_S))
			{
				if (object3d_player->position.z <= -20.0f)
				{
					object3d_player->position.z += 1.0f;
					sphere.center += moveZ;
				}
				else
				{
					object3d_player->position.z -= 1.0f;
					sphere.center -= moveZ;
				}
				
			}
			if (input->isKey(DIK_A))
			{
				if (object3d_player->position.x <= -30.0f)
				{
					object3d_player->position.x += 1.0f;
					sphere.center += moveX2;
				}
				else
				{
					object3d_player->position.x -= 1.0f;
					sphere.center -= moveX2;
				}
				
			}
			if (input->isKey(DIK_D))
			{
				if (object3d_player->position.x >= +30.0f)
				{
					object3d_player->position.x -= 1.0f;
					sphere.center -= moveX2;
				}
				else
				{
					object3d_player->position.x += 1.0f;
					sphere.center += moveX2;
				}
			}


			//三角形の初期値を設定
			triangle_ene2.p0 = XMVectorSet(ene_x2 - 1.0f, ene_y2, ene_z2, 1);//左手前
			triangle_ene2.p1 = XMVectorSet(ene_x2, ene_y2, ene_z2 + 5.0f, 1);//左奥
			triangle_ene2.p2 = XMVectorSet(ene_x2 + 1.0f, ene_y2, ene_z2, 1);//右手前
			triangle_ene2.normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);//上向き

			bool hit_left = Collision::CheckSphere2Triangle(sphere, triangle_ene2);


			sprintf_s(moji, "%d", secound);
			sprintf_s(moji2, "%d", secound2);
			sprintf_s(moji3, "%d", minute);
		


	//当たり判定



	

			if (hit_left)
			{
				audio->PlayWave("powerdown07.wav");
			}

//秒数カウント
			timer--;
			if (timer <= 0)
			{
				secound++;
				if (secound >= 10)
				{
					secound2++;
					secound = 0;

				}
				if (secound2 >= 6)
				{
					minute++;
					secound2 = 0;
				}
				timer = 60;

			}
			debugtext_minute->Print( moji, secound_x, secound_y, 1.0f);
			debugtext_minute2->Print(moji2, secound2_x, secound2_y, 1.0f);

		}


		object3d_player->Update();
		object3d_roadCenter->Update();
		object3d_roadCenter2->Update();
		object3d_roadCenter3->Update();

		object3d_eneCenter->Update();
		object3d_eneLeft->Update();
		object3d_eneRight->Update();

		object3d_glass->Update();
		object3d_glass2->Update();

		object3d_home->Update();
		object3d_homeWhite->Update();
		object3d_homeLong->Update();
	
		dxcommon->BeginDraw();

		// ４．描画コマンドここから


		Object3d::PreDraw(dxcommon->GetCmdlist());
		
		object3d_player->Draw();
		object3d_roadCenter->Draw();
		object3d_roadCenter2->Draw();
		object3d_roadCenter3->Draw();

		object3d_eneCenter->Draw();
		object3d_eneLeft->Draw();
		object3d_eneRight->Draw();

		object3d_glass->Draw();
		object3d_glass2->Draw();

		object3d_home->Draw();
		object3d_homeWhite->Draw();
		object3d_homeLong->Draw();

		Object3d::PostDraw();

		////スプライト共通コマンド
		spriteCommon->PreDraw();

		////スプライト描画
		sprite->Draw();


		debugtext_minute->DrawAll();
		debugtext_minute2->DrawAll();
		// ４．描画コマンドここまで
		// 
		// ５．リソースバリアを戻す

		dxcommon->EndDraw();
		
	}
	// ウィンドウクラスを登録解除
	win->WinFinalize();

	delete object3d_player;
	delete object3d_roadCenter;
	delete object3d_roadCenter2;
	delete object3d_roadCenter3;
	delete object3d_home;
	delete object3d_homeWhite;
	delete object3d_homeLong;
	delete object3d_eneCenter;
	delete object3d_eneLeft;
	delete object3d_eneRight;
	delete object3d_glass;
	delete object3d_glass2;

	//Audio解放
	audio->Finalize();
	delete audio;

	//デバッグテキスト解放
	debugtext_minute->Finalize();
	delete debugtext_minute;


	delete sprite;
	delete spriteCommon;
	FbxLoader::GetInstance()->Finalize();
	delete dxcommon;
	delete input;
	
	
	
	return 0;
}