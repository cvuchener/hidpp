#include <hidpp10/IReceiver.h>

#include <hidpp10/defs.h>
#include <hidpp10/Device.h>

#include <stdexcept>

using namespace HIDPP10;

IReceiver::IReceiver (Device *dev):
	_dev (dev)
{
}

void IReceiver::getDeviceInformation (unsigned int device,
				      uint8_t *destination_id,
				      uint8_t *report_interval,
				      uint16_t *wpid,
				      DeviceType *type)
{
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	HIDPP::Parameters params (HIDPP::ShortParamLength),
			  results (HIDPP::LongParamLength);

	params[0] = DeviceInformation | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");

	if (destination_id)
		*destination_id = results[1];
	if (report_interval)
		*report_interval = results[2];
	if (wpid)
		*wpid = results.getWordBE (3);
	if (type)
		*type = static_cast<DeviceType> (results[7]);
}

void IReceiver::getDeviceExtendedInformation (unsigned int device,
					      uint32_t *serial,
					      uint32_t *report_types,
					      PowerSwitchLocation *ps_loc)
{
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	HIDPP::Parameters params (HIDPP::ShortParamLength),
			  results (HIDPP::LongParamLength);

	params[0] = ExtendedDeviceInformation | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");

	if (serial)
		*serial = results.getDWordBE (1);
	if (report_types)
		*report_types = results.getDWordBE (5);
	if (ps_loc)
		*ps_loc = static_cast<PowerSwitchLocation> (results[9] & 0x0F);
}

std::string IReceiver::getDeviceName (unsigned int device)
{
	
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	HIDPP::Parameters params (HIDPP::ShortParamLength),
			  results (HIDPP::LongParamLength);

	params[0] = DeviceName | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");
	
	std::size_t length = results[1];
	return std::string (reinterpret_cast<char *> (&results[2]), std::min (length, 14ul));
}
