#include "../include/Chess_Board.h"
#include "../include/Pawn.h"
#include "../include/Rook.h"
#include "../include/Bishop.h"
#include "../include/Queen.h"
#include "../include/King.h"
#include "../include/Horse.h"

Chess_Piece* Chess_Board::getPiece(std::pair<int,int> coord) {
    auto it = board.find(coord);
    if (it == board.end()) return nullptr;
    return it->second.get();
}
const Chess_Piece* Chess_Board::getPiece(std::pair<int,int> coord) const {
    auto it = board.find(coord);
    if (it == board.end()) return nullptr;
    return it->second.get();
}

bool Chess_Board::movePiece(std::pair<int,int> from, std::pair<int,int> to) {
    auto it = board.find(from);
    if (it == board.end()) return false;

    Chess_Piece* piece = it->second.get();
    if (!piece) return false;

    // Список легальных ходов (с учетом блокировок, своих фигур и шаха)
    auto moves = getLegalMoves(from);
    bool allowed = false;
    for (auto &m : moves) {
        if (m == to) { allowed = true; break; }
    }
    if (!allowed) return false;

    // Если на целевой клетке есть фигура — "съедаем" её
    auto cap = board.find(to);
    if (cap != board.end()) {
        // captured piece will be destroyed when unique_ptr goes out of scope
        board.erase(cap);
    }

    // Перемещаем фигуру: обновляем её текущую клетку и переносим unique_ptr
    it->second->cur_cell = to;
    board[to] = std::move(it->second);
    board.erase(it);
    // После каждого хода проверим превращение пешек
    handlePromotion();
    return true;
}

void Chess_Board::setupBoard() {
    board.clear();

    // Белые
    addPiece({0,0}, std::make_unique<Rook>(std::pair<int,int>{0,0}, true));
    addPiece({7,0}, std::make_unique<Rook>(std::pair<int,int>{7,0}, true));
    addPiece({1,0}, std::make_unique<Horse>(std::pair<int,int>{1,0}, true));
    addPiece({6,0}, std::make_unique<Horse>(std::pair<int,int>{6,0}, true));
    addPiece({2,0}, std::make_unique<Bishop>(std::pair<int,int>{2,0}, true));
    addPiece({5,0}, std::make_unique<Bishop>(std::pair<int,int>{5,0}, true));
    addPiece({3,0}, std::make_unique<Queen>(std::pair<int,int>{3,0}, true));
    addPiece({4,0}, std::make_unique<King>(std::pair<int,int>{4,0}, true));
    for(int i=0;i<8;i++) addPiece({i,1}, std::make_unique<Pawn>(std::pair<int,int>{i,1}, true));

    // Черные
    addPiece({0,7}, std::make_unique<Rook>(std::pair<int,int>{0,7}, false));
    addPiece({7,7}, std::make_unique<Rook>(std::pair<int,int>{7,7}, false));
    addPiece({1,7}, std::make_unique<Horse>(std::pair<int,int>{1,7}, false));
    addPiece({6,7}, std::make_unique<Horse>(std::pair<int,int>{6,7}, false));
    addPiece({2,7}, std::make_unique<Bishop>(std::pair<int,int>{2,7}, false));
    addPiece({5,7}, std::make_unique<Bishop>(std::pair<int,int>{5,7}, false));
    addPiece({3,7}, std::make_unique<Queen>(std::pair<int,int>{3,7}, false));
    addPiece({4,7}, std::make_unique<King>(std::pair<int,int>{4,7}, false));
    for(int i=0;i<8;i++) addPiece({i,6}, std::make_unique<Pawn>(std::pair<int,int>{i,6}, false));
}

bool Chess_Board::isCheck(bool white){ 
    auto kingPos = getKingPos(white);
    if (kingPos.first == -1) return false;
    // Если любая фигура противника может пойти на клетку короля по псевдо-правилам — шах
    for (auto &kv : board){
        Chess_Piece* p = kv.second.get();
        if (!p || p->collor == white) continue;
        if (isPseudoLegalMove(kv.first, kingPos)) return true;
    }
    return false; 
}
std::pair<int,int> Chess_Board::getKingPos(bool white){ 
    for(auto &p: board){
        if(p.second->type() == "king" && p.second->collor == white)
            return p.first;
    }
    return {-1,-1};
}

void Chess_Board::handlePromotion(){
    // Простой вариант: превращаем любую пешку, достигшую последней горизонтали, в ферзя
    std::vector<std::pair<int,int>> toPromote;
    for (auto &kv : board){
        if (kv.second && kv.second->type()=="pawn"){
            auto y = kv.first.second;
            bool white = kv.second->collor;
            if ((white && y==7) || (!white && y==0)){
                toPromote.push_back(kv.first);
            }
        }
    }
    for (auto &pos : toPromote){
        bool white = board[pos]->collor;
        board[pos] = std::make_unique<Queen>(pos, white);
    }
}

bool Chess_Board::isInside(int x, int y) const { return x>=0 && x<8 && y>=0 && y<8; }
bool Chess_Board::isEmpty(const std::pair<int,int>& p) const { return board.find(p)==board.end(); }
bool Chess_Board::isEnemyAt(const std::pair<int,int>& p, bool white) const { 
    auto it = board.find(p);
    return it!=board.end() && it->second && it->second->collor != white;
}
bool Chess_Board::isOwnAt(const std::pair<int,int>& p, bool white) const {
    auto it = board.find(p);
    return it!=board.end() && it->second && it->second->collor == white;
}

