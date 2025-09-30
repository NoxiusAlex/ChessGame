#pragma once
#include "Chess_Piece.h"
#include "Rook.h"
#include "Bishop.h"

class Queen : public Chess_Piece {
public:
    Queen(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        Rook r(cur_cell, collor);
        Bishop b(cur_cell, collor);
        auto rm = r.getAvailableMoves();
        auto bm = b.getAvailableMoves();
        moves.insert(moves.end(), rm.begin(), rm.end());
        moves.insert(moves.end(), bm.begin(), bm.end());
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<Queen>(*this);
    }
    
    std::string type() const { return "queen"; }
};
