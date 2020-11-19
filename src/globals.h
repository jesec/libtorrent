#ifndef LIBTORRENT_GLOBALS_H
#define LIBTORRENT_GLOBALS_H

#include <rak/priority_queue_default.h>
#include <rak/timer.h>

namespace torrent {

extern rak::priority_queue_default taskScheduler;
extern rak::timer                  cachedTime;

} // namespace torrent

#endif
