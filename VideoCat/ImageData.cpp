#include "pch.h"

#include "ImageData.h"
#include <algorithm>

inline unsigned sqr( const unsigned value )
{
	return value * value;
}

std::vector<unsigned char> g_Sqrt;


void ImageData::AddPixel( int sample, int x, int y, unsigned char r, unsigned char g, unsigned char b )
{
	SampledLine & line = _data[y];
	auto & rgb = line[x];
	rgb.r.s[sample] = r;
	rgb.g.s[sample] = g;
	rgb.b.s[sample] = b;

	_lastSample = sample;
}

void ImageData::GetResult( int x, int y, unsigned char & r, unsigned char & g, unsigned char & b ) const
{
	const auto & rgb = _result[ y*_width + x];

	r = rgb.s[0];
	g = rgb.s[1];
	b = rgb.s[2];
}

void ImageData::GetLine( int y, unsigned char * line ) const
{
	auto ptr =  &(_result[y*_width]);

	memcpy( line, ptr, _width*3);
}

void ImageData::Create( unsigned width, unsigned height )
{
	_width = width;
	_height = height;

	_data.resize( _height );
	for( SampledLine & line : _data )
	{
		line.resize( _width );
	}
}

void ImageData::ProcessMean()
{
	if( g_Sqrt.empty() )
	{
		g_Sqrt.resize( 65536);
		for( int i = 0; i < 65536; ++i )
		{
			g_Sqrt[i] = (unsigned char)( sqrt(i) );
		}
	}

	if( _result.empty() )
	{
		_result.resize( _height * _width );
	}

	for( int y = 0; y < _height; ++y )
	{
		const SampledLine & line = _data[y];

		const int resultY = y * _width;

#pragma omp parallel for
		for( int x = 0; x < _width; ++x )
		{
			unsigned valueR = 0;
			unsigned valueG = 0;
			unsigned valueB = 0;

			const auto & rgb = line[x];

			for( int sample = 0; sample < NUM_SAMPLES; ++sample )
			{
				valueR += sqr( rgb.r.s[sample] );
				valueG += sqr( rgb.g.s[sample] );
				valueB += sqr( rgb.b.s[sample] );
			}

			auto & res = _result[resultY + x];

			res.s[0] = g_Sqrt[ valueR / NUM_SAMPLES ];
			res.s[1] = g_Sqrt[ valueG / NUM_SAMPLES ];
			res.s[2] = g_Sqrt[ valueB / NUM_SAMPLES ];
		}
	}
}

void Sort( unsigned char* arr )
{
	for( int i = 1; i < NUM_SAMPLES; ++i )
	{
		for( int j = i; j > 0 && arr[j - 1] > arr[j]; --j )
		{
			unsigned char tmp = arr[j - 1];
			arr[j - 1] = arr[j];
			arr[j] = tmp;
		}
	}
}

void ImageData::ProcessSigmaClipping()
{
	if( g_Sqrt.empty() )
	{
		g_Sqrt.resize( 65536);
		for( int i = 0; i < 65536; ++i )
		{
			g_Sqrt[i] = (unsigned char)( sqrt(i) );
		}

	}

	if( _result.empty() )
	{
		_result.resize( _height * _width );
	}

	for( int y = 0; y < _height; ++y )
	{
		const SampledLine & line = _data[y];

		const int resultY = y * _width;

#pragma omp parallel for
		for( int x = 0; x < _width; ++x )
		{
			constexpr short diff = 30;

			SampleRGB pix = { line[x] };

			//std::sort( pix.r.s, &pix.r.s[0] + NUM_SAMPLES );
			//std::sort( pix.g.s, &pix.g.s[0] + NUM_SAMPLES  );
			//std::sort( pix.b.s, &pix.b.s[0] + NUM_SAMPLES  );
			Sort(  pix.r.s );
			Sort(  pix.g.s );
			Sort(  pix.b.s );

			const short midR = pix.r.s[3];
			const short midG = pix.g.s[3];
			const short midB = pix.b.s[3];

			const short minR = midR - diff;
			const short maxR = midR + diff;

			const short minG = midG - diff;
			const short maxG = midG + diff;

			const short minB = midB - diff;
			const short maxB = midB + diff;

			unsigned valueR = 0;
			unsigned valueG = 0;
			unsigned valueB = 0;

			unsigned numR = 0;
			unsigned numG = 0;
			unsigned numB = 0;

			for( int sample = 0; sample < NUM_SAMPLES; ++sample )
			{
				const short red = pix.r.s[sample];
				if( minR <= red && red <= maxR )
				{
					valueR += sqr(red);
					++numR;
				}

				const unsigned char green = pix.g.s[sample];
				if( minG <= green && green <= maxG )
				{
					valueG += sqr(green);
					++numG;
				}

				const unsigned char blue = pix.b.s[sample];
				if( minB <= blue && blue <= maxB )
				{
					valueB += sqr(blue);
					++numB;
				}
			}

			auto & res = _result[resultY + x];

			res.s[0] = g_Sqrt[ valueR / numR ];
			res.s[1] = g_Sqrt[ valueG / numG ];
			res.s[2] = g_Sqrt[ valueB / numB ];
		}
	}
}

void ImageData::ProcessNone()
{	
	if( _result.empty() )
	{
		_result.resize( _height * _width );
	}


#pragma omp parallel for
	for( int y = 0; y < _height; ++y )
	{
		const SampledLine & line = _data[y];

		const int resultY = y * _width;

		for( int x = 0; x < _width; ++x )
		{
			const auto & rgb = line[x];

			auto & res = _result[resultY+x];

			res.s[0] = rgb.r.s[_lastSample];
			res.s[1] = rgb.g.s[_lastSample];
			res.s[2] = rgb.b.s[_lastSample];
		}
	}
}
