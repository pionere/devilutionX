#include "frame_queue.h"

#include <cstring>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void frame_queue::read(uint32_t s, BYTE* dest)
{
	//if (current_size < s)
	//	throw frame_queue_exception();
	while (s > 0 && s >= buffer_deque.front().size()) {
		s -= buffer_deque.front().size();
		current_size -= buffer_deque.front().size();
		memcpy(dest, buffer_deque.front().data(), buffer_deque.front().size());
		dest += buffer_deque.front().size();
		buffer_deque.pop_front();
	}
	if (s > 0) {
		memcpy(dest, buffer_deque.front().data(), s);
		buffer_deque.front().erase(buffer_deque.front().begin(),
		    buffer_deque.front().begin() + s);
		current_size -= s;
	}
}

void frame_queue::write(buffer_t buf)
{
	current_size += buf.size();
	buffer_deque.push_back(std::move(buf));
}

bool frame_queue::packet_ready()
{
	if (nextsize == 0) {
		if (current_size < sizeof(uint32_t))
			return false;
		read(sizeof(uint32_t), (BYTE*)&nextsize);
		nextsize = SwapLE32(nextsize);
		if (nextsize == 0)
			// should not happen. Ignore the packet to avoid crash
			return false; // throw frame_queue_exception();
	}
	return current_size >= nextsize;
}

buffer_t frame_queue::read_packet()
{
	//if (nextsize == 0 || current_size < nextsize)
	//	throw frame_queue_exception();
	buffer_t ret(nextsize);
	read(nextsize, ret.data());
	nextsize = 0;
	return ret;
}

buffer_t* frame_queue::make_frame(buffer_t packetbuf)
{
	buffer_t* ret;
	uint32_t size = packetbuf.size();
	//if (size > (MAX_FRAME_SIZE - sizeof(uint32_t)))
	//	app_error(ERR_APP_FRAME_BUFSIZE);
	ret = new buffer_t(sizeof(uint32_t) + size);
	if (ret == NULL)
		app_error(ERR_APP_FRAME_BUFSIZE);
	BYTE* data = ret->data();
	*(uint32_t*)data = SwapLE32(size);
	memcpy(data + sizeof(uint32_t), packetbuf.data(), size);
	return ret;
}

} // namespace net
DEVILUTION_END_NAMESPACE