bool Chess_Board::isPathClearLine(const std::pair<int,int>& from, const std::pair<int,int>& to) const {
    int dx = (to.first > from.first) ? 1 : (to.first < from.first ? -1 : 0);
    int dy = (to.second > from.second) ? 1 : (to.second < from.second ? -1 : 0);
    if (dx!=0 && dy!=0) return false; // не по прямой
    int x = from.first + dx, y = from.second + dy;
    while (x!=to.first || y!=to.second) {
        if (!isEmpty({x,y})) return false;
        x += dx; y += dy;
    }
    return true;
}

bool Chess_Board::isPathClearDiag(const std::pair<int,int>& from, const std::pair<int,int>& to) const {
    int dx = to.first - from.first;
    int dy = to.second - from.second;
    if (std::abs(dx) != std::abs(dy)) return false; // не диагональ
    int stepx = (dx>0)?1:-1;
    int stepy = (dy>0)?1:-1;
    int x = from.first + stepx, y = from.second + stepy;
    while (x!=to.first || y!=to.second) {
        if (!isEmpty({x,y})) return false;
        x += stepx; y += stepy;
    }
    return true;
}

bool Chess_Board::isPseudoLegalMove(const std::pair<int,int>& from, const std::pair<int,int>& to) const {
    auto it = board.find(from);
    if (it==board.end() || !it->second) return false;
    Chess_Piece* p = it->second.get();
    bool white = p->collor;
    if (!isInside(to.first, to.second)) return false;
    if (isOwnAt(to, white)) return false; // нельзя бить свою

    std::string t = p->type();
    int fx = from.first, fy = from.second, tx = to.first, ty = to.second;
    int dx = tx - fx, dy = ty - fy;

    if (t == "pawn") {
        int dir = white ? 1 : -1;
        // ход вперед
        if (dx==0 && dy==dir && isEmpty(to)) return true;
        // первый ход на 2 клетки
        if (dx==0 && dy==2*dir && ((white && fy==1) || (!white && fy==6)) && isEmpty({fx, fy+dir}) && isEmpty(to)) return true;
        // взятие по диагонали
        if (std::abs(dx)==1 && dy==dir && isEnemyAt(to, white)) return true;
        return false;
    }
    if (t == "rook") {
        if ((fx==tx || fy==ty) && isPathClearLine(from, to)) return true;
        return false;
    }
    if (t == "bishop") {
        if (std::abs(dx)==std::abs(dy) && isPathClearDiag(from, to)) return true;
        return false;
    }
    if (t == "queen") {
        if ((fx==tx || fy==ty) && isPathClearLine(from, to)) return true;
        if (std::abs(dx)==std::abs(dy) && isPathClearDiag(from, to)) return true;
        return false;
    }
    if (t == "knight") {
        if ((std::abs(dx)==1 && std::abs(dy)==2) || (std::abs(dx)==2 && std::abs(dy)==1)) return true;
        return false;
    }
    if (t == "king") {
        if (std::max(std::abs(dx), std::abs(dy))==1) return true; // рокировка не реализована
        return false;
    }
    return false;
}

bool Chess_Board::wouldLeaveKingInCheck(const std::pair<int,int>& from, const std::pair<int,int>& to) const {
    // смоделируем ход на копии доски
    Chess_Board copy;
    for (auto &kv : board) {
        copy.addPiece(kv.first, kv.second->clone());
    }
    auto it = copy.board.find(from);
    if (it==copy.board.end()) return true;
    // выполнить ход на копии
    auto cap = copy.board.find(to);
    if (cap!=copy.board.end()) copy.board.erase(cap);
    it->second->cur_cell = to;
    copy.board[to] = std::move(it->second);
    copy.board.erase(it);

    bool side = getPiece(from)->collor;
    return copy.isCheck(side);
}

std::vector<std::pair<int,int>> Chess_Board::getLegalMoves(std::pair<int,int> from) const {
    std::vector<std::pair<int,int>> res;
    auto it = board.find(from);
    if (it==board.end() || !it->second) return res;
    Chess_Piece* p = it->second.get();
    // перебираем все клетки 8x8 и фильтруем через псевдолегальные + проверка на шах своему
    for (int x=0;x<8;x++){
        for (int y=0;y<8;y++){
            std::pair<int,int> to{x,y};
            if (!isPseudoLegalMove(from, to)) continue;
            // смоделировать и проверить не будет ли шаха
            Chess_Board copy;
            for (auto &kv : board) copy.addPiece(kv.first, kv.second->clone());
            auto it2 = copy.board.find(from);
            auto cap = copy.board.find(to);
            if (cap!=copy.board.end()) copy.board.erase(cap);
            it2->second->cur_cell = to;
            copy.board[to] = std::move(it2->second);
            copy.board.erase(it2);
            if (!copy.isCheck(p->collor)) res.push_back(to);
        }
    }
    return res;
}

bool Chess_Board::hasAnyLegalMove(bool white){
    for (auto &kv : board){
        if (kv.second && kv.second->collor == white) {
            auto moves = getLegalMoves(kv.first);
            if (!moves.empty()) return true;
        }
    }
    return false;
}

bool Chess_Board::isCheckmate(bool white){
    if (!isCheck(white)) return false;
    return !hasAnyLegalMove(white);
}
