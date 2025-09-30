#pragma once
#include <memory>
#include <utility>
#include <vector>

class Chess_Piece {
public:
  std::pair<int, int> cur_cell;
  bool alive;
  bool collor; // true = white, false = black

  Chess_Piece(std::pair<int, int> cord, bool collor_)
      : cur_cell(cord), alive(true), collor(collor_) {}
  virtual ~Chess_Piece() = default;

  virtual std::vector<std::pair<int, int>> getAvailableMoves() const = 0;
  virtual std::unique_ptr<Chess_Piece> clone() const = 0;
  virtual std::string type() const = 0;
};
