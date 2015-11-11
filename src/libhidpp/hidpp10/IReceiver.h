#ifndef HIDPP10_IRECEIVER_H
#define HIDPP10_IRECEIVER_H

#include <cstdint>
#include <string>

namespace HIDPP10
{

class Device;

class IReceiver
{
public:
	enum InformationType: uint8_t {
		DeviceInformation = 0x20,
		ExtendedDeviceInformation = 0x30,
		DeviceName = 0x40,
	};

	enum DeviceType: uint8_t {
		Unknown = 0x00,
		Keyboard = 0x01,
		Mouse = 0x02,
		Numpad = 0x03,
		Presenter = 0x04,
		Trackball = 0x08,
		Touchpad = 0x09,
	};

	enum PowerSwitchLocation {
		Base = 0x1,
		TopCase = 0x2,
		TopRightCornerEdge = 0x3,
		Other = 0x4,
		TopLeftCorner = 0x5,
		BottomLeftCorner = 0x6,
		TopRightCorner = 0x7,
		BottomRightCorner = 0x8,
		TopEdge = 0x9,
		RightEdge = 0xA,
		LeftEdge = 0xB,
		BottomEdge = 0xC,
	};

	IReceiver (Device *dev);

	void getDeviceInformation (unsigned int device,
				   uint8_t *destination_id,
				   uint8_t *report_interval,
				   uint16_t *wpid,
				   DeviceType *type);
	void getDeviceExtendedInformation (unsigned int device,
					   uint32_t *serial,
					   uint32_t *report_types,
					   PowerSwitchLocation *ps_loc);
	std::string getDeviceName (unsigned int device);

private:
	Device *_dev;
};

}

#endif

