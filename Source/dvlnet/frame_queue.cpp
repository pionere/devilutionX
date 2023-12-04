#include "frame_queue.h"

#include <cstring>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void frame_queue::read(uint32_t s, BYTE* dest)
{
	//if (current_size < s)
	//	throw frame_queue_exception();
	uint32_t bs;
	BYTE* src;

	while (s > 0) {
		auto& next_buf = buffer_deque.front();
		bs = next_buf.size() - current_offset;
		src = next_buf.data() + current_offset;
		if (s >= bs) {
			// read the whole entry
			s -= bs;
			current_size -= bs;
			memcpy(dest, src, bs);
			buffer_deque.pop_front();
			current_offset = 0;
			continue;
		}
		// read part of the entry
		memcpy(dest, src, s);
		current_size -= s;
		current_offset += s;
		break;
	}
}

bool frame_queue::packet_ready()
{
	if (next_size == 0) {
		if (current_size < sizeof(uint32_t))
			return false;
		read(sizeof(uint32_t), (BYTE*)&next_size);
		next_size = SwapLE32(next_size);
		if (next_size == 0)
			// should not happen. Ignore the packet to avoid crash
			return false; // throw frame_queue_exception();
	}
	return current_size >= next_size;
}

buffer_t frame_queue::read_packet()
{
	//if (next_size == 0 || current_size < next_size)
	//	throw frame_queue_exception();
	buffer_t ret(next_size);
	read(next_size, ret.data());
	next_size = 0;
	return ret;
}

void frame_queue::write(const buffer_t& buf, unsigned len)
{
	current_size += len;
	buffer_deque.push_back(buffer_t(buf.begin(), buf.begin() + len));
}

void frame_queue::clear()
{
	next_size = 0;
	current_size = 0;
	current_offset = 0;
	buffer_deque.clear();
}

buffer_t* frame_queue::make_frame(const buffer_t packetbuf)
{
	buffer_t* ret;
	uint32_t size = packetbuf.size();
	//if (size > (MAX_FRAME_SIZE - sizeof(uint32_t)))
	//	app_error(ERR_APP_FRAME_BUFSIZE);
	ret = new buffer_t(sizeof(uint32_t) + size);
	//if (ret == NULL)
	//	app_error(ERR_APP_FRAME_BUFSIZE);
	BYTE* data = ret->data();
	*(uint32_t*)data = SwapLE32(size);
	memcpy(data + sizeof(uint32_t), packetbuf.data(), size);
	return ret;
}

} // namespace net
DEVILUTION_END_NAMESPACE
