// cwchessboard -- A C++ chessboard tool set
//
//! @file MemoryBlockList.cc This file contains the implementation of class MemoryBlockList.
//
// Copyright (C) 2010, by
// 
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef USE_PCH
#include "sys.h"
#include <glib.h>
#endif

#include "MemoryBlockList.h"

namespace util {

// Let the data start at the first multiple of sizeof(size_t) after the MemoryBlockNode object.
size_t const MemoryBlockNode::S_data_offset = (sizeof(MemoryBlockNode) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);

void MemoryBlockList::need_more_data_callback(void)
{
  // Create and read at most one new block per 'buffer full' incident.
  // If multiple new blocks have been processed in the mean time,
  // then the call to read_async_ready will trigger subsequent reads.
  if (!M_buffer_full)
    return;
  M_buffer_full = false;
  M_slot_need_more_data();
}

void MemoryBlockListIterator::advance_to_next_block(void)
{
  // Count the number of fully processed blocks.
  ++M_processed_blocks;
  Dout(dc::notice, "Finished processing of block " << M_processed_blocks);
  if (G_UNLIKELY(M_block->is_last_block()))
  {
    // We should never even have been processing the last block,
    // unless the buffer was already closed.
    assert(M_buffer->closed());

    // This might free the memory of this block, if no other iterator is pointing at it.
    Dout(dc::notice, "Setting M_block to NULL.");
    M_block.reset();

    // We're done. Set the iterator to past-the-end and return.
    M_ptr = NULL;
    return;
  }
  else
  {
    M_block = M_block->next();
    M_ptr = M_block->block_begin();
    M_block_end = M_block->block_end() - 1;
  }
  if (M_buffer->full() && M_buffer->appended_blocks() - M_processed_blocks <= MemoryBlockList::S_max_blocks / 2)
  {
    Dout(dc::notice, "Requesting more data!");
    M_buffer->need_more_data().emit();
  }
  // If M_buffer->full() was just false but around here the main thread
  // sets it to true, then that means that the number of remaining blocks
  // in the buffer is much larger than the threshold and the previous
  // condition would have failed anyway. What would happen then is that
  // the next condition is definitely false (because there ARE blocks to
  // be processed), the next block in the buffer will be processed and
  // we'll recheck the above condition. Hence, there is no race condition.
  while (!M_buffer->can_process_next_block(*this))
  {
    // If can_process_next_block returned false (so we get here)
    // then the buffer has less than 2 blocks is thus not full (yet).
    // If now we manage to get into a sleeping state before the
    // buffer is full then everything works. But, if right here
    // the buffer is filled -- which causes an attempt to wake
    // up this thread, but we're not inside wait() yet... and
    // then we enter wait(), then we have a hang.
    // Therefore it is needed to lock the mutex then test if
    // the buffer is full and only if NOT we may really sleep.
    // In the main thread then we may only mark that the buffer is
    // full and emit the wake up signal if this mutex is not locked.
    // The locking and extra buffer full test is done inside wait_for_more_data().

    // This is triggered when a new block is appended, either when there are at least two blocks,
    // or when the last block was appended (the buffer was closed).
    M_buffer->wait_for_more_data(*this);
  }
}

} // namespace util
