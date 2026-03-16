#include "DrunkardWalk.h"

#include <Engine/Profiler.h>
#include <Engine/Random/RandomMgr.h>
#include <Engine/Globals.h>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>

#include <stdlib.h>
#include <assert.h>
#include <set>
#include <vector>

DrunkardWalk::DrunkardWalk(unsigned sizeX, unsigned sizeY, unsigned iteration, unsigned distance, unsigned distanceFactor) :
	SizeX(sizeX), SizeY(sizeY), Iteration(iteration), Distance(distance), DistanceFactor(distanceFactor)
{
}

DrunkardWalk::~DrunkardWalk()
{
	Reset();
}

bool DrunkardWalk::Generate()
{
	Reset();

	bool bInit = false;
	PROFILER_EVENT_BEGIN(PROFILER_COLOR_PURPLE, "DrunkardWalk Generation");
	{
		PROFILER_EVENT_BEGIN(PROFILER_COLOR_CYAN, "DrunkardWalk Generation : InitializeGrid");
		{
			bInit = InitializeGrid();
		}
		PROFILER_EVENT_END();

		if (bInit)
		{
			PROFILER_EVENT_BEGIN(PROFILER_COLOR_PINK, "DrunkardWalk Generation : RunIterations");
			RunIterations();
			PROFILER_EVENT_END();
		}
	}
	PROFILER_EVENT_END();
	return bInit;
}

char DrunkardWalk::GetValue(unsigned x, unsigned y) const
{
	return 0;
}

void DrunkardWalk::GenerateTexture(sf::Texture& texture) const
{
	if (!Pixels)
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
	for (unsigned x = 0; x < SizeX; ++x)
	{
		for (unsigned y = 0; y < SizeY; ++y)
		{
			unsigned index = Index2DTo1D(x, y);

			sf::Color color = (Pixels[index] == 1) ? sf::Color(0, 0, 0, 255) : sf::Color(255, 255, 255, 255);
			img.setPixel({ x, y }, color);
		}
	}

	if (!texture.loadFromImage(img))
	{
		assert(false);
	}

	free(data);
}

void DrunkardWalk::Reset()
{
	if (Pixels)
	{
		free(Pixels);
		Pixels = nullptr;
	}
}

bool DrunkardWalk::InitializeGrid()
{
	Pixels = (char*)calloc(SizeX * SizeY, sizeof(char));
	return Pixels != nullptr;
}

void DrunkardWalk::RunIterations()
{
	unsigned randomInstanceId = gData.RandomMgr->CreateInstance();
	RandomInstance* randomInstance = gData.RandomMgr->GetInstance(randomInstanceId);
	assert(randomInstance);
	randomInstance->SetSeed(RandomMgr::GenerateRandomSeed());

	std::set<unsigned> walkedCellIndexSet;
	std::vector<unsigned> walkedCellIndexVector;
	walkedCellIndexVector.reserve(SizeX * SizeY);

	for (unsigned i = 0; i < Iteration; ++i)
	{
		unsigned currentPosition = 0;

		if (i == 0)
		{
			//unsigned initX = randomInstance->RandUInt(0, SizeX - 1);
			//unsigned initY = randomInstance->RandUInt(0, SizeY - 1);

			unsigned initX = SizeX / 2;
			unsigned initY = SizeY / 2;

			currentPosition = Index2DTo1D(initX, initY);

			walkedCellIndexSet.insert(currentPosition);
			walkedCellIndexVector.push_back(currentPosition);
		}
		else
		{
			unsigned index = randomInstance->RandUInt(0, static_cast<unsigned>(walkedCellIndexVector.size()) - 1);
			currentPosition = walkedCellIndexVector[index];
		}

		Pixels[currentPosition] = 1;

		unsigned distance = 0;
		while (distance < Distance)
		{
			unsigned dir = randomInstance->RandUInt(0, 3);

			int currentPositionX, currentPositionY;
			Index1DTo2D(currentPosition, currentPositionX, currentPositionY);

			int offsetX, offsetY;

			switch (dir)
			{
			case 0:
				offsetX = 1;
				offsetY = 0;
				break;

			case 1:
				offsetX = -1;
				offsetY = 0;
				break;

			case 2:
				offsetX = 0;
				offsetY = 1;
				break;

			case 3:
				offsetX = 0;
				offsetY = -1;
				break;

			default:
				assert(false);
				break;
			}

			int newPositionX = std::clamp(currentPositionX + offsetX * static_cast<int>(DistanceFactor), 0, static_cast<int>(SizeX) - 1);
			int newPositionY = std::clamp(currentPositionY + offsetY * static_cast<int>(DistanceFactor), 0, static_cast<int>(SizeY) - 1);

			unsigned x = currentPositionX;
			while (x != newPositionX)
			{
				x += offsetX;
				unsigned newPosition = Index2DTo1D(x, currentPositionY);
				Pixels[newPosition] = 1;
				if (!walkedCellIndexSet.contains(newPosition))
				{
					walkedCellIndexSet.insert(newPosition);
					walkedCellIndexVector.push_back(newPosition);
				}

				currentPosition = newPosition;
			}

			unsigned y = currentPositionY;
			while (y != newPositionY)
			{
				y += offsetY;
				unsigned newPosition = Index2DTo1D(currentPositionX, y);
				Pixels[newPosition] = 1;
				if (!walkedCellIndexSet.contains(newPosition))
				{
					walkedCellIndexSet.insert(newPosition);
					walkedCellIndexVector.push_back(newPosition);
				}

				currentPosition = newPosition;
			}

			++distance;
		}
	}

	gData.RandomMgr->DestroyInstance(randomInstanceId);
}

unsigned DrunkardWalk::Index2DTo1D(int x, int y) const
{
	return y * SizeX + x;
}

void DrunkardWalk::Index1DTo2D(int index, int& x, int& y) const
{
	x = index % SizeX;
	y = index / SizeX;
}
