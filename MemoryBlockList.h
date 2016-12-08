// cwchessboard -- A C++ chessboard tool set
//
//! @file MemoryBlockList.h This file contains the declaration of class MemoryBlockList.
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

#ifndef MEMORYBLOCKLIST_H
#define MEMORYBLOCKLIST_H

#ifndef USE_PCH
#include "debug.h"
#include <glibmm/refptr.h>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>
#endif

#include "Referenceable.h"

namespace util {

/** @brief A contiguous block in memory.
 *
 * The data block is allocated with malloc(3) and the
 * object is placed at the beginning of that allocated memory block.
 */
struct MemoryBlock : public Referenceable {
  private:
    // Do not allow construction, copying or assignment of this class.
    MemoryBlock(MemoryBlock const&) : Referenceable() { }
    MemoryBlock& operator=(MemoryBlock const&) { return *this; }

  /** @name Construction and destruction */
  //@{

  protected:
    // Except construction by MemoryBlockNode (which derives from this class).
    //! Constructor.
    MemoryBlock() { }

  public:
    /** @brief Operator new.
     *
     * Construct object (derived from MemoryBlock) with:
     * \code
     * Object* obj = new (block_size) Object(params);
     */
    void* operator new(size_t object_size, size_t block_size)
    {
      void* ptr = malloc(object_size + block_size);
      AllocTag(reinterpret_cast<char*>(ptr), "MemoryBlockNode (" << object_size << " bytes) + buffer allocation");
      return ptr;
    }

    //! Operator delete.
    void operator delete(void* ptr) { free(ptr); }

  //@}
};

/** @brief A node in a linked list of memory blocks.
 *
 * This class is derived from MemoryBlock.
 * The difference is that each node contains a reference to the next node.
 * As a result is that the very existence of a node keeps the chain of
 * following nodes alive.
 */
struct MemoryBlockNode : public MemoryBlock {
  private:
    size_t M_valid_bytes;			//!< Number of valid bytes in this block.
    Glib::RefPtr<MemoryBlockNode> M_next;	//!< Reference to the next block, if any.

    //! The offset from the 'this' pointer of this object to the start of the data block.
    static size_t const S_data_offset;

  private:
    // Constructor. See MemoryBlockNode::create.
    MemoryBlockNode(void) : M_valid_bytes(0) { }

  /** @name Creation and destruction */
  //@{

  public:
    /** @brief Allocate a new MemoryBlock with room for \a size bytes of data.
      *
      * The actual number of bytes allocated are sizeof(MemoryBlockNode) + \a size.
      */
    static Glib::RefPtr<MemoryBlockNode> create(size_t size)
    {
      return Glib::RefPtr<MemoryBlockNode>(new (size) MemoryBlockNode);
    }

  //@}

  private:
    friend class MemoryBlockList;
    // This function may only be called by class MemoryBlockList.

    /** @brief Set how many bytes where just written to \a new_block and append it to the chain.
     *
     * This block must be the last node in the chain.
     */
    void append(Glib::RefPtr<MemoryBlockNode>& new_block, size_t valid_bytes)
    {
      new_block->M_valid_bytes = valid_bytes;
      M_next.swap(new_block);
    }

  public:
  /** @name Accessors */
  //@{

    //! Return a pointer to the first data byte.
    char* block_begin(void) { return reinterpret_cast<char*>(this) + S_data_offset; }

    //! Return a const pointer to the first data byte.
    char const* block_begin(void) const { return reinterpret_cast<char const*>(this) + S_data_offset; }

    //! Return a pointer to one byte past the last valid byte in this block.
    char const* block_end(void) const { return reinterpret_cast<char const*>(this) + S_data_offset + M_valid_bytes; }

    //! Return the number of valid bytes in this block.
    size_t valid_bytes(void) const { return M_valid_bytes; }

    //! Return true if this is the last block in the chain.
    bool is_last_block(void) const { return !M_next; }

    //! Return a reference to the next block.
    Glib::RefPtr<MemoryBlockNode> const& next(void) const { return M_next; }

  //@}
};

struct MutexCondPair {
  Glib::Mutex mutex;
  Glib::Cond cond;
};

class MemoryBlockList;

/** @brief An immutable MemoryBlockList forward iterator for boost::spirit.
 *
 * However, this class is <em>not</em> Default Constructible.
 *
 * Also, this class also has no postincrement operator.
 * This is a deliberate choice; a good algorithm should use preincrement only for efficiency reasons.
 *
 * It is
 *
 * - <a href="http://www.sgi.com/tech/stl/Assignable.html">Assignable</a> (copy-constructor and <code>operator=</code>)
 * - <a href="http://www.sgi.com/tech/stl/EqualityComparable.html">Equality Comparable</a> (<code>operator==</code> and <code>operator!=</code>)
 * - Dereferenceable (<code>operator*</code> which returns an <em>immutable</em> reference)
 * - Incrementable (preincrement <code>operator++</code>)
 *
 * A forward iterator is essentially an <a href="http://www.sgi.com/tech/stl/InputIterator.html">Input Iterator</a>.
 * However, incrementing a forward iterator does not invalidate copies of the old value and it is guaranteed that,
 * if i and j are dereferenceable and i == j, then ++i == ++j. As a consequence of these two facts, it is possible
 * to pass through the same Forward Iterator twice. In other words, it is possible to use Forward Iterators (unlike
 * Input Iterators) in multipass algorithms.
 *
 * Of course, the past-the-end iterator is singular, non-dereferencable and non-incrementable.
 */
class MemoryBlockListIterator {
  public:
  /** @name Required iterator types */
  //@{

