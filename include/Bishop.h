#pragma once
#include "Chess_Piece.h"
#include <vector>

class Bishop : public Chess_Piece {
public:
    Bishop(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        int x = cur_cell.first, y = cur_cell.second;
        for(int dx=-7; dx<=7; dx++){
            if(dx==0) continue;
            if(x+dx>=0 && x+dx<8 && y+dx>=0 && y+dx<8) moves.push_back({x+dx,y+dx});
            if(x+dx>=0 && x+dx<8 && y-dx>=0 && y-dx<8) moves.push_back({x+dx,y-dx});
        }
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<Bishop>(*this);
    }
    
    std::string type() const  { return "bishop"; }
};
