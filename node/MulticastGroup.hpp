/*
 * ZeroTier One - Global Peer to Peer Ethernet
 * Copyright (C) 2012-2013  ZeroTier Networks LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * ZeroTier may be used and distributed under the terms of the GPLv3, which
 * are available at: http://www.gnu.org/licenses/gpl-3.0.html
 *
 * If you would like to embed ZeroTier into a commercial application or
 * redistribute it in a modified binary form, please contact ZeroTier Networks
 * LLC. Start here: http://www.zerotier.com/
 */

#ifndef _ZT_MULTICASTGROUP_HPP
#define _ZT_MULTICASTGROUP_HPP

#include <stdint.h>
#include <string>
#include "MAC.hpp"
#include "InetAddress.hpp"

namespace ZeroTier {

/**
 * A multicast group composed of a multicast MAC and a 32-bit ADI field
 *
 * ADI stands for additional distinguishing information. ADI is primarily for
 * adding additional information to broadcast (ff:ff:ff:ff:ff:ff) memberships,
 * since straight-up broadcast won't scale. Right now it's zero except for
 * IPv4 ARP, where it holds the IPv4 address itself to make ARP into a
 * selective multicast query that can scale.
 *
 * In the future we might add some kind of plugin architecture that can add
 * ADI for things like mDNS (multicast DNS) to improve the selectivity of
 * those protocols.
 *
 * MulticastGroup behaves as an immutable value object.
 */
class MulticastGroup
{
public:
	MulticastGroup()
		throw() :
		_mac(),
		_adi(0)
	{
	}

	MulticastGroup(const MAC &m,uint32_t a)
		throw() :
		_mac(m),
		_adi(a)
	{
	}

	/**
	 * Derive the multicast group used for address resolution (ARP/NDP) for an IP
	 *
	 * @param ip IP address (port field is ignored)
	 * @return Multicat group for ARP/NDP
	 */
	static inline MulticastGroup deriveMulticastGroupForAddressResolution(const InetAddress &ip)
		throw()
	{
		if (ip.isV4()) {
			// IPv4 wants braodcast MACs, so we shove the V4 address itself into
			// the Multicast Group ADI field. Making V4 ARP work is basically why
			// ADI was added, as well as handling other things that want mindless
			// Ethernet broadcast to all.
			return MulticastGroup(MAC((unsigned char)0xff),Utils::ntoh(*((const uint32_t *)ip.rawIpData())));
		} else if (ip.isV6()) {
			// IPv6 is better designed in this respect. We can compute the IPv6
			// multicast address directly from the IP address, and it gives us
			// 24 bits of uniqueness. Collisions aren't likely to be common enough
			// to care about.
			const unsigned char *a = (const unsigned char *)ip.rawIpData();
			MAC m;
			m.data[0] = 0x33;
			m.data[1] = 0x33;
			m.data[2] = 0xff;
			m.data[3] = a[13];
			m.data[4] = a[14];
			m.data[5] = a[15];
			return MulticastGroup(m,0);
		}
		return MulticastGroup();
	}

	/**
	 * @return Human readable string representing this group (MAC/ADI in hex)
	 */
	inline std::string toString() const
	{
		char buf[64];
		Utils::snprintf(buf,sizeof(buf),"%.2x%.2x%.2x%.2x%.2x%.2x/%lx",(unsigned int)_mac.data[0],(unsigned int)_mac.data[1],(unsigned int)_mac.data[2],(unsigned int)_mac.data[3],(unsigned int)_mac.data[4],(unsigned int)_mac.data[5],(unsigned long)_adi);
		return std::string(buf);
	}

	/**
	 * @return Multicast address
	 */
	inline const MAC &mac() const throw() { return _mac; }

	/**
	 * @return Additional distinguishing information
	 */
	inline uint32_t adi() const throw() { return _adi; }

	inline bool operator==(const MulticastGroup &g) const throw() { return ((_mac == g._mac)&&(_adi == g._adi)); }
	inline bool operator!=(const MulticastGroup &g) const throw() { return ((_mac != g._mac)||(_adi != g._adi)); }
	inline bool operator<(const MulticastGroup &g) const throw()
	{
		if (_mac < g._mac)
			return true;
		else if (_mac == g._mac)
			return (_adi < g._adi);
		return false;
	}
	inline bool operator>(const MulticastGroup &g) const throw() { return (g < *this); }
	inline bool operator<=(const MulticastGroup &g) const throw() { return !(g < *this); }
	inline bool operator>=(const MulticastGroup &g) const throw() { return !(*this < g); }

private:
	MAC _mac;
	uint32_t _adi;
};

} // namespace ZeroTier

#endif