    typedef std::input_iterator_tag iterator_category;
    typedef char value_type;
    typedef ptrdiff_t difference_type;
    typedef char const* pointer;
    typedef char const& reference;

  //@}

  private:
    MemoryBlockList* const M_buffer;			//!< The linked list of blocks that this iterator belongs to.
    Glib::RefPtr<MemoryBlockNode const> M_block;	//!< Reference to block that this iterator points into.
    char const* M_ptr;					//!< Pointer to the current character inside that block.
    char const* M_block_end;				//!< Cache of pointer to the end of the block (MemoryBlockNode::block_end()).
    int M_processed_blocks;				//!< The number of blocks this iterator consumed.

  public:
  /** @name Assignable */
  //@{

    //! Copy-constructor.
    MemoryBlockListIterator(MemoryBlockListIterator const& iter) :
	M_buffer(iter.M_buffer),
        M_block(iter.M_block),
	M_ptr(iter.M_ptr),
	M_block_end(iter.M_block_end),
	M_processed_blocks(iter.M_processed_blocks)
	{ }

    //! Assignment operator (used for backtracking).
    MemoryBlockListIterator& operator=(MemoryBlockListIterator const& iter)
        {
	  assert(M_buffer == iter.M_buffer);
	  M_block = iter.M_block;
	  M_ptr = iter.M_ptr;
	  M_block_end = iter.M_block_end;
	  M_processed_blocks = iter.M_processed_blocks;
	}

  //@}

  /** @name Equality Comparable */
  //@{

    friend bool operator==(MemoryBlockListIterator const& a, MemoryBlockListIterator const& b) { return a.M_ptr == b.M_ptr; }
    friend bool operator!=(MemoryBlockListIterator const& a, MemoryBlockListIterator const& b) { return a.M_ptr != b.M_ptr; }

  //@}

  /** @name Dereferenceable */
  //@{

    value_type const& operator*(void) const { return *M_ptr; }

  //@}

  /** @name Incrementable */
  //@{

    //! Pre-increment operator.
    // This function is only called from the DatabaseSeekable::read_thread thread.
    MemoryBlockListIterator& operator++(void)
    {
      if (G_UNLIKELY(M_ptr == M_block_end))
	advance_to_next_block();
      else
	++M_ptr;
      return *this;
    }

  //@}

    //! Construct the past-the-end iterator.
    MemoryBlockListIterator(MemoryBlockList* buffer) : M_buffer(buffer), M_ptr(NULL), M_processed_blocks(0) { }

    //! Make this iterator point to the beginning of \a node.
    MemoryBlockListIterator& operator=(Glib::RefPtr<MemoryBlockNode const> const& node)
        {
	  assert(M_buffer);
	  M_block = node;
	  M_ptr = node->block_begin();
	  M_block_end = node->block_end() - 1;
	  M_processed_blocks = 0;
	}

  /** @name Accessors */
  //@{

    int processed_blocks(void) const { return M_processed_blocks; }

    MemoryBlockList* buffer(void) { return M_buffer; }

  //@}

  private:
    void advance_to_next_block(void);
};

/** A linked list of MemoryBlockNode objects.
 *
 * The idea of this linked list is as follows:
 *
 * A newly created list is entirely empty.
 * When the first block is appended, then M_begin is set to point to this first block.
 *
 * A different thread should only start to process this block once the
 * second block is appended: when
 */
class MemoryBlockList {
  public:
    typedef MemoryBlockListIterator iterator;
    typedef sigc::slot<void> SlotNeedMoreData;

    // The the minimum buffer size in blocks is 4.
    // We use 8 because that makes the inner loop use
    // a little let overhead related to buffer full checks.
    static int const S_max_blocks = 8;

  private:
    Glib::RefPtr<MemoryBlockNode> M_last_node;	//!< A pointer to the last node in the list.
    iterator M_begin;			  	//!< A pointer to the first node in the list. Read/write access by 'DatabaseSeekable::read_thread'.
    int M_appended_blocks;			//!< The number of blocks appended to this list. Read access by 'DatabaseSeekable::read_thread'.
    bool M_closed;				//!< False until the last block was appended and the read_thread is the only thread accessing this buffer.
    bool M_buffer_full;				//!< True while the main thread stops reading new blocks.
    Glib::Dispatcher M_need_more_data;		//!< Used to signal the main thread that more data can be appended to the buffer.
    SlotNeedMoreData M_slot_need_more_data;	//!< Pass the signal on the calling object.
    MutexCondPair M_more_data;			//!< Used for signaling that more data in the buffer is ready to be processed.

