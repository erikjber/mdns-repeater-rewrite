mdns-repeater-rewrite
==============
mdns-repeater-rewrite is a Multicast DNS repeater for Linux. Multicast DNS uses the 
224.0.0.251 address, which is "administratively scoped" and does not 
leave the subnet.

This program re-broadcast mDNS packets from one interface to other interfaces.
This allows zeroconf devices to work properly across two or more subnets.
The program is also capable of rewriting source addresses on the fly in order
to solve some routing problems.

mdns-repeate-rewrite is a fork of mdns-repeater by Darell Tan.

USAGE
-----
mdns-repeater-rewrite only requires the interface names and it will do the rest.
For example, the dd-wrt standard installation defines br0 for the wireless 
interface and vlan1 as the WAN interface, I would use:

    mdns-repeater-rewrite br0 vlan1

You can also specify the -f flag for debugging, which prints packets as they 
are received.

If you specify the -r flag, packets will have their IP addresses re-written 
on the fly so that they appear to point to the machine running mdns-repeater-rewrite. 
See the REWRITING section for why this might sometimes be useful.

You are free to modify the code to repeat whatever traffic you require, as
long as you abide by the software license.

REWRITING
---------
mdns-repeater-rewrite is capable of not just forwarding packets, but also rewriting
IP addresses so that they appear to point to the mdns-machine.

"Vanilla" mdns-repeater-rewrite is sufficient if the machines on all networks served
by mdns-repeater-rewrite knows how to route on all other networks. This might not be 
the case, for example if several small networks with the same network addresses are connected 
to one larger, central network. If all the small networks have the network address
192.168.1.0/24 (for example), a machine on the larger network will have no way to route
packets to one of the machines on one of the small networks.

This could be handled  automatically if mDNS resolvers used the IP address of 
the source of an mDNS packet instead of the IP address in the Response section 
of the mDNS packet. 

Unfortunately, it is not know if any mDNS resolvers do this - systemd-resolved, for exmple, does not.
This issue is not covered by RFC 6762.
To get around this limitation, rewriting is introduced. For rewriting to work, 
NAT and port forwarding must be in use on the machines routing between the
networks.


LICENSE
--------
Copyright (C) 2011 Darell Tan
Rewrite changes copyright (C) 2025 Erik Berglund

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

