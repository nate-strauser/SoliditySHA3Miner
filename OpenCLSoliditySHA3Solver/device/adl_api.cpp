#include "adl_api.h"

#pragma unmanaged

// --------------------------------------------------------------------
// Static
// --------------------------------------------------------------------

HINSTANCE ADL_API::hDLL{ NULL };

ADL_API::ADL_MAIN_CONTROL_CREATE				ADL_API::ADL_Main_Control_Create{ NULL };
ADL_API::ADL_MAIN_CONTROL_DESTROY				ADL_API::ADL_Main_Control_Destroy{ NULL };
ADL_API::ADL_ADAPTER_NUMBEROFADAPTERS_GET		ADL_API::ADL_Adapter_NumberOfAdapters_Get{ NULL };
ADL_API::ADL_ADAPTER_ADAPTERINFO_GET			ADL_API::ADL_Adapter_AdapterInfo_Get{ NULL };
ADL_API::ADL2_OVERDRIVE_CAPS					ADL_API::ADL2_Overdrive_Caps{ NULL };
ADL_API::ADL2_OVERDRIVEN_CAPABILITIES_GET		ADL_API::ADL2_OverdriveN_Capabilities_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET		ADL_API::ADL2_OverdriveN_SystemClocks_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_MEMORYCLOCKS_GET		ADL_API::ADL2_OverdriveN_MemoryClocks_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET	ADL_API::ADL2_OverdriveN_PerformanceStatus_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_FANCONTROL_GET			ADL_API::ADL2_OverdriveN_FanControl_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_POWERLIMIT_GET			ADL_API::ADL2_OverdriveN_PowerLimit_Get{ NULL };
ADL_API::ADL2_OVERDRIVEN_TEMPERATURE_GET		ADL_API::ADL2_OverdriveN_Temperature_Get{ NULL };

int ADL_API::numberOfAdapters{ -1 };
LPAdapterInfo ADL_API::lpAdapterInfo{ NULL };
bool ADL_API::isInitialized{ false };

void* __stdcall ADL_API::ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

bool ADL_API::foundAdlApi()
{
	return (LoadLibrary(TEXT(ADL64_API)) != NULL);
}

void ADL_API::initialize()
{
	hDLL = LoadLibrary(TEXT(ADL64_API));
	if (hDLL == NULL) throw std::exception("Failed to initialize ADL64_API.");

	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
	ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");

	if (NULL == ADL_Main_Control_Create || NULL == ADL_Main_Control_Destroy)
		throw std::exception("Failed to get ADL function pointers.");

	ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
	ADL2_OverdriveN_Capabilities_Get = (ADL2_OVERDRIVEN_CAPABILITIES_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_Capabilities_Get");
	ADL2_OverdriveN_SystemClocks_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocks_Get");
	ADL2_OverdriveN_MemoryClocks_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
	ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PerformanceStatus_Get");
	ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_FanControl_Get");
	ADL2_OverdriveN_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PowerLimit_Get");
	ADL2_OverdriveN_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_Temperature_Get");
	ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_Overdrive_Caps");

	if (ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) != ADL_OK)
		throw std::exception("Failed to initialize nested ADL2 context.");

	if (ADL_Adapter_NumberOfAdapters_Get(&numberOfAdapters) != ADL_OK)
		throw std::exception("Cannot get the number of adapters!\n");

	if (0 < numberOfAdapters)
	{
		lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * numberOfAdapters);
		memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * numberOfAdapters);

		ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * numberOfAdapters);
	}
}

void ADL_API::unload()
{
	if (ADL_Main_Control_Destroy != NULL) ADL_Main_Control_Destroy();

	if (hDLL != NULL) FreeLibrary(hDLL);
}

// --------------------------------------------------------------------
// Private
// --------------------------------------------------------------------

bool ADL_API::checkVersion(std::string *errorMessage)
{
	*errorMessage = "";
	if (m_version == 7) return true;

	*errorMessage = "Only ADL Overdrive version 7 is supported.";
	*errorMessage += " Current version: " + std::to_string(m_version);
	return false;
}

bool ADL_API::getOverDriveNCapabilities(ADLODNCapabilities *capabilities, std::string *errorMessage)
{
	*errorMessage = "";
	if (ADL2_OverdriveN_Capabilities_Get(m_context, m_adapterInfo.iAdapterIndex, capabilities) == ADL_OK) return true;

	*errorMessage = "Failed ADL2_OverdriveN_Capabilities_Get.";
	return false;
}

// --------------------------------------------------------------------
// Public
// --------------------------------------------------------------------

