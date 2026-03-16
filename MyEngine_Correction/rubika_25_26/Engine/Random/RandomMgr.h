#pragma once

#include <random>
#include <unordered_map>

#include <Engine/Debug/DebugMgr.h>

class RandomInstance
{
public:
	RandomInstance();

	unsigned RandUInt(unsigned min, unsigned max);
	int RandInt32(int min, int max);
	double RandDouble(double min, double max);
	double RandNormalDouble(double center, double disp);

	void SetSeed(uint32_t seed);
	uint32_t GetSeed() const { return Seed; }

private:
	std::default_random_engine RandomEng;
	uint32_t Seed;
};

class RandomMgr : public IDebugable
{
public:
	RandomMgr();

	void Init();
	void Shut();

	unsigned CreateInstance();
	void DestroyInstance(unsigned instanceId);

	unsigned RandUInt(unsigned min, unsigned max) const;
	unsigned RandUInt(unsigned instanceId, unsigned min, unsigned max) const;
	int RandInt32(int min, int max) const;
	int RandInt32(unsigned instanceId, int min, int max) const;
	double RandDouble(double min, double max) const;
	double RandDouble(unsigned instanceId, double min, double max) const;
	double RandNormalDouble(double center, double disp) const;
	double RandNormalDouble(unsigned instanceId, double center, double disp) const;

	void SetSeed(uint32_t seed) const;
	void SetSeed(unsigned instanceId, uint32_t seed) const;
	uint32_t GetSeed() const;
	uint32_t GetSeed(unsigned instanceId) const;

	static uint32_t GenerateRandomSeed();

	RandomInstance* GetInstance(unsigned instanceId) const;

	virtual void DrawDebug() override;

private:


	const unsigned InstanceGlobalId = 0;
	unsigned InstanceIdCount = 1;

	std::unordered_map<unsigned, RandomInstance> Instances;
};

