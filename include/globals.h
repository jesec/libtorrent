#ifndef LIBTORRENT_GLOBALS_H
#define LIBTORRENT_GLOBALS_H

#include "torrent/utils/priority_queue_default.h"
#include "torrent/utils/timer.h"

namespace torrent {

extern torrent::utils::priority_queue_default taskScheduler;
extern torrent::utils::timer                  cachedTime;

} // namespace torrent

#endif
