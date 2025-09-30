#pragma once
#include "Chess_Piece.h"
#include <vector>

class King : public Chess_Piece {
public:
    King(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        int dx[] = {-1,0,1};
        int dy[] = {-1,0,1};
        int x = cur_cell.first, y = cur_cell.second;
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                if(dx[i]==0 && dy[j]==0) continue;
                int nx = x+dx[i], ny = y+dy[j];
                if(nx>=0 && nx<8 && ny>=0 && ny<8) moves.push_back({nx,ny});
            }
        }
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<King>(*this);
    }
    
    std::string type() const override { return "king"; }
};
