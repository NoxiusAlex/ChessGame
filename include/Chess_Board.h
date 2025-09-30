#pragma once
#include "Chess_Piece.h"
#include <map>
#include <memory>
#include <iostream>

class Chess_Board {
public:
    std::map<std::pair<int,int>, std::unique_ptr<Chess_Piece>> board;

    void addPiece(std::pair<int,int> coord, std::unique_ptr<Chess_Piece> piece){
        board[coord] = std::move(piece);
    }

    void printMoves(std::pair<int,int> coord){
        if(board.find(coord)==board.end()) return;
        auto moves = board[coord]->getAvailableMoves();
        std::cout << "Moves for piece at (" << coord.first << "," << coord.second << "):\n";
        for(auto &m : moves) std::cout << "(" << m.first << "," << m.second << ")\n";
    }

    void setupBoard(); // реализация в cpp

    // Возвращает сырой указатель на фигуру на клетке или nullptr
    Chess_Piece* getPiece(std::pair<int,int> coord);
    const Chess_Piece* getPiece(std::pair<int,int> coord) const;

    // Пытается переместить фигуру. Возвращает true, если ход выполнен
    bool movePiece(std::pair<int,int> from, std::pair<int,int> to);

    // Легальные ходы для фигуры на клетке (с учетом блокировок, своих фигур и шаха)
    std::vector<std::pair<int,int>> getLegalMoves(std::pair<int,int> from) const;

    // Проверка шаха и получение позиции короля
    bool isCheck(bool white);
    std::pair<int,int> getKingPos(bool white);

    // Проверка мата/наличия ходов для стороны
    bool isCheckmate(bool white);
    bool hasAnyLegalMove(bool white);

    // Превращение пешки: если пешка достигла последней горизонтали, превращаем в ферзя
    void handlePromotion();

private:
    bool isInside(int x, int y) const;
    bool isEmpty(const std::pair<int,int>& p) const;
    bool isEnemyAt(const std::pair<int,int>& p, bool white) const;
    bool isOwnAt(const std::pair<int,int>& p, bool white) const;
    bool isPathClearLine(const std::pair<int,int>& from, const std::pair<int,int>& to) const; // по прямой
    bool isPathClearDiag(const std::pair<int,int>& from, const std::pair<int,int>& to) const;  // по диагонали
    bool isPseudoLegalMove(const std::pair<int,int>& from, const std::pair<int,int>& to) const; // без учета шаха
    bool wouldLeaveKingInCheck(const std::pair<int,int>& from, const std::pair<int,int>& to) const;
};
