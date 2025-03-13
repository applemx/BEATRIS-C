#pragma once
#include "board.hpp"
#include <string>
#include <sstream>

namespace renderer {
  template <unsigned W, unsigned H>
  std::string draw(const reachability::board_t<W,H>& board) {
      return to_string(board);
  }
}