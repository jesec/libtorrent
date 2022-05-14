#include "test/helpers/chunk.h"
#include "torrent/exceptions.h"

torrent::Chunk*
func_create_chunk(uint32_t index, int) {
  // Do proper handling of prot_flags...
  char* memory_part1 = (char*)mmap(
    NULL, 10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

  if (memory_part1 == MAP_FAILED)
    throw torrent::internal_error(
      "func_create_chunk() failed: " +
      torrent::utils::error_number::current().message());

  std::memset(memory_part1, index, 10);

  auto chunk = new torrent::Chunk();
  chunk->push_back(torrent::ChunkPart::MAPPED_MMAP,
                   torrent::MemoryChunk(memory_part1,
                                        memory_part1,
                                        memory_part1 + 10,
                                        torrent::MemoryChunk::prot_read,
                                        0));

  if (chunk == nullptr)
    throw torrent::internal_error(
      "func_create_chunk() failed: chunk == nullptr.");

  return chunk;
}

uint64_t
func_free_diskspace(torrent::ChunkList*) {
  return 0;
}

void
func_storage_error(torrent::ChunkList*, const std::string&) {}

void
chunk_done(done_chunks_type*          done_chunks,
           torrent::HashChunk*        hash_chunk,
           const torrent::HashString& hash_value) {
  std::lock_guard lk(done_chunks_lock);
  (*done_chunks)[hash_chunk->handle().index()] = hash_value;
}

void
chunk_done(torrent::ChunkList*  chunk_list,
           done_chunks_type*    done_chunks,
           torrent::ChunkHandle handle,
           const char*          hash_value) {
  if (hash_value != nullptr)
    (*done_chunks)[handle.index()] =
      *torrent::HashString::cast_from(hash_value);

  chunk_list->release(&handle);
}

torrent::HashString
hash_for_index(uint32_t index) {
  char buffer[10];
  std::memset(buffer, index, 10);

  torrent::Sha1       sha1;
  torrent::HashString hash;
  sha1.init();
  sha1.update(buffer, 10);
  sha1.final_c(hash.data());

  return hash;
}

bool
verify_hash(const done_chunks_type*    done_chunks,
            int                        index,
            const torrent::HashString& hash) {
  done_chunks_lock.lock();

  auto itr = done_chunks->find(index);

  if (itr == done_chunks->end()) {
    done_chunks_lock.unlock();
    return false;
  }

  bool matches = itr->second == hash;
  done_chunks_lock.unlock();

  if (!matches) {
    // std::cout << "chunk compare: " << index << " "
    //           << torrent::hash_string_to_hex_str(itr->second) << ' ' <<
    //           torrent::hash_string_to_hex_str(hash) << ' '
    //           << (itr != done_chunks->end() && itr->second == hash)
    //           << std::endl;
    throw torrent::internal_error("Could not verify hash...");
  }

  return true;
}