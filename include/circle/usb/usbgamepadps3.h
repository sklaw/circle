//
// usbgamepadps3.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _circle_usb_usbgamepadps3_h
#define _circle_usb_usbgamepadps3_h

#include <circle/usb/usbgamepad.h>
#include <circle/types.h>

class CUSBGamePadPS3Device : public CUSBGamePadDevice
{
public:
	CUSBGamePadPS3Device (CUSBFunction *pFunction);
	~CUSBGamePadPS3Device (void);

	boolean Configure (void);

	const TGamePadState *GetReport (void);		// returns 0 on failure

private:
	void DecodeReport (const u8 *pReportBuffer);

	boolean PS3Enable (void);

private:
	boolean m_bInterfaceOK;
};

#endif
