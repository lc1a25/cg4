#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <map>
#include<fstream>
#include <cassert>
#include <wrl.h>
class Audio
{
private:
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	struct ChunkHeader
	{
		char id[4];
		int32_t size;
	};

	struct RiffHeader
	{
		ChunkHeader chunk;
		char type[4];

	};

	struct FormatChunk
	{
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};

	struct SoundData
	{
		WAVEFORMATEX wfex;

		BYTE* pBuffer;

		unsigned int bufferSize;
	};




private:

	ComPtr<IXAudio2> xAudio2_;

	//サウンドデータの連想配列    [添え字が好きな名前]
	std::map < std::string, SoundData > soundDatas_;

	//サウンド格納ディレクトリ
	std::string directoryPath_;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Init(const std::string& directoryPath_ = "Resource/");

	/// <summary>
	/// WAV音声読み込み
	/// </summary>
	/// <param name="filename">WAVファイル名</param>
	void LoadWave(const std::string& filename);


	/// <summary>
	/// 音声データの解放
	/// </summary>
	/// <param name="soundData">サウンドデータ</param>
	void Unload(SoundData* soundData);

	/// <summary>
	/// 音声再生
	/// </summary>
	/// <param name="filename"></param>
	void PlayWave(const std::string& filename);
	
	void PlayWaveLoop(const std::string& filename);

	void Finalize();


};

