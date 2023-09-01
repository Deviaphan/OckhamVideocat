#pragma once

#include <vector>

enum
{
	NUM_CHANNELS = 3,
	NUM_SAMPLES = 7
};	/// Число цветовых каналов на каждый пиксел

struct SampleByte
{
	union
	{
		unsigned __int64 b8;
		unsigned char s[8];
	};
};

struct SampleRGB
{
	SampleByte r, g, b;
};

struct RGB8
{
	unsigned char s[3];
};

class ImageData
{
public:
	ImageData()
		: _width( 0 )
		, _height( 0 )
		, _lastSample(0)
	{}

public:
	void Create( unsigned width, unsigned height );

	void AddPixel( int sample, int x, int y, unsigned char r, unsigned char g, unsigned char b );
	void GetResult( int x, int y, unsigned char & r, unsigned char & g, unsigned char & b ) const;

	void GetLine( int y, unsigned char * line ) const;

	void ProcessMean();
	void ProcessSigmaClipping();
	void ProcessNone();

private:

	int _width;		/// Ширина изображения
	int _height;	/// Высота изображения

	typedef std::vector<SampleRGB> SampledLine;
	std::vector< SampledLine > _data;

	std::vector<RGB8> _result;

	int _lastSample;
};
