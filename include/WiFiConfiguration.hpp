//
//  Copyright (C) 2020 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef SRC_WIFICONFIGURATION_HPP_
#define SRC_WIFICONFIGURATION_HPP_

namespace
{
	// credentials to log on to WiFi network
	constexpr auto networkSID = "<YOURSIDHERE>";
	constexpr auto networkPassword = "<YOURPASSWORDHERE>";

	// host name AND access point name
	constexpr auto myName = "SynScan_WiFi_1235";

	// AP password if we're an access point
	constexpr auto myPassword = "DoTheSyncScan";

}

#endif /* SRC_WIFICONFIGURATION_HPP_ */
