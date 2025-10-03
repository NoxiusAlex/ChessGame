#include "../include/Chess_Board.h"
#include "../include/DragonAI.h"

#include <limits>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <functional>

// Генераторы случайных чисел для ИИ
static std::mt19937& rng_ai(){ static std::random_device rd; static std::mt19937 g(rd()); return g; }
static int randint_ai(int a,int b){ std::uniform_int_distribution<int>d(a,b); return d(rng_ai()); }

// Конструктор DragonAI
DragonAI::DragonAI(bool aiWhite, int level) : m_aiWhite(aiWhite), m_level(level) {}

// Функция оценки стоимости фигур
int DragonAI::pieceValue(const std::string& t) const {
    if (t=="pawn") return 100;      // пешка
    if (t=="knight") return 320;    // конь
    if (t=="bishop") return 330;    // слон
    if (t=="rook") return 500;      // ладья
    if (t=="queen") return 900;     // ферзь
    if (t=="king") return 20000;    // король
    return 0;
}

// Позиционные таблицы для оценки расположения фигур

// Таблица для пешек - стимулирует продвижение пешек
const int pawnTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

// Таблица для коней - ценит центральные позиции
const int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Таблица для слонов - предпочитает диагонали и центр
const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Таблица для королей - безопасность в эндшпиле
const int kingTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

// Получение позиционной оценки для фигуры
int DragonAI::getPositionalScore(int x, int y, const std::string& pieceType, bool isWhite) const {
    // Инвертируем координаты для черных фигур
    int index = isWhite ? y * 8 + x : (7 - y) * 8 + (7 - x);
    
    if (pieceType == "pawn") return pawnTable[index];
    if (pieceType == "knight") return knightTable[index];
    if (pieceType == "bishop") return bishopTable[index];
    if (pieceType == "king") return kingTable[index];
    
    return 0;
}

// Основная функция оценки позиции
int DragonAI::evaluate(const Chess_Board& board) const {
    int material = 0;    // материальный баланс
    int positional = 0;  // позиционная оценка
    int mobility = 0;    // мобильность фигур
    int kingSafety = 0;  // безопасность короля
    
    // Обходим все фигуры на доске
    for (auto &kv : board.board) {
        auto *p = kv.second.get(); 
        if (!p) continue;
        
        int x = kv.first.first;
        int y = kv.first.second;
        bool isWhite = p->collor;
        std::string type = p->type();
        
        // Материальная оценка
        int pieceVal = pieceValue(type);
        material += (isWhite ? pieceVal : -pieceVal);
        
        // Позиционная оценка
        positional += (isWhite ? 1 : -1) * getPositionalScore(x, y, type, isWhite);
        
        // Оценка мобильности (количество возможных ходов)
        auto moves = board.getLegalMoves(kv.first);
        mobility += (isWhite ? 1 : -1) * static_cast<int>(moves.size()) * 2;
        
        // Оценка безопасности короля
        if (type == "king") {
            int kingSafetyPenalty = 0;
            if (y == (isWhite ? 0 : 7)) kingSafetyPenalty -= 20; // штраф за короля на краю
            kingSafety += (isWhite ? -1 : 1) * kingSafetyPenalty;
        }
    }
    
    // Итоговая оценка с весовыми коэффициентами
    int totalScore = material + positional/2 + mobility/3 + kingSafety;
    return m_aiWhite ? totalScore : -totalScore;
}

// Транспозиционная таблица для кэширования оценок позиций
struct TranspositionEntry {
    uint64_t hash;    // хэш позиции
    int depth;        // глубина поиска
    int score;        // оценка позиции
    int flag;         // тип оценки (точная, нижняя граница, верхняя граница)
};

static std::unordered_map<uint64_t, TranspositionEntry> transpositionTable;

// Вычисление хэша позиции для транспозиционной таблицы
uint64_t computePositionHash(const Chess_Board& board) {
    uint64_t hash = 0;
    for (auto &kv : board.board) {
        if (!kv.second) continue;
        int x = kv.first.first;
        int y = kv.first.second;
        std::string type = kv.second->type();
        bool color = kv.second->collor;
        
        // Комбинируем координаты, тип и цвет фигуры в хэш
        hash ^= (x << 0) | (y << 3) | (type[0] << 6) | (color ? 1 << 9 : 0);
    }
    return hash;
}

