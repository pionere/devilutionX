#include "frame_queue.h"

#include <cstring>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

buffer_t frame_queue::read(uint32_t s)
{
	//if (current_size < s)
	//	throw frame_queue_exception();
	buffer_t ret;
	while (s > 0 && s >= buffer_deque.front().size()) {
		s -= buffer_deque.front().size();
		current_size -= buffer_deque.front().size();
		ret.insert(ret.end(),
		    buffer_deque.front().begin(),
		    buffer_deque.front().end());
		buffer_deque.pop_front();
	}
	if (s > 0) {
		ret.insert(ret.end(),
		    buffer_deque.front().begin(),
		    buffer_deque.front().begin() + s);
		buffer_deque.front().erase(buffer_deque.front().begin(),
		    buffer_deque.front().begin() + s);
		current_size -= s;
	}
	return ret;
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
		auto szbuf = read(sizeof(uint32_t));
		std::memcpy(&nextsize, &szbuf[0], sizeof(uint32_t));
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
	auto ret = read(nextsize);
	nextsize = 0;
	return ret;
}

buffer_t frame_queue::make_frame(buffer_t packetbuf)
{
	buffer_t ret;
	if (packetbuf.size() > MAX_FRAME_SIZE)
		app_error(ERR_APP_FRAME_BUFSIZE);
	uint32_t size = SwapLE32(packetbuf.size());
	ret.insert(ret.end(), packet_factory::begin(size), packet_factory::end(size));
	ret.insert(ret.end(), packetbuf.begin(), packetbuf.end());
	return ret;
}

} // namespace net
DEVILUTION_END_NAMESPACE
