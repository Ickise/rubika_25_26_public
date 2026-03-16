#pragma once

#include <vector>
#include <string>

namespace sf
{
	class Texture;
}

class CellularAutomata
{
public:
	CellularAutomata(unsigned sizeX, unsigned sizeY, double spawnPercent, unsigned threshold, unsigned iteration);

	bool Generate();
	short GetValue(unsigned x, unsigned y) const;

	void GenerateTexture(std::vector<sf::Texture>& textures) const;

	static const unsigned NbMaxIteration = 10;

private:

	void InitializeCells();
	void ComputeIteration(unsigned iteration);
	void ComputeFirstIteration();
	void ComputeSecondaryIteration(unsigned iteration);
	void Reset();

	unsigned SizeX;
	unsigned SizeY;
	double SpawnPercent;
	unsigned NeightborThreshold;
	unsigned Iteration;

	std::vector<std::vector<short>> Cells[NbMaxIteration];
};