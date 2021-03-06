#pragma once

#include <atomic>
#include <chrono>
#include <cuda_runtime.h>
#include "nv_api.h"
#include "..\types.h"

#pragma managed(push, off)

#ifdef _M_CEE 
#	undef _M_CEE 
#	include <thread>
#	define _M_CEE 001 
#else 
#	include <thread>
#endif 

#pragma managed(pop)

#define DEFALUT_INTENSITY 25.0f

class Device
{
private:
	static uint32_t constexpr MAX_TPB_500{ 1024u };
	static uint32_t constexpr MAX_TPB_350{ 384u };

public:
	int deviceID;
	std::string name;
	uint32_t computeVersion;
	float intensity;

	bool initialized;
	bool mining;

	std::thread miningThread;
	std::atomic<uint64_t> hashCount;
	std::atomic<std::chrono::steady_clock::time_point> hashStartTime;

	uint64_t* d_Solutions;
	uint64_t* h_Solutions;
	uint32_t* d_SolutionCount;
	uint32_t* h_SolutionCount;

	bool checkChanges;
	bool isNewTarget;
	bool isNewMessage;

	message_ut currentMessage;
	sponge_ut currentMidstate;
	byte32_t currentTarget;
	uint64_t currentHigh64Target;

private:
	dim3 m_block;
	dim3 m_grid;

	uint32_t m_lastCompute;
	uint32_t m_lastBlockX;
	uint32_t m_lastThreads;
	float m_lastIntensity;

	NV_API m_api;
	uint32_t pciBusID;

public:
	Device(int deviceID);

	bool getSettingMaxCoreClock(int *maxCoreClock, std::string *errorMessage);
	bool getSettingMaxMemoryClock(int *maxMemoryClock, std::string *errorMessage);
	bool getSettingPowerLimit(int *powerLimit, std::string *errorMessage);
	bool getSettingThermalLimit(int *thermalLimit, std::string *errorMessage);
	bool getSettingFanLevelPercent(int *fanLevel, std::string *errorMessage);

	bool getCurrentFanTachometerRPM(int *tachometerRPM, std::string *errorMessage);
	bool getCurrentTemperature(int *temperature, std::string *errorMessage);
	bool getCurrentCoreClock(int *coreClock, std::string *errorMessage);
	bool getCurrentMemoryClock(int *memoryClock, std::string *errorMessage);
	bool getCurrentUtilizationPercent(int *utilization, std::string *errorMessage);
	bool getCurrentPstate(int *pstate, std::string *errorMessage);
	bool getCurrentThrottleReasons(std::string *reasons, std::string *errorMessage);

	uint32_t threads();
	dim3 block();
	dim3 grid();

	uint64_t hashRate();
};
