#pragma once

#include <deque>
//#include <exception>
#include <cstdint>

#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

/*class frame_queue_exception : public std::exception {
public:
	const char* what() const throw() override
	{
		return "Incorrect frame size";
	}
};*/

class frame_queue {
public:
	constexpr static uint32_t MAX_FRAME_SIZE = 0xFFFF;

private:
	uint32_t current_size = 0;
	std::deque<buffer_t> buffer_deque;
	uint32_t next_size = 0;
	uint32_t current_offset = 0;

	void read(uint32_t s, BYTE* dest);

public:
	/* Report whether the frame contains a completed packet. */
	bool packet_ready();
	/* Read the next packet from the queue. Assumes the packet is ready (packet_ready returns true). */
	buffer_t read_packet();
	/* Append the content of a buffer to the frame. */
	void write(const buffer_t& buf, unsigned len);
	/* Clear the content of the frame. */
	void clear();

	static buffer_t* make_frame(const buffer_t packetbuf);
};

} // namespace net
DEVILUTION_END_NAMESPACE