void ADL_API::assignPciBusID(int adapterBusID)
{
	m_context = NULL;
	m_enabled = NULL;
	m_version = NULL;
	m_supported = NULL;
	this->m_adapterBusID = adapterBusID;

	for (int i = 0; i < numberOfAdapters; ++i)
		if (lpAdapterInfo[i].iBusNumber == adapterBusID) m_adapterInfo = lpAdapterInfo[i];

	ADL2_Overdrive_Caps(m_context, m_adapterInfo.iAdapterIndex, &m_supported, &m_enabled, &m_version);
}

void ADL_API::getAdapterName(std::string *adapterName)
{
	*adapterName = m_adapterInfo.strAdapterName;
}

bool ADL_API::getSettingMaxCoreClock(int *maxCoreClock, std::string *errorMessage)
{
	*maxCoreClock = -1;

	int bufSize{ -1 };
	void* performanceLevelsBuffer{ NULL };
	ADLODNCapabilities overdriveCapabilities{ NULL };
	ADLODNPerformanceLevels *odPerformanceLevels{ NULL };
	
	try
	{
		if (!checkVersion(errorMessage)) return false;

		if (!getOverDriveNCapabilities(&overdriveCapabilities, errorMessage)) return false;

		bufSize = sizeof(ADLODNPerformanceLevels) + (sizeof(ADLODNPerformanceLevel) * (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1));
		performanceLevelsBuffer = new char[bufSize];
		std::memset(performanceLevelsBuffer, 0, bufSize);

		odPerformanceLevels = (ADLODNPerformanceLevels *)performanceLevelsBuffer;
		odPerformanceLevels->iSize = bufSize;
		odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;

		if (ADL2_OverdriveN_SystemClocks_Get(m_context, m_adapterInfo.iAdapterIndex, odPerformanceLevels) != ADL_OK)
		{
			*errorMessage = "Failed ADL2_OverdriveN_SystemClocks_Get.";
			free(performanceLevelsBuffer);
			return false;
		}

		for (int i = 0; i < overdriveCapabilities.iMaximumNumberOfPerformanceLevels; ++i)
			if (odPerformanceLevels->aLevels[i].iEnabled && (odPerformanceLevels->aLevels[i].iClock > *maxCoreClock))
				*maxCoreClock = odPerformanceLevels->aLevels[i].iClock;

		if (*maxCoreClock > 0) *maxCoreClock /= 100; // trim 2 decimal places

		free(performanceLevelsBuffer);
		return true;
	}
	catch (std::exception ex) { *errorMessage = ex.what(); }

	if (performanceLevelsBuffer != NULL) free(performanceLevelsBuffer);

	return false;
}

bool ADL_API::getSettingMaxMemoryClock(int *maxMemoryClock, std::string *errorMessage)
{
	*maxMemoryClock = -1;

	int bufSize{ -1 };
	void* performanceLevelsBuffer{ NULL };
	ADLODNCapabilities overdriveCapabilities{ NULL };
	ADLODNPerformanceLevels *odPerformanceLevels{ NULL };

	try
	{
		if (!checkVersion(errorMessage)) return false;

		if (!getOverDriveNCapabilities(&overdriveCapabilities, errorMessage)) return false;

		bufSize = sizeof(ADLODNPerformanceLevels) + (sizeof(ADLODNPerformanceLevel) * (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1));
		performanceLevelsBuffer = new char[bufSize];
		std::memset(performanceLevelsBuffer, 0, bufSize);

		odPerformanceLevels = (ADLODNPerformanceLevels *)performanceLevelsBuffer;
		odPerformanceLevels->iSize = bufSize;
		odPerformanceLevels->iMode = 0; // current
		odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;

		if (ADL2_OverdriveN_MemoryClocks_Get(m_context, m_adapterInfo.iAdapterIndex, odPerformanceLevels) != ADL_OK)
		{
			*errorMessage = "Failed ADL2_OverdriveN_MemoryClocks_Get.";
			free(performanceLevelsBuffer);
			return false;
		}

		for (int i = 0; i < overdriveCapabilities.iMaximumNumberOfPerformanceLevels; ++i)
			if (odPerformanceLevels->aLevels[i].iEnabled && (odPerformanceLevels->aLevels[i].iClock > *maxMemoryClock))
				*maxMemoryClock = odPerformanceLevels->aLevels[i].iClock;

		if (*maxMemoryClock > 0) *maxMemoryClock /= 100; // remove 2 decimal places

		free(performanceLevelsBuffer);
		return true;
	}
	catch (std::exception ex) { *errorMessage = ex.what(); }

	if (performanceLevelsBuffer != NULL) free(performanceLevelsBuffer);

	return false;
}

