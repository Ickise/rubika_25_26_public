#pragma once

namespace sf
{
	class Texture;
}

class DrunkardWalk
{
public:
	DrunkardWalk(unsigned sizeX, unsigned sizeY, unsigned iteration, unsigned distance, unsigned distanceFactor);
	~DrunkardWalk();

	bool Generate();
	char GetValue(unsigned x, unsigned y) const;
	void GenerateTexture(sf::Texture& texture) const;

private:
	void Reset();
	bool InitializeGrid();
	void RunIterations();

	unsigned Index2DTo1D(int x, int y) const;
	void Index1DTo2D(int index, int& x, int& y) const;

	char* Pixels;

	unsigned SizeX;
	unsigned SizeY;
	unsigned Iteration;
	unsigned Distance;
	unsigned DistanceFactor;
};