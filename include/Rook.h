#pragma once
#include "Chess_Piece.h"
#include <vector>

class Rook : public Chess_Piece {
public:
    Rook(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        int x = cur_cell.first, y = cur_cell.second;
        for(int i=0;i<8;i++){
            if(i!=x) moves.push_back({i,y});
            if(i!=y) moves.push_back({x,i});
        }
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<Rook>(*this);
    }
    
    std::string type() const { return "rook"; }
};
