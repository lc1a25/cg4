#include "Audio.h"

void Audio::LoadWave(const std::string& filename)
{

	if (soundDatas_.find(filename) != soundDatas_.end())
	{
		//重複読み込みなので、何もせずぬける
		return;
	}

	//ディレクトリパスとファイル名を連結　フルパスになる
	std::string fullpath = directoryPath_ + filename;

	HRESULT result;

	//File open
	std::ifstream file;

	file.open(fullpath, std::ios_base::binary);

	assert(file.is_open());

	//wavData Load
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}

	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}

	FormatChunk format = {};

	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}

	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);



	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	if (strncmp(data.id, "JUNK", 4) == 0)
	{
		file.seekg(data.size, std::ios_base::cur);

		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "LIST", 4) == 0)
	{
		file.seekg(data.size, std::ios_base::cur);

		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	file.close();
	//File close

	//return
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	//サウンドデータを連想配列に送る
	soundDatas_.insert(std::make_pair(filename, soundData));
}

void Audio::Init(const std::string& directoryPath)
{
	directoryPath_ = directoryPath;

	HRESULT result;

	IXAudio2MasteringVoice* masterVoice;

	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	//マスターボイスを作成
	result = xAudio2_->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

void Audio::Unload(SoundData* soundData)
{
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audio::PlayWave(const std::string& filename)
{
	HRESULT result;


	soundDatas_.find(filename);
	std::map<std::string, SoundData>::iterator it = soundDatas_.find(filename);
	//未読み込みの検出
	assert(it != soundDatas_.end());
	//サウンドデータの参照を取得
	SoundData& soundData = it->second;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	// buf.AudioBytes = size;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void Audio::PlayWaveLoop(const std::string& filename)
{
	HRESULT result;

	soundDatas_.find(filename);
	std::map<std::string, SoundData>::iterator it = soundDatas_.find(filename);
	//未読み込みの検出
	assert(it != soundDatas_.end());
	//サウンドデータの参照を取得
	SoundData& soundData = it->second;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// buf.AudioBytes = size;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void Audio::Finalize()
{
	xAudio2_.Reset();

	//音声データの解放
	std::map<std::string, SoundData>::iterator it = soundDatas_.begin();

	for ( ; it != soundDatas_.end();it++ )
	{
		Unload(&it->second);
	}
	soundDatas_.clear();
}
