#include "SlabAllocatorEngine.hpp"
#include "../BitOperations.hpp"

#include <cassert>


namespace exc {


SlabAllocatorEngine::SlabAllocatorEngine(size_t poolSize)
	: m_poolSize(poolSize), m_blocks((poolSize + SlotsPerBlock - 1) / SlotsPerBlock)
{
	Reset();
}


size_t SlabAllocatorEngine::Allocate() {
	// check if not full
	if (m_first != nullptr) {
		size_t mask = m_first->slotOccupancy;
		mask = ~mask;
		int index = CountTrailingZeros(mask);

		if (index >= 0) {
			bool correct = !BitTestAndSet(m_first->slotOccupancy, index);
			assert(correct);
			return IndexOf(m_first) * SlotsPerBlock + index;
		}
		else {
			m_first = m_first->nextBlockIndex < m_blocks.size() ? &m_blocks[m_first->nextBlockIndex] : nullptr;
			return Allocate();
		}
	}
	else {
		throw std::bad_alloc();
	}
}


void SlabAllocatorEngine::Deallocate(size_t index) {
	Block* block;
	int inBlockIndex;
	BlockOf(index, block, inBlockIndex);

	size_t prevMask = block->slotOccupancy;
	bool correct = BitTestAndClear(block->slotOccupancy, inBlockIndex);
	assert(correct);
	if (prevMask == ~size_t(0) && block != m_first) {
		block->nextBlockIndex = m_first != nullptr ? IndexOf(m_first) : m_blocks.size();
		m_first = block;
	}
}


void SlabAllocatorEngine::Resize(size_t newPoolSize) {
	// allocate space for new block list
	std::vector<Block> newBlocks((newPoolSize + SlotsPerBlock - 1) / SlotsPerBlock);

	// copy old elements
	intptr_t indexOfLastFree = -1;
	for (size_t i = 0; i < m_blocks.size() && i < newBlocks.size(); ++i) {
		newBlocks[i].slotOccupancy = m_blocks[i].slotOccupancy;
	}
	// clear new elements
	for (size_t i = m_blocks.size(); i < newBlocks.size(); ++i) {
		newBlocks[i].slotOccupancy = 0;
	}

	// unlock slots of the OLD last block
	if (newPoolSize > m_poolSize) {
		auto last = &newBlocks[m_blocks.size() - 1];
		int numLastSlots = (m_poolSize % SlotsPerBlock);
		last->slotOccupancy &= ~size_t(0) >> (sizeof(size_t) * 8 - numLastSlots);
	}

	// lock last slots of the NEW last block
	{
		auto last = &newBlocks[newBlocks.size() - 1];
		int numLastSlots = (newPoolSize % SlotsPerBlock);
		last->slotOccupancy = ~size_t(0) << numLastSlots;
	}

	// create free blocks chain
	intptr_t prevFree = std::numeric_limits<intptr_t>::max();
	for (intptr_t i = newBlocks.size() - 1; i >= 0; --i) {
		if (newBlocks[i].slotOccupancy < ~size_t(0)) {
			newBlocks[i].nextBlockIndex = prevFree;
			prevFree = i;
		}
	}
	
	// commit new storage space
	m_blocks = std::move(newBlocks);
	m_first = prevFree < (intptr_t)m_blocks.size() ? m_blocks.data() + prevFree : nullptr;
}


void SlabAllocatorEngine::Reset() {
	// chain all blocks into free list and reset occupancy
	for (size_t idx = 0; idx < m_blocks.size(); ++idx) {
		m_blocks[idx].nextBlockIndex = idx + 1;
		m_blocks[idx].slotOccupancy = 0;
	}

	// get first and last blocks
	m_first = &m_blocks[0];
	auto last = &m_blocks[m_blocks.size() - 1];

	// mask out unused part of last block
	int numLastSlots = (m_poolSize % SlotsPerBlock);
	if (numLastSlots != 0) {
		last->slotOccupancy = ~size_t(0) << numLastSlots;
	}
}


} // namespace exc