#include "Perlin.h"

#include <Engine/Globals.h>
#include <Engine/Random/RandomMgr.h>
#include <Engine/Profiler.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <assert.h>
#include <cmath>

Perlin::Perlin(unsigned sizeX, unsigned sizeY, unsigned cellSize, unsigned octaves) :
	SizeX(sizeX), SizeY(sizeY), CellSize(cellSize), GridSizeX(sizeX / cellSize + 1), GridSizeY(sizeY / cellSize + 1), Octaves(octaves)
{
}


bool Perlin::Generate()
{
	if (SizeX == 0 || SizeY == 0 || CellSize == 0 || CellSize > SizeX || CellSize > SizeY)
	{
		return false;
	}

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_PURPLE, "Perlin Generation");

	Reset();

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_CYAN, "Perlin Generation : Populate Vectors");

	PopulateVectors();

	PROFILER_EVENT_END();
	PROFILER_EVENT_BEGIN(PROFILER_COLOR_PINK, "Perlin Generation : Generate Pixels");

	GeneratePixels();

	PROFILER_EVENT_END();
	PROFILER_EVENT_END();

	return true;
}

float Perlin::GetValue(unsigned x, unsigned y) const
{
	if (x >= SizeX || y >= SizeY)
	{
		return 0;
	}

	return m_vPixels[x][y];
}

void Perlin::GenerateTexture(sf::Texture& texture, bool blackAndWhite) const
{
	if (m_vPixels.size() == 0)
	{
		return;
	}

	size_t dataSize = SizeX * SizeY;
	unsigned char* data = (unsigned char*)calloc(dataSize * 4, sizeof(unsigned char));
	if (!data)
	{
		return;
	}

	sf::Image img({ SizeX, SizeY }, data);

	unsigned int dataIndex = 0;
	for (unsigned x = 0; x < m_vPixels.size(); ++x)
	{
		for (unsigned y = 0; y < m_vPixels[x].size(); ++y)
		{
			sf::Color color;
			float p = m_vPixels[x][y];
			assert(p == p);
			if (blackAndWhite)
			{
				float p1 = (p + 1.0) / 2.0;
				float l = std::lerp(0.0, 255, p1);
				unsigned u = std::floor(l);

				assert(u >= 0);
				assert(u <= 255);

				unsigned char c = static_cast<unsigned char>(u);
				color = sf::Color(c, c, c, 255);
			}
			else
			{
				if (p < -0.5f)
				{
					color = sf::Color(0, 0, 75);
				}
				else if (p < -0.1f)
				{
					color = sf::Color(0, 173, 252);
				}
				else if (p <= 0.f)
				{
					color = sf::Color(140, 255, 255);
				}
				else if (p <= 0.1f)
				{
					color = sf::Color(255, 255, 122);
				}
				else if (p <= 0.4f)
				{
					color = sf::Color(50, 255, 50);
				}
				else if (p <= 0.7f)
				{
					color = sf::Color(88, 41, 0);
				}
				else
				{
					color = sf::Color(255, 255, 255);
				}
			}

			img.setPixel({ x, y }, color);
		}
	}

	if (!texture.loadFromImage(img))
	{
		assert(false);
	}

	free(data);
}

void Perlin::PopulateVectors()
{
	unsigned randomInstanceId = gData.RandomMgr->CreateInstance();
	RandomInstance* randomInstance = gData.RandomMgr->GetInstance(randomInstanceId);
	assert(randomInstance);
	randomInstance->SetSeed(RandomMgr::GenerateRandomSeed());

	std::uniform_real_distribution<> dist(-1, 1);

	m_vVectors.resize(GridSizeX);
	for (unsigned x = 0; x < GridSizeX; ++x)
	{
		m_vVectors[x].resize(GridSizeY);

		for (unsigned y = 0; y < GridSizeY; ++y)
		{
			sf::Vector2f& value = m_vVectors[x][y];

			value.x = randomInstance->RandDouble(-1, 1);
			value.y = randomInstance->RandDouble(-1, 1);

			// Normalizaed
			float norm = sqrt(value.x * value.x + value.y * value.y);
			value.x /= norm;
			value.y /= norm;
		}
	}

	gData.RandomMgr->DestroyInstance(randomInstanceId);
}

void Perlin::GeneratePixels()
{
	m_vPixels.resize(SizeX);
	for (unsigned x = 0; x < SizeX; ++x)
	{
		m_vPixels[x].resize(SizeY);
		for (unsigned y = 0; y < SizeY; ++y)
		{
			float freq = 1.f;
			float amp = 1.f;

			float value = 0.f;
			for (unsigned i = 0; i < Octaves; ++i)
			{
				value += GetPerlinValue(x * freq / GridSizeX, y * freq / GridSizeY) * amp;

				freq *= 2.f;
				amp /= 2.f;
			}

			value = std::clamp(value, -1.f, 1.f);
			m_vPixels[x][y] = value;
		}
	}
}

float Perlin::GetPerlinValue(float x, float y) const
{
	unsigned x0 = x;
	unsigned x1 = x0 + 1;
	unsigned y0 = y;
	unsigned y1 = y0 + 1;

	float sx = x - (float)x0;
	float sy = y - (float)y0;

	float nx0 = DotGridGradient(x0, y0, x, y);
	float nx1 = DotGridGradient(x1, y0, x, y);
	float ix0 = Interpolate(nx0, nx1, sx);

	float ny0 = DotGridGradient(x0, y1, x, y);
	float ny1 = DotGridGradient(x1, y1, x, y);
	float ix1 = Interpolate(ny0, ny1, sx);

	float value = Interpolate(ix0, ix1, sy);
	return value;
}

void Perlin::Reset()
{
	m_vVectors.clear();
	m_vPixels.clear();
}

float Perlin::DotGridGradient(unsigned ix, unsigned iy, float x, float y) const
{
	float dx = x - ix;
	float dy = y - iy;

	unsigned _x = ix % GridSizeX;
	unsigned _y = iy % GridSizeY;

	return dx * m_vVectors[_x][_y].x + dy * m_vVectors[_x][_y].y;
}

float SmoothStep(float w)
{
	if (w <= 0.0)
	{
		return 0;
	}

	if (w >= 1.0)
	{
		return 1.0;
	}

	return w * w * (3.0 - 2.0 * w);
}

float Perlin::Interpolate(float a0, float a1, float w) const
{
	return a0 + (a1 - a0) * SmoothStep(w);
}
