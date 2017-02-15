cmake_minimum_required(VERSION 2.8)
project(libhidpp)

set(LIBHIDPP_SOURCES
	misc/HIDRaw.cpp
	misc/Log.cpp
	misc/CRC.cpp
	misc/UsageStrings.cpp
	hidpp/Device.cpp
	hidpp/Report.cpp
	hidpp/DeviceInfo.cpp
	hidpp/Setting.cpp
	hidpp/SettingLookup.cpp
	hidpp/Enum.cpp
	hidpp/Address.cpp
	hidpp/Profile.cpp
	hidpp/Macro.cpp
	base/ProfileFormat.cpp
	base/MemoryMapping.cpp
	base/MacroFormat.cpp
	hidpp10/Device.cpp
	hidpp10/Error.cpp
	hidpp10/WriteError.cpp
	hidpp10/IMemory.cpp
	hidpp10/IReceiver.cpp
	hidpp10/IIndividualFeatures.cpp
	hidpp10/Sensor.cpp
	hidpp10/IResolution.cpp
	hidpp10/IProfile.cpp
	hidpp10/ProfileDirectoryFormat.cpp
	hidpp10/ProfileFormat.cpp
	hidpp10/ProfileFormatCommon.cpp
	hidpp10/ProfileFormatG500.cpp
	hidpp10/ProfileFormatG700.cpp
	hidpp10/MemoryMapping.cpp
	hidpp10/RAMMapping.cpp
	hidpp10/MacroFormat.cpp
	hidpp20/Device.cpp
	hidpp20/Error.cpp
	hidpp20/IRoot.cpp
	hidpp20/IFeatureSet.cpp
	hidpp20/IOnboardProfiles.cpp
	hidpp20/ProfileDirectoryFormat.cpp
	hidpp20/ProfileFormat.cpp
	hidpp20/MemoryMapping.cpp
	hidpp20/MacroFormat.cpp
)

include_directories(.)
add_library(hidpp SHARED ${LIBHIDPP_SOURCES})

