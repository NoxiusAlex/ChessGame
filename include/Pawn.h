
#pragma once
#include "Chess_Piece.h"
#include <vector>

class Pawn : public Chess_Piece {
public:
    Pawn(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        int dir = collor ? 1 : -1;
        int x = cur_cell.first, y = cur_cell.second + dir;
        if(y>=0 && y<8) moves.push_back({x,y});
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<Pawn>(*this);
    }
    
    std::string type() const { return "pawn"; }
};
