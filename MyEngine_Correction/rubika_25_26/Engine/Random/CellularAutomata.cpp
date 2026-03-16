#include "CellularAutomata.h"

#include <Engine/Globals.h>
#include <Engine/Random/RandomMgr.h>
#include <Engine/Profiler.h>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <assert.h>

#define CELL_ALIVE 0
#define CELL_DEAD 1

CellularAutomata::CellularAutomata(unsigned sizeX, unsigned sizeY, double spawnPercent, unsigned threshold, unsigned iteration) :
	SizeX(sizeX), SizeY(sizeY), SpawnPercent(spawnPercent), NeightborThreshold(threshold), Iteration(iteration)
{
}

bool CellularAutomata::Generate()
{
	Reset();

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_PURPLE, "CellularAutomata Generation");
	{
		PROFILER_EVENT_BEGIN(PROFILER_COLOR_CYAN, "CellularAutomata Generation : InitializeCells");
		{
			InitializeCells();
		}
		PROFILER_EVENT_END();

		for (unsigned i = 0; i < Iteration; ++i)
		{
			PROFILER_EVENT_BEGIN(PROFILER_COLOR_PINK, "CellularAutomata Generation : Compute iteration %d", i);
			ComputeIteration(i);
			PROFILER_EVENT_END();
		}
	}
	PROFILER_EVENT_END();

	return true;
}


void CellularAutomata::GenerateTexture(std::vector<sf::Texture>& textures) const
{
	textures.clear();
	textures.reserve(Iteration);

	size_t dataSize = SizeX * SizeY;
	unsigned char* data = (unsigned char*)calloc(dataSize * 4, sizeof(unsigned char));
	if (!data)
	{
		return;
	}

	for (unsigned i = 0; i < Iteration; ++i)
	{
		sf::Image img({ SizeX, SizeY }, data);

		auto& v = Cells[i];
		for (unsigned x = 0; x < SizeX; ++x)
		{
			for (unsigned y = 0; y < SizeY; ++y)
			{
				sf::Color color = (v[x][y] == CELL_ALIVE) ? sf::Color(0, 0, 0, 255) : sf::Color(255, 255, 255, 255);
				img.setPixel({ x, y }, color);
			}
		}

		textures.push_back(std::move(sf::Texture(img)));
	}

	free(data);

	return;
}

short CellularAutomata::GetValue(unsigned x, unsigned y) const
{
	if (x >= SizeX || y >= SizeY)
	{
		return 0;
	}

	return Cells[Iteration - 1][x][y];
}

void CellularAutomata::InitializeCells()
{
	for (unsigned i = 0; i < Iteration; ++i)
	{
		auto& v = Cells[i];
		v.resize(SizeX);

		for (unsigned x = 0; x < SizeX; ++x)
		{
			auto& v2 = v[x];
			v2.resize(SizeY);

			memset(v2.data(), 0, sizeof(short) * SizeY);
		}
	}
}

void CellularAutomata::ComputeIteration(unsigned iteration)
{
	if (iteration == 0)
	{
		ComputeFirstIteration();
	}
	else if (iteration < Iteration)
	{
		ComputeSecondaryIteration(iteration);
	}
}

void CellularAutomata::ComputeFirstIteration()
{
	unsigned randomInstanceId = gData.RandomMgr->CreateInstance();
	RandomInstance* randomInstance = gData.RandomMgr->GetInstance(randomInstanceId);
	assert(randomInstance);
	randomInstance->SetSeed(RandomMgr::GenerateRandomSeed());

	auto& v = Cells[0];
	for (unsigned x = 0; x < SizeX; ++x)
	{
		auto& v2 = v[x];
		for (unsigned y = 0; y < SizeY; ++y)
		{
			double r = randomInstance->RandDouble(0, 1);
			if (r <= SpawnPercent)
			{
				v2[y] = CELL_ALIVE;
			}
			else
			{
				v2[y] = CELL_DEAD;
			}
		}
	}

	gData.RandomMgr->DestroyInstance(randomInstanceId);
}

void CellularAutomata::ComputeSecondaryIteration(unsigned iteration)
{
	if (iteration == 0 || iteration >= Iteration)
	{
		return;
	}

	auto& oldV = Cells[iteration - 1];
	auto& v = Cells[iteration];

	for (unsigned x = 0; x < SizeX; ++x)
	{
		for (unsigned y = 0; y < SizeY; ++y)
		{
			unsigned neighborCount = 0;
			for (int offsetX : { -1, 0, 1})
			{
				for (int offsetY : { -1, 0, 1})
				{
					if (offsetX == 0 && offsetY == 0)
					{
						continue;
					}

					long long checkX = x + offsetX;
					long long checkY = y + offsetY;

					if (checkX < 0 || checkX >= SizeX || checkY < 0 || checkY >= SizeY)
					{
						++neighborCount;
					}
					else if (oldV[checkX][checkY] == CELL_ALIVE)
					{
						++neighborCount;
					}
				}
			}

			v[x][y] = neighborCount >= NeightborThreshold ? CELL_ALIVE : CELL_DEAD;
		}
	}
}

void CellularAutomata::Reset()
{
	for (auto& v : Cells)
	{
		v.clear();
	}
}