bool ADL_API::getSettingPowerLimit(int *powerLimit, std::string *errorMessage)
{
	*powerLimit = INT32_MIN;

	ADLODNPowerLimitSetting odNPowerControl{ NULL };

	if (!checkVersion(errorMessage)) return false;

	if (ADL2_OverdriveN_PowerLimit_Get(m_context, m_adapterInfo.iAdapterIndex, &odNPowerControl) == ADL_OK)
	{
		*powerLimit = odNPowerControl.iTDPLimit;
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PowerLimit_Get";
	return false;
}

bool ADL_API::getSettingThermalLimit(int *thermalLimit, std::string *errorMessage)
{
	*thermalLimit = INT32_MIN;

	ADLODNPowerLimitSetting odNPowerControl{ NULL };

	if (!checkVersion(errorMessage)) return false;

	if (ADL2_OverdriveN_PowerLimit_Get(m_context, m_adapterInfo.iAdapterIndex, &odNPowerControl) == ADL_OK)
	{
		*thermalLimit = odNPowerControl.iMaxOperatingTemperature;
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PowerLimit_Get";
	return false;
}

bool ADL_API::getSettingFanLevelPercent(int *fanLevel, std::string *errorMessage)
{
	*fanLevel = -1;

	ADLODNCapabilities overdriveCapabilities{ NULL };
	ADLODNFanControl odNFanControl{ NULL };

	if (!checkVersion(errorMessage)) return false;

	if (!getOverDriveNCapabilities(&overdriveCapabilities, errorMessage)) return false;

	if (ADL2_OverdriveN_FanControl_Get(m_context, m_adapterInfo.iAdapterIndex, &odNFanControl) == ADL_OK)
	{
		const int stepCount = overdriveCapabilities.fanSpeed.iMax - overdriveCapabilities.fanSpeed.iMin;
		*fanLevel = (int)((double)odNFanControl.iTargetFanSpeed / stepCount * 100);
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_FanControl_Get";
	return false;
}

bool ADL_API::getCurrentFanTachometerRPM(int *tachometerRPM, std::string *errorMessage)
{
	*tachometerRPM = -1;

	ADLODNFanControl odNFanControl{ NULL };

	if (!checkVersion(errorMessage)) return false;

	if (ADL2_OverdriveN_FanControl_Get(m_context, m_adapterInfo.iAdapterIndex, &odNFanControl) == ADL_OK)
	{
		*tachometerRPM = odNFanControl.iCurrentFanSpeed;
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_FanControl_Get";
	return false;
}

bool ADL_API::getCurrentTemperature(int *temperature, std::string *errorMessage)
{
	*temperature = INT32_MIN;
	int tempTemperature{ 0 };

	if (ADL2_OverdriveN_Temperature_Get(m_context, m_adapterInfo.iAdapterIndex, 1, &tempTemperature) == ADL_OK)
	{
		*temperature = tempTemperature / 1000;
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PowerLimit_Get";
	return false;
}

bool ADL_API::getCurrentCoreClock(int *coreClock, std::string *errorMessage)
{
	*coreClock = -1;

	ADLODNPerformanceStatus odNPerformanceStatus{ NULL };

	if (ADL2_OverdriveN_PerformanceStatus_Get(m_context, m_adapterInfo.iAdapterIndex, &odNPerformanceStatus) == ADL_OK)
	{
		*coreClock = odNPerformanceStatus.iCoreClock / 100; // trim 2 decimal places
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PerformanceStatus_Get";
	return false;
}

bool ADL_API::getCurrentMemoryClock(int *memoryClock, std::string *errorMessage)
{
	*memoryClock = -1;

	ADLODNPerformanceStatus odNPerformanceStatus{ NULL };

	if (ADL2_OverdriveN_PerformanceStatus_Get(m_context, m_adapterInfo.iAdapterIndex, &odNPerformanceStatus) == ADL_OK)
	{
		*memoryClock = odNPerformanceStatus.iMemoryClock / 100; // trim 2 decimal places
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PerformanceStatus_Get";
	return false;
}

bool ADL_API::getCurrentUtilizationPercent(int *utilization, std::string *errorMessage)
{
	*utilization = -1;

	ADLODNPerformanceStatus odNPerformanceStatus{ NULL };

	if (ADL2_OverdriveN_PerformanceStatus_Get(m_context, m_adapterInfo.iAdapterIndex, &odNPerformanceStatus) == ADL_OK)
	{
		*utilization = odNPerformanceStatus.iGPUActivityPercent;
		return true;
	}

	*errorMessage = "Failed ADL2_OverdriveN_PerformanceStatus_Get";
	return false;
}
