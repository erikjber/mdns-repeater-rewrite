/*
 * packet-rewrite.h - mDNS repeater packet rewrite utility
 * Copyright (C) 2025 Erik Berglund
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#define TYPE_A  1
#define TYPE_AAAA 28

// Helper to read a DNS name (handles compression only partially)
int read_dns_name(const uint8_t *packet, size_t packet_len, size_t offset, char *out, size_t out_len, size_t *name_end) {
    size_t pos = offset;
    size_t out_pos = 0;
    int jumped = 0;
    size_t jump_pos = 0;

    while (pos < packet_len) {
        uint8_t len = packet[pos];
        if (len == 0) {
            if (!jumped) pos++;
            break;
        }
        if ((len & 0xC0) == 0xC0) { // pointer
            if (!jumped) jump_pos = pos + 2;
            size_t ptr = ((len & 0x3F) << 8) | packet[pos + 1];
            pos = ptr;
            jumped = 1;
            continue;
        }
        pos++;
        if (out_pos + len + 1 >= out_len) return -1;
        memcpy(out + out_pos, packet + pos, len);
        out_pos += len;
        out[out_pos++] = '.';
        pos += len;
    }
    if (out_pos == 0) {
        out[0] = '\0';
    } else {
        out[out_pos - 1] = '\0'; // remove trailing dot
    }
    *name_end = jumped ? jump_pos : pos;
	return 0;
}

/**
 * @brief Find the size of the questions section of an mDNS packet.
 * 
 * @param questions The number of questions.
 * @param packet The mDNS packet data.
 * @param length The lenght of the packet data.
 * @param data_pointer An output variable that, once this method returns, points to the first byte after the questions section.
 */
void scan_questions(uint16_t questions, const uint8_t * packet, size_t length, size_t *data_pointer){
	for(int q = 0;q<questions;q++){
		char name[1024];
		size_t end = 0;
		read_dns_name(packet,length,*data_pointer,name,1024,&end);
		*data_pointer = end+4;
		if(*data_pointer >= length)
			break;
	}
}

/**
 * @brief Replace all IP addresses in the Resource Record (both IPv4 and IPv6) with one of the arguments, as appropriate.
 * 
 * @param records The number of mDNS Resource Records to replace.
 * @param packet The mDNS packet.
 * @param length The lenght of the mDNS packet.
 * @param data_pointer A pointer to the start of the Resource Record section. After returning this will point to the start of the next section.
 * @param inet4_addr The new IPv4 address, or NULL.
 * @param inet6_addr The new IPv6 address, or NULL.
 */
void scan_and_replace_rr( uint16_t records, uint8_t * packet, size_t length, size_t *data_pointer, uint8_t *inet4_addr, uint8_t *inet6_addr){
	for(int r = 0;r<records;r++){
		char name[1024];
		size_t end = 0;
		read_dns_name(packet,length,*data_pointer,name,1024,&end);
		*data_pointer = end;
		uint16_t type = (packet[*data_pointer]<<8)|packet[*data_pointer+1];
        // We're skipping the two "class" and "flush cache" bytes and the four "TTL" bytes
		*data_pointer += 8;
		uint16_t data_length = (packet[*data_pointer]<<8)|packet[*data_pointer+1];
		*data_pointer += 2;
        if (type == TYPE_A && inet4_addr != NULL){
            // Replace the IPv4 address
            memcpy(&packet[*data_pointer],inet4_addr,data_length);
        }
        else if (type == TYPE_AAAA && inet6_addr != NULL){
            // Replace the IPv6 address
            memcpy(&packet[*data_pointer],inet6_addr,data_length);
        }
		*data_pointer += data_length;
		if(*data_pointer >= length)
			break;
	}
}


/**
 * @brief Replace all IP addresses (both IPv4 and IPv6) with one of the arguments, as appropriate.
 * 
 * @param packet The mDNS packet.
 * @param length The lenght of the mDNS packet.
 * @param new_address_IPv4 The new IPv4 address, or NULL.
 * @param new_address_IPv6 The new IPv6 address, or NULL.
 */
void rewrite_mDNS_packet(uint8_t *packet,size_t length, struct in_addr *new_address_IPv4, struct in6_addr *new_address_IPv6){
	if (length < 12){
		printf("Packet too short.");
		return;
	}

	// The two first bytes are the ID which is usually zero, ignore
	// the third and fourth bytes are the flags, we ignore them

    // Get the number of entries in the four main sections of the packet
	uint16_t num_questions      = (packet[4] << 8)  | packet[5];
	uint16_t num_answers        = (packet[6] << 8)  | packet[7];
	uint16_t num_authorities    = (packet[8] << 8)  | packet[9];
	uint16_t num_additional     = (packet[10] << 8) | packet[11];
	size_t data_pointer = 12;

    // Convert internet addresses to byte arrays
    uint8_t *inet4_addr = NULL;
    if (new_address_IPv4 != NULL){
        inet4_addr = malloc(4*sizeof(uint8_t));
        for (int i = 0; i < 4; i++)
        {
            inet4_addr[i] = (new_address_IPv4->s_addr>>(i*8)) & 0xFF;
        }
    }
    uint8_t *inet6_addr = NULL;
    if (new_address_IPv4 != NULL){
        inet6_addr = new_address_IPv6->__in6_u.__u6_addr8;
    }

    // Scan througgh the entire packet
	scan_questions(num_questions,packet,length,&data_pointer);
	scan_and_replace_rr(num_answers,packet,length,&data_pointer,inet4_addr,inet6_addr);
	scan_and_replace_rr(num_authorities,packet,length,&data_pointer,inet4_addr,inet6_addr);
	scan_and_replace_rr(num_additional,packet,length,&data_pointer,inet4_addr,inet6_addr);
    if (inet4_addr != NULL){
        free(inet4_addr);
    }
}
