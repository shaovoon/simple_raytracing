#pragma once

#include <random>

class RandomNumGen
{
public:
	RandomNumGen()
		: gen(rd())
		, dis(0.0, 1.0)
	{
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<> dis(1.0, 2.0);
	}
	double GetRand()
	{
		return dis(gen);
	}

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen; //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis;

};