  public:
    MemoryBlockList(SlotNeedMoreData const& slot) : M_begin(this), M_appended_blocks(0), M_closed(false), M_buffer_full(false), M_slot_need_more_data(slot)
	{ M_need_more_data.connect(sigc::mem_fun(*this, &MemoryBlockList::need_more_data_callback)); }

    void append(Glib::RefPtr<MemoryBlockNode>& new_block, size_t valid_bytes)
    {
      int blocks;
      if (G_UNLIKELY(!M_last_node))
      {
        // If there is no last node, then this is the very first data block
	// being appended. In that case the 'read thread' is not running:
	// there wasn't any data yet in this buffer. Therefore, no locks
	// are necessary.
	new_block->M_valid_bytes = valid_bytes;
        M_last_node.swap(new_block);
        M_begin = M_last_node;
	Dout(dc::notice, "Appending FIRST block to the list. Number of blocks is now 1");
	assert(M_appended_blocks == 0);
	M_appended_blocks = 1;
	blocks = 1;
      }
      else
      {
        // The read thread is only running if there is more than one
	// block available for processing (it won't touch the last
	// block). Therefore, appending a new block doesn't need locking.
	M_last_node->append(new_block, valid_bytes);
	M_last_node = M_last_node->M_next;
	// Increment block counter and signal the read thread if we have more than 1 block.
	++M_appended_blocks;
	blocks = M_appended_blocks - M_begin.processed_blocks();
	Dout(dc::notice, "Appending a new block to the list. Number of unread blocks: " << blocks);
	// There should be always at least two blocks in the buffer now.
	M_more_data.cond.signal();
      }
      // Stop reading if there are eight or more blocks already buffered and waiting.
      if (blocks < S_max_blocks)
        M_slot_need_more_data();
      else
      {
	Dout(dc::notice, "The buffer is full!");
	M_more_data.mutex.lock();
	M_buffer_full = true;
	// Wake up 'read thread' just in case (special case).
	M_more_data.cond.signal();
	M_more_data.mutex.unlock();
      }
    }

    void close(void)
    {
      M_more_data.mutex.lock();
      M_closed = true;
      // Wake up 'read thread' just in case (special case).
      M_more_data.cond.signal();
      M_more_data.mutex.unlock();
    }

    bool closed(void) const { return M_closed; }
    bool full(void) const { return M_buffer_full; }

    iterator& begin(void)
    {
      // Wait until there is another block, so we can start to process the first block.
      while (!can_process_next_block(M_begin))
        wait_for_more_data(M_begin);
      return M_begin;
    }
    iterator end(void) const { return iterator(const_cast<MemoryBlockList*>(this)); }

    //! @brief Return the number of blocks that were appended to the linked list.
    int appended_blocks(void) const { return M_appended_blocks; }

    //! Returns ok if the read_thread is allowed to process the block that M_begin is pointing at.
    // This function is only called from DatabaseSeekable::read_thread.
    bool can_process_next_block(iterator const& iter)
    {
      // Do not process the last block while we're still writing to the
      // buffer because that would require using a mutex on a per character
      // basis, while not processing the last block allows us to not us
      // locking at all!
      return M_closed || M_appended_blocks - iter.processed_blocks() >= 2;
    }

    MutexCondPair& more_data(void) { return M_more_data; }

    Glib::Dispatcher& need_more_data(void) { return M_need_more_data; }

    void need_more_data_callback(void);

    void wait_for_more_data(iterator const& iter)
    {
      Dout(dc::notice, "Waiting for more data...");
continue_waiting:
#if 0
      timespec start_sleep_time_real, stop_sleep_time_real;
      timespec start_sleep_time_process, stop_sleep_time_process;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_sleep_time_process);
      clock_gettime(CLOCK_REALTIME, &start_sleep_time_real);
#endif
      M_more_data.mutex.lock();
      if (!M_buffer_full && !M_closed)
	M_more_data.cond.wait(M_more_data.mutex);
      M_more_data.mutex.unlock();
#if 0
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop_sleep_time_process);
      clock_gettime(CLOCK_REALTIME, &stop_sleep_time_real);
      stop_sleep_time_process -= start_sleep_time_process;
      wait_time_thread_process += stop_sleep_time_process;
      stop_sleep_time_real -= start_sleep_time_real;
      wait_time_thread_real += stop_sleep_time_real;
#endif
      // There is a small possibility that the main thread delayed
      // setting M_buffer_full so that this thread already processed
      // half of the buffer and got here even before M_buffer_full
      // is set. Therefore, this thread is also woken up again
      // when M_buffer_full is actually set.
      if (M_buffer_full)
      {
	// Ok that is "weird". Now waste some cpu for this special case.
	if (!can_process_next_block(iter))
	{
	  M_need_more_data.emit();
	  goto continue_waiting;
        }
      }
      Dout(dc::notice, "Got data!");
    }
};

} // namespace util

#endif	// MEMORYBLOCKLIST_H