// Алгоритм минимакса с альфа-бета отсечением
int DragonAI::minimax(Chess_Board& board, int depth, int alpha, int beta, bool maximizing) {
    // Если достигнута максимальная глубина, переходим к поиску тихих позиций
    if (depth == 0) return quiescenceSearch(board, alpha, beta, maximizing);
    
    // Проверка транспозиционной таблицы
    uint64_t hash = computePositionHash(board);
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        if (it->second.flag == 0) return it->second.score;        // точное значение
        if (it->second.flag == 1 && it->second.score >= beta) return beta; // нижняя граница
        if (it->second.flag == 2 && it->second.score <= alpha) return alpha; // верхняя граница
    }
    
    // Проверка мата
    bool side = maximizing ? m_aiWhite : !m_aiWhite;
    if (board.isCheckmate(side)) return maximizing ? -100000 : 100000;
    
    // Получение и сортировка ходов
    auto moves = getOrderedMoves(board, side);
    if (moves.empty()) return evaluate(board);
    
    int best = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    int originalAlpha = alpha;
    
    // Рекурсивный перебор ходов
    for (auto &move : moves) {
        Chess_Board copy;
        for (auto &c : board.board) copy.addPiece(c.first, c.second->clone());
        if (!copy.movePiece(move.from, move.to)) continue;
        
        int val = minimax(copy, depth - 1, alpha, beta, !maximizing);
        
        if (maximizing) {
            if (val > best) best = val;
            if (val > alpha) alpha = val;
        } else {
            if (val < best) best = val;
            if (val < beta) beta = val;
        }
        
        // Альфа-бета отсечение
        if (beta <= alpha) break;
    }
    
    // Сохранение результата в транспозиционной таблице
    TranspositionEntry entry;
    entry.hash = hash;
    entry.depth = depth;
    entry.score = best;
    entry.flag = (best <= originalAlpha) ? 2 : (best >= beta) ? 1 : 0;
    transpositionTable[hash] = entry;
    
    return best;
}

// Поиск тихих позиций (только взятия)
int DragonAI::quiescenceSearch(Chess_Board& board, int alpha, int beta, bool maximizing) {
    // Статическая оценка текущей позиции
    int standPat = evaluate(board);
    
    if (maximizing) {
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    } else {
        if (standPat <= alpha) return alpha;
        if (standPat < beta) beta = standPat;
    }
    
    // Получение всех взятий
    auto captures = getCaptureMoves(board, maximizing ? m_aiWhite : !m_aiWhite);
    
    // Оценка взятий
    for (auto &move : captures) {
        Chess_Board copy;
        for (auto &c : board.board) copy.addPiece(c.first, c.second->clone());
        if (!copy.movePiece(move.from, move.to)) continue;
        
        int score = quiescenceSearch(copy, alpha, beta, !maximizing);
        
        if (maximizing) {
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        } else {
            if (score <= alpha) return alpha;
            if (score < beta) beta = score;
        }
    }
    
    return maximizing ? alpha : beta;
}

