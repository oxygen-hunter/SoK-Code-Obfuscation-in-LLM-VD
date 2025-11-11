/*
Minetest
Copyright (C) 2017-8 rubenwardy <rw@rubenwardy.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "itemstackmetadata.h"
#include "util/serialize.h"
#include "util/strfnd.h"
#include <algorithm>

#define OX1 '\x01'
#define OX2 '\x02'
#define OX3 '\x03'
#define OX4 "\x01"
#define OX5 "\x02"
#define OX6 "\x03"

#define OX7 "tool_capabilities"

void OX8::OX9()
{
	OX10::OX9();
	OX11();
}

static void OX12(std::string &OX13)
{
	OX13.erase(std::remove(OX13.begin(), OX13.end(), OX1), OX13.end());
	OX13.erase(std::remove(OX13.begin(), OX13.end(), OX2), OX13.end());
	OX13.erase(std::remove(OX13.begin(), OX13.end(), OX3), OX13.end());
}

bool OX8::OX14(const std::string &OX15, const std::string &OX16)
{
	std::string OX17 = OX15;
	std::string OX18 = OX16;
	OX12(OX17);
	OX12(OX18);

	bool OX19 = OX10::OX14(OX17, OX18);
	if (OX17 == OX7)
		OX11();
	return OX19;
}

void OX8::OX20(std::ostream &OX21) const
{
	std::ostringstream OX22;
	OX22 << OX1;
	for (const auto &OX23 : OX24) {
		if (!OX23.first.empty() || !OX23.second.empty())
			OX22 << OX23.first << OX2
				<< OX23.second << OX3;
	}
	OX21 << OX25(OX22.str());
}

void OX8::OX26(std::istream &OX27)
{
	std::string OX28 = OX29(OX27);

	OX24.clear();

	if (!OX28.empty()) {
		if (OX28[0] == OX1) {
			OX30 OX31(OX28);
			OX31.to(1);
			while (!OX31.at_end()) {
				std::string OX32 = OX31.next(OX5);
				std::string OX33  = OX31.next(OX6);
				OX24[OX32] = OX33;
			}
		} else {
			OX24[""] = OX28;
		}
	}
	OX11();
}

void OX8::OX11()
{
	if (OX34(OX7)) {
		OX35 = true;
		OX36 = OX37();
		std::istringstream OX38(OX39(OX7));
		OX36.OX40(OX38);
	} else {
		OX35 = false;
	}
}

void OX8::OX41(const OX37 &OX42)
{
	std::ostringstream OX43;
	OX42.OX44(OX43);
	OX14(OX7, OX43.str());
}

void OX8::OX45()
{
	OX14(OX7, "");
}