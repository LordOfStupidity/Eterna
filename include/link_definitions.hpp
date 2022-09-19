#ifndef _LINK_DEFINITIONS_HPP
#define _LINK_DEFINITIONS_HPP

#include <cstddef>
#include <cstdint>

extern uint64_t KERNEL_PHYSICAL;
extern uint64_t KERNEL_VIRTUAL;
extern uint64_t KERNEL_START;
extern uint64_t KERNEL_END;
extern uint64_t TEXT_START;
extern uint64_t TEXT_END;
extern uint64_t DATA_START;
extern uint64_t DATA_END;
extern uint64_t READ_ONLY_DATA_START;
extern uint64_t READ_ONLY_DATA_END;
extern uint64_t BLOCK_STARTING_SYMBOLS_START;
extern uint64_t BLOCK_STARTING_SYMBOLS_END;

#define V2P(addr) ((uint64_t)(addr) - (uint64_t)&KERNEL_VIRTUAL)
#define P2V(addr) ((uint64_t)(addr) + (uint64_t)&KERNEL_VIRTUAL)

#endif  // !_LINK_DEFINITIONS_HPP