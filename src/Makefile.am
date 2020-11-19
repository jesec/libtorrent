lib_LTLIBRARIES = libtorrent.la
noinst_LTLIBRARIES = \
	libtorrent_torrent.la \
	libtorrent_other.la

libtorrent_la_LDFLAGS = -version-info $(LIBTORRENT_INTERFACE_VERSION_INFO)
libtorrent_la_LIBADD = \
	libtorrent_torrent.la \
	libtorrent_other.la

libtorrent_la_SOURCES = \
	globals.cc \
	manager.cc \
	thread_disk.cc \
	thread_main.cc

libtorrent_torrent_la_SOURCES = \
	torrent/data/block.cc \
	torrent/data/block_list.cc \
	torrent/data/chunk_utils.cc \
	torrent/data/download_data.cc \
	torrent/data/file.cc \
	torrent/data/file_list.cc \
	torrent/data/file_list_iterator.cc \
	torrent/data/file_manager.cc \
	torrent/data/file_utils.cc \
	torrent/data/transfer_list.cc \
	torrent/download/choke_group.cc \
	torrent/download/choke_queue.cc \
	torrent/download/download_manager.cc \
	torrent/download/resource_manager.cc \
	torrent/net/address_info.cc \
	torrent/net/fd.cc \
	torrent/net/socket_address.cc \
	torrent/net/socket_address_key.cc \
	torrent/net/socket_event.cc \
	torrent/peer/client_info.cc \
	torrent/peer/client_list.cc \
	torrent/peer/connection_list.cc \
	torrent/peer/peer.cc \
	torrent/peer/peer_info.cc \
	torrent/peer/peer_list.cc \
	torrent/utils/directory_events.cc \
	torrent/utils/log.cc \
	torrent/utils/log_buffer.cc \
	torrent/utils/option_strings.cc \
	torrent/utils/random.cc \
	torrent/utils/resume.cc \
	torrent/utils/signal_bitfield.cc \
	torrent/utils/thread_base.cc \
	torrent/utils/thread_interrupt.cc \
	torrent/utils/uri_parser.cc \
	torrent/bitfield.cc \
	torrent/chunk_manager.cc \
	torrent/connection_manager.cc \
	torrent/dht_manager.cc \
	torrent/download.cc \
	torrent/error.cc \
	torrent/event.cc \
	torrent/exceptions.cc \
	torrent/hash_string.cc \
	torrent/http.cc \
	torrent/object.cc \
	torrent/object_static_map.cc \
	torrent/object_stream.cc \
	torrent/path.cc \
	torrent/poll_epoll.cc \
	torrent/poll_kqueue.cc \
	torrent/poll_select.cc \
	torrent/rate.cc \
	torrent/throttle.cc \
	torrent/torrent.cc \
	torrent/tracker.cc \
	torrent/tracker_controller.cc \
	torrent/tracker_list.cc \
	torrent/tracker_list.h

libtorrent_other_la_SOURCES = \
	data/chunk.cc \
	data/chunk_list.cc \
	data/chunk_part.cc \
	data/hash_check_queue.cc \
	data/hash_chunk.cc \
	data/hash_queue.cc \
	data/hash_queue_node.cc \
	data/hash_torrent.cc \
	data/memory_chunk.cc \
	data/socket_file.cc \
	dht/dht_bucket.cc \
	dht/dht_node.cc \
	dht/dht_router.cc \
	dht/dht_server.cc \
	dht/dht_tracker.cc \
	dht/dht_transaction.cc \
	download/available_list.cc \
	download/chunk_selector.cc \
	download/chunk_statistics.cc \
	download/delegator.cc \
	download/download_constructor.cc \
	download/download_main.cc \
	download/download_wrapper.cc \
	net/address_list.cc \
	net/local_addr.cc \
	net/listen.cc \
	net/socket_base.cc \
	net/socket_datagram.cc \
	net/socket_fd.cc \
	net/socket_listen.cc \
	net/socket_set.cc \
	net/socket_stream.cc \
	net/throttle_internal.cc \
	net/throttle_list.cc \
	protocol/extensions.cc \
	protocol/handshake.cc \
	protocol/handshake_encryption.cc \
	protocol/handshake_manager.cc \
	protocol/initial_seed.cc \
	protocol/peer_connection_base.cc \
	protocol/peer_connection_leech.cc \
	protocol/peer_connection_metadata.cc \
	protocol/peer_factory.cc \
	protocol/request_list.cc \
	tracker/tracker_dht.cc \
	tracker/tracker_http.cc \
	tracker/tracker_udp.cc \
	utils/diffie_hellman.cc \
	utils/instrumentation.cc \
	utils/sha_fast.cc \
	utils/queue_buckets.h

AM_CPPFLAGS = -I$(top_srcdir)/include
