#ifndef DRAGONAI_H
#define DRAGONAI_H

#include <vector>
#include <string>
#include "Chess_Board.h"

struct AIMove {
    std::pair<int,int> from;
    std::pair<int,int> to;
};

class DragonAI {
public:
    DragonAI(bool aiWhite, int level);

    int pieceValue(const std::string& t) const;
    int getPositionalScore(int x, int y, const std::string& pieceType, bool isWhite) const;
    int evaluate(const Chess_Board& board) const;
    int minimax(Chess_Board& board, int depth, int alpha, int beta, bool maximizing);
    int quiescenceSearch(Chess_Board& board, int alpha, int beta, bool maximizing);
    std::vector<AIMove> getOrderedMoves(const Chess_Board& board, bool side) const;
    std::vector<AIMove> getCaptureMoves(const Chess_Board& board, bool side) const;
    AIMove chooseMove(Chess_Board& board);
    AIMove getBestImmediateMove(Chess_Board& board) const;

private:
    bool m_aiWhite;
    int m_level;
};

#endif