// Получение упорядоченных ходов (сортировка по эвристике)
std::vector<AIMove> DragonAI::getOrderedMoves(const Chess_Board& board, bool side) const {
    std::vector<std::pair<AIMove, int>> movesWithScore;
    
    for (auto &kv : board.board) {
        if (!kv.second || kv.second->collor != side) continue;
        auto moves = board.getLegalMoves(kv.first);
        
        for (auto &to : moves) {
            AIMove move = {kv.first, to};
            int score = 0;
            
            // Оценка хода: взятия оцениваются выше
            auto* targetPiece = board.getPiece(to);
            if (targetPiece) {
                // Разность стоимости взятой фигуры и своей
                score = pieceValue(targetPiece->type()) * 10 - pieceValue(kv.second->type());
            } else {
                // Позиционное улучшение
                score = getPositionalScore(to.first, to.second, kv.second->type(), side) - 
                       getPositionalScore(kv.first.first, kv.first.second, kv.second->type(), side);
            }
            
            movesWithScore.push_back({move, score});
        }
    }
    
    // Сортировка по убыванию оценки
    std::sort(movesWithScore.begin(), movesWithScore.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<AIMove> result;
    for (auto& pair : movesWithScore) {
        result.push_back(pair.first);
    }
    
    return result;
}

// Получение только взятий
std::vector<AIMove> DragonAI::getCaptureMoves(const Chess_Board& board, bool side) const {
    std::vector<AIMove> captures;
    
    for (auto &kv : board.board) {
        if (!kv.second || kv.second->collor != side) continue;
        auto moves = board.getLegalMoves(kv.first);
        
        for (auto &to : moves) {
            if (board.getPiece(to)) {
                captures.push_back({kv.first, to});
            }
        }
    }
    
    return captures;
}

// Основная функция выбора хода
AIMove DragonAI::chooseMove(Chess_Board& board) {
    // Периодическая очистка транспозиционной таблицы
    static int moveCounter = 0;
    if (++moveCounter % 1000 == 0) {
        transpositionTable.clear();
    }
    
    // Получение всех возможных ходов
    std::vector<AIMove> allMoves;
    for (auto &kv : board.board) {
        if (!kv.second || kv.second->collor != m_aiWhite) continue;
        auto moves = board.getLegalMoves(kv.first);
        for (auto &to : moves) allMoves.push_back({kv.first, to});
    }
    
    if (allMoves.empty()) return { {-1,-1}, {-1,-1} };
    
    // Уровень 0: случайный ход
    if (m_level <= 0) {
        return allMoves[randint_ai(0, allMoves.size() - 1)];
    }
    
    // Уровень 1: жадный алгоритм (один ход вперед)
    if (m_level == 1) {
        return getBestImmediateMove(board);
    }
    
    // Уровни 2+: минимакс с увеличивающейся глубиной
    int searchDepth = std::min(m_level + 1, 6);
    
    int bestVal = std::numeric_limits<int>::min();
    AIMove bestMove = allMoves[0];
    
    // Итеративное углубление
    for (int depth = 1; depth <= searchDepth; depth++) {
        int currentBestVal = std::numeric_limits<int>::min();
        AIMove currentBestMove = bestMove;
        
        for (auto &mv : allMoves) {
            Chess_Board copy;
            for (auto &c : board.board) copy.addPiece(c.first, c.second->clone());
            if (!copy.movePiece(mv.from, mv.to)) continue;
            
            int val = minimax(copy, depth - 1, std::numeric_limits<int>::min(), 
                            std::numeric_limits<int>::max(), false);
            
            if (val > currentBestVal) {
                currentBestVal = val;
                currentBestMove = mv;
            }
        }
        
        // Обновление лучшего хода, если найдено значительное улучшение
        if (currentBestVal > bestVal + 50 || depth == 1) {
            bestVal = currentBestVal;
            bestMove = currentBestMove;
        }
        
        // Прерывание если найден выигрывающий ход
        if (bestVal > 50000) break;
    }
    
    return bestMove;
}

// Жадный алгоритм выбора хода (оценка только на один ход вперед)
AIMove DragonAI::getBestImmediateMove(Chess_Board& board) const {
    std::vector<AIMove> allMoves;
    for (auto &kv : board.board) {
        if (!kv.second || kv.second->collor != m_aiWhite) continue;
        auto moves = board.getLegalMoves(kv.first);
        for (auto &to : moves) allMoves.push_back({kv.first, to});
    }
    
    int bestScore = std::numeric_limits<int>::min();
    AIMove bestMove = allMoves[0];
    
    for (auto &mv : allMoves) {
        Chess_Board copy;
        for (auto &c : board.board) copy.addPiece(c.first, c.second->clone());
        if (!copy.movePiece(mv.from, mv.to)) continue;
        
        int score = evaluate(copy);
        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }
    
    return bestMove;
}
