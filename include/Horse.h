#pragma once
#include "Chess_Piece.h"
#include <vector>

class Horse : public Chess_Piece {
public:
    Horse(std::pair<int,int> cord, bool collor_) : Chess_Piece(cord, collor_) {}
    
    std::vector<std::pair<int,int>> getAvailableMoves() const override {
        std::vector<std::pair<int,int>> moves;
        int x = cur_cell.first, y = cur_cell.second;
        int dx[] = {-2,-1,1,2};
        int dy[] = {-2,-1,1,2};
        for(int i=0;i<4;i++){
            for(int j=0;j<4;j++){
                if(abs(dx[i])!=abs(dy[j])){
                    int nx = x+dx[i], ny = y+dy[j];
                    if(nx>=0 && nx<8 && ny>=0 && ny<8) moves.push_back({nx,ny});
                }
            }
        }
        return moves;
    }
    
    std::unique_ptr<Chess_Piece> clone() const override {
        return std::make_unique<Horse>(*this);
    }
    
    std::string type() const { return "knight"; }
};
