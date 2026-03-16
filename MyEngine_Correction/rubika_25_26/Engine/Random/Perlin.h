#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>

namespace sf
{
	class Texture;
}

class Perlin
{
public:
	Perlin(unsigned sizeX, unsigned sizeY, unsigned cellSize, unsigned octaves);

	bool Generate();
	float GetValue(unsigned x, unsigned y) const;
	void GenerateTexture(sf::Texture& texture, bool blackAndWhite) const;

private:
	void PopulateVectors();
	void GeneratePixels();
	float GetPerlinValue(float x, float y) const;
	void Reset();

	float DotGridGradient(unsigned ix, unsigned iy, float x, float y) const;
	float Interpolate(float a0, float a1, float w) const;

	std::vector<std::vector<sf::Vector2f>> m_vVectors;
	std::vector<std::vector<float>> m_vPixels;

	unsigned SizeX;
	unsigned SizeY;
	unsigned CellSize;
	unsigned GridSizeX;
	unsigned GridSizeY;
	unsigned Octaves;
};