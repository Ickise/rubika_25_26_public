#include "RandomMgr.h"

#include <Engine/Globals.h>
#include <Engine/Random/Perlin.h>
#include <Engine/Random/CellularAutomata.h>
#include <Engine/Random/DrunkardWalk.h>

#ifdef _USE_IMGUI
#include <Imgui/imgui.h>
#include <Imgui/imgui-SFML.h>
#include <SFML/Graphics/Texture.hpp>
#endif

#include <assert.h>

#define GET_INSTANCE_AND_CHECK(...) \
	RandomInstance* instance = GetInstance(instanceId);	\
	assert(instance);	\
	if (!instance)	\
	{	\
		return __VA_ARGS__;	\
	}

RandomInstance::RandomInstance() : Seed(0)
{
}

unsigned RandomInstance::RandUInt(unsigned min, unsigned max)
{
	std::uniform_int_distribution<uint32_t> uid(min, max);
	return uid(RandomEng);
}

int RandomInstance::RandInt32(int min, int max)
{
	std::uniform_int_distribution<int32_t> uid(min, max);
	return uid(RandomEng);
}

double RandomInstance::RandDouble(double min, double max)
{
	std::uniform_real_distribution<double> urd(min, max);
	return urd(RandomEng);
}

double RandomInstance::RandNormalDouble(double center, double disp)
{
	std::normal_distribution<double> nrd(center, disp);
	return nrd(RandomEng);
}

void RandomInstance::SetSeed(uint32_t seed)
{
	RandomEng.seed(seed);
	Seed = seed;
}

RandomMgr::RandomMgr()
{
	Instances.emplace(std::piecewise_construct,
		std::forward_as_tuple(InstanceGlobalId),
		std::forward_as_tuple());
}

void RandomMgr::Init()
{
	gData.DebugMgr->RegisterDebugableWindow("RandomMgr", this);
}

void RandomMgr::Shut()
{
	gData.DebugMgr->UnregisterDebugableWindow("RandomMgr");
}

unsigned RandomMgr::CreateInstance()
{
	Instances.emplace(std::piecewise_construct,
		std::forward_as_tuple(InstanceIdCount),
		std::forward_as_tuple());

	return InstanceIdCount++;
}

void RandomMgr::DestroyInstance(unsigned instanceId)
{
	if (instanceId == InstanceGlobalId)
	{
		return;
	}

	Instances.erase(instanceId);
}

unsigned RandomMgr::RandUInt(unsigned min, unsigned max) const
{
	return RandUInt(InstanceGlobalId, min, max);
}

unsigned RandomMgr::RandUInt(unsigned instanceId, unsigned min, unsigned max) const
{
	GET_INSTANCE_AND_CHECK(0);
	return instance->RandUInt(min, max);
}

int RandomMgr::RandInt32(int min, int max) const
{
	return RandInt32(InstanceGlobalId, min, max);
}

int RandomMgr::RandInt32(unsigned instanceId, int min, int max) const
{
	GET_INSTANCE_AND_CHECK(0);
	return instance->RandInt32(min, max);
}

double RandomMgr::RandDouble(double min, double max) const
{
	return RandDouble(InstanceGlobalId, min, max);
}

double RandomMgr::RandDouble(unsigned instanceId, double min, double max) const
{
	GET_INSTANCE_AND_CHECK(0);
	return instance->RandDouble(min, max);
}

double RandomMgr::RandNormalDouble(double center, double disp) const
{
	return RandNormalDouble(InstanceGlobalId, center, disp);
}

double RandomMgr::RandNormalDouble(unsigned instanceId, double center, double disp) const
{
	GET_INSTANCE_AND_CHECK(0);
	return instance->RandNormalDouble(center, disp);
}

void RandomMgr::SetSeed(uint32_t seed) const
{
	SetSeed(InstanceGlobalId, seed);
}

void RandomMgr::SetSeed(unsigned instanceId, uint32_t seed) const
{
	GET_INSTANCE_AND_CHECK();
	instance->SetSeed(seed);
}

uint32_t RandomMgr::GetSeed() const
{
	return GetSeed(InstanceGlobalId);
}

uint32_t RandomMgr::GetSeed(unsigned instanceId) const
{
	GET_INSTANCE_AND_CHECK(0);
	return instance->GetSeed();
}

uint32_t RandomMgr::GenerateRandomSeed()
{
	std::random_device rd;
	return rd();
}

RandomInstance* RandomMgr::GetInstance(unsigned instanceId) const
{
	const auto& it = Instances.find(instanceId);
	if (it == Instances.end())
	{
		return nullptr;
	}

	return const_cast<RandomInstance*>(&it->second);
}

void RandomMgr::DrawDebug()
{
#ifdef _USE_IMGUI

	if (ImGui::CollapsingHeader("Instances"))
	{

	}
	else if (ImGui::CollapsingHeader("Perlin"))
	{
		static int size = 256;
		static int cellSize = 8;
		static int octaves = 4;
		static bool drawRGB = true;
		static sf::Texture perlinTexture;

		ImGui::SliderInt("Texture Size", &size, 16, 4096);
		ImGui::SliderInt("Cell Size", &cellSize, 1, size);
		ImGui::SliderInt("Octaves", &octaves, 1, 16);
		ImGui::Checkbox("Draw RGB", &drawRGB);

		if (ImGui::Button("Generate new perlin noise"))
		{
			Perlin p(size, size, cellSize, octaves);
			p.Generate();
			p.GenerateTexture(perlinTexture, !drawRGB);
		}

		ImGui::Image(perlinTexture);
	}
	else if (ImGui::CollapsingHeader("Cellular Automata"))
	{
		static int size = 256;
		static float spawnRate = 0.6f;
		static int threshold = 5;
		static int iteration = 5;
		static std::vector<sf::Texture> CATextures;

		ImGui::SliderInt("Texture Size", &size, 16, 4096);
		ImGui::SliderFloat("Spawn Rate", &spawnRate, 0.f, 1.f);
		ImGui::SliderInt("Nb neightbor to survive", &threshold, 1, 9);
		ImGui::SliderInt("Nb iteration", &iteration, 1, CellularAutomata::NbMaxIteration);

		static int indexToDisplay = 0;
		if (ImGui::Button("Generate new cellular automata"))
		{
			CellularAutomata c(size, size, spawnRate, threshold, iteration);
			c.Generate();
			c.GenerateTexture(CATextures);
			indexToDisplay = 0;
		}

		if (indexToDisplay < CATextures.size())
		{
			ImGui::Image(CATextures[indexToDisplay]);
		}
		ImGui::SliderInt("Generation to display", &indexToDisplay, 0, static_cast<int>(CATextures.size()) - 1);
	}
	else if (ImGui::CollapsingHeader("Drunkard Walk"))
	{
		static int size = 256;
		static int iteration = 5;
		static int distance = 25;
		static int distanceFactor = 1;
		static sf::Texture drunkardWalkTexture;

		ImGui::SliderInt("Texture Size", &size, 16, 4096);
		ImGui::SliderInt("Iteration", &iteration, 1, 5000);
		ImGui::SliderInt("Distance each iteration", &distance, 1, 100);
		ImGui::SliderInt("Distance Factor", &distanceFactor, 1, 25);

		if (ImGui::Button("Generate new drunkard walk"))
		{
			DrunkardWalk dw(size, size, iteration, distance, distanceFactor);
			dw.Generate();
			dw.GenerateTexture(drunkardWalkTexture);
		}

		ImGui::Image(drunkardWalkTexture);
	}
#endif
}
