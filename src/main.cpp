#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include "../include/Chess_Board.h"
#include "../include/Pawn.h"
#include "../include/Rook.h"
#include "../include/Bishop.h"
#include "../include/Queen.h"
#include "../include/King.h"
#include "../include/Horse.h"
#include "../include/MathProblem.h"
#include "../include/DragonAI.h"

// Константы для размера доски и отступа
const int SIZE = 8; // Размер шахматной доски 8x8
const int BOARD_LEFT = 320; // Отступ для левого меню

// Структура для анимации перемещения фигуры
struct Animation {
    sf::Vector2f startPos;
    sf::Vector2f endPos;
    sf::Vector2f currentPos;
    std::string pieceKey;
    std::pair<int, int> fromCoord;
    std::pair<int, int> toCoord;
    float progress;
    float duration;
    bool active;
    
    Animation(std::pair<int, int> from, std::pair<int, int> to, 
              sf::Vector2f start, sf::Vector2f end, const std::string& key, float dur = 0.5f)
        : fromCoord(from), toCoord(to), startPos(start), endPos(end), currentPos(start), 
          pieceKey(key), progress(0.0f), duration(dur), active(true) {}
    
    void update(float dt) {
        if (!active) return;
        
        progress += dt / duration;
        if (progress >= 1.0f) {
            progress = 1.0f;
            active = false;
        }
        
        // Плавное движение с ease-out
        float t = progress;
        t = 1.0f - (1.0f - t) * (1.0f - t); // ease-out quadratic
        currentPos = startPos + (endPos - startPos) * t;
    }
};

// Функция преобразования координат доски в экранные координаты
sf::Vector2f toScreenCoords(std::pair<int,int> pos, float cellW, float cellH, float boardOffsetX, float boardOffsetY) {
    return sf::Vector2f(boardOffsetX + pos.first * cellW, boardOffsetY + (7 - pos.second) * cellH);
}

int main() {
    // Создание окна с фиксированным размером и ограничениями
    sf::RenderWindow window(sf::VideoMode(900, 600), "Chess Game", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60); // Ограничение FPS для стабильности
    window.setKeyRepeatEnabled(true); // Разрешение повторного ввода клавиш

    // Инициализация шахматной доски
    Chess_Board board;
    board.setupBoard(); // Расстановка фигур в начальную позицию

    // Загрузка единой текстуры всех шахматных фигур
    sf::Texture allPiecesTexture;
    if (!allPiecesTexture.loadFromFile("textures/chess_pieces.png")) {
        std::cerr << "Не удалось загрузить текстуру шахматных фигур!" << std::endl;
        return -1;
    }

    // Создание спрайтов для каждой фигуры с использованием текстурных прямоугольников
    std::map<std::string, sf::Sprite> pieceSprites;
    
    // Размеры одной фигуры на текстуре (предполагаем, что все фигуры одинакового размера)
    const int PIECE_WIDTH = allPiecesTexture.getSize().x / 6; // 6 фигур в строке
    const int PIECE_HEIGHT = allPiecesTexture.getSize().y / 2; // 2 строки (черные сверху, белые снизу)
    
    // Порядок фигур в текстуре: ладья, слон, королева, король, конь, пешка
    std::vector<std::string> pieceNames = {"rook", "bishop", "queen", "king", "horse", "pawn"};
    
    // Создание спрайтов для черных фигур (верхняя строка)
    for (int i = 0; i < 6; i++) {
        std::string key = "black_" + pieceNames[i];
        pieceSprites[key].setTexture(allPiecesTexture);
        pieceSprites[key].setTextureRect(sf::IntRect(i * PIECE_WIDTH, 0, PIECE_WIDTH, PIECE_HEIGHT));
    }
    
    // Создание спрайтов для белых фигур (нижняя строка)
    for (int i = 0; i < 6; i++) {
        std::string key = "white_" + pieceNames[i];
        pieceSprites[key].setTexture(allPiecesTexture);
        pieceSprites[key].setTextureRect(sf::IntRect(i * PIECE_WIDTH, PIECE_HEIGHT, PIECE_WIDTH, PIECE_HEIGHT));
    }

    // Загрузка текстур драконов (визуальное представление ИИ)
    std::map<std::string, sf::Texture> dragonTextures;
    std::map<std::string, sf::Sprite> dragonSprites;
    
    // Лямбда-функция для загрузки текстур драконов
    auto loadDragonTexture = [&](const std::string& color, const std::string& state) {
        std::string key = color + "_" + state; // Ключ (например, "green_afk")
        std::string path = "textures/dracons/" + color + "_drakon_" + state + ".png";
        if (!dragonTextures[key].loadFromFile(path)) {
            std::cerr << "Не удалось загрузить дракона: " << path << std::endl;
            // Создание placeholder-текстуры если загрузка не удалась
            sf::Image placeholder;
            placeholder.create(200, 200, sf::Color::Transparent);
            dragonTextures[key].loadFromImage(placeholder);
        }
        dragonSprites[key].setTexture(dragonTextures[key]);
    };

    // Загрузка всех возможных текстур драконов
    std::vector<std::string> dragonColors = {"green", "yellow", "red", "black"};
    std::vector<std::string> dragonStates = {"afk", "loos", "win"};
    
    for (const auto& color : dragonColors) {
        for (const auto& state : dragonStates) {
            loadDragonTexture(color, state);
        }
    }

    // Настройки игры по умолчанию
    MathProblem::Difficulty mathDiff = MathProblem::Difficulty::Green; // Сложность математических задач
    bool playerIsWhite = true; // Игрок играет белыми
    int aiLevel = 1; // Уровень ИИ: 0-3 соответствуют драконам: green, yellow, red, black
    DragonAI ai(!playerIsWhite, aiLevel); // Инициализация ИИ

    // Игровые состояния и переменные
    bool whiteTurn = true; // Чей ход (true - белые)
    std::pair<int,int> selected{-1,-1}; // Выбранная клетка (-1,-1 если ничего не выбрано)
    std::vector<std::pair<int,int>> legalMoves; // Вектор допустимых ходов для выбранной фигуры
    int moveCount = 1; // Счетчик ходов
    bool gameOver = false; // Флаг окончания игры
    bool playerWon = false; // true если игрок победил

    // Состояния ввода математических ответов
    bool awaitingAnswer = false; // Ожидание ответа на математическую задачу
    MathProblem currentProblem(mathDiff, moveCount); // Текущая математическая задача
    std::pair<int,int> pendingFrom{-1,-1}; // Клетка откуда планируется ход
    std::pair<int,int> pendingTo{-1,-1}; // Клетка куда планируется ход
    std::string answerStr; // Строка для ввода ответа

    // Анимации
    std::vector<Animation> animations;
    sf::Clock animationClock;

    // Функция получения цвета дракона по уровню ИИ
    auto getDragonColor = [](int level) -> std::string {
        switch(level) {
            case 0: return "green";
            case 1: return "yellow";
            case 2: return "red";
            case 3: return "black";
            default: return "green";
        }
    };

    // Функция получения состояния дракона в зависимости от состояния игры
    auto getDragonState = [](bool gameOver, bool playerWon) -> std::string {
        if (!gameOver) return "afk"; // Обычное состояние (не активно)
        if (playerWon) return "loos"; // Дракон проиграл
        return "win"; // Дракон выиграл
    };

    // Функция получения позиции короля для текущего хода
    auto getKingPosition = [&](bool forWhite) -> std::pair<int, int> {
        for (const auto& [pos, piece] : board.board) {
            if (dynamic_cast<King*>(piece.get()) && piece->collor == forWhite) {
                return pos;
            }
        }
        return {-1, -1};
    };

    // Функция проверки шаха
    auto isKingInCheck = [&](bool forWhite) -> bool {
        return board.isCheck(forWhite);
    };

    // Функция добавления анимации перемещения
    auto addMoveAnimation = [&](std::pair<int, int> from, std::pair<int, int> to, 
                               float cellW, float cellH, float offX, float offY, 
                               const std::string& pieceKey) {
        sf::Vector2f startPos = toScreenCoords(from, cellW, cellH, offX, offY);
        sf::Vector2f endPos = toScreenCoords(to, cellW, cellH, offX, offY);
        animations.emplace_back(from, to, startPos, endPos, pieceKey, 0.5f);
    };

    // Функция проверки, находится ли клетка в процессе анимации
    auto isCellAnimating = [&](std::pair<int, int> coord) -> bool {
        for (const auto& anim : animations) {
            if ((anim.fromCoord == coord || anim.toCoord == coord) && anim.active) {
                return true;
            }
        }
        return false;
    };

    // Функция сброса игры в начальное состояние
    auto resetGame = [&]() {
        board.setupBoard(); // Перерасстановка фигур
        whiteTurn = true; // Белые начинают
        selected = {-1,-1}; // Сброс выбора
        legalMoves.clear(); // Очистка допустимых ходов
        moveCount = 1; // Сброс счетчика ходов
        gameOver = false; // Сброс флага окончания игры
        playerWon = false; // Сброс флага победы игрока
        awaitingAnswer = false; // Сброс ожидания ответа
        pendingFrom = {-1,-1}; // Сброс отложенных ходов
        pendingTo = {-1,-1};
        answerStr.clear(); // Очистка строки ответа
        animations.clear(); // Очистка анимаций
        ai = DragonAI(!playerIsWhite, aiLevel); // Переинициализация ИИ
    };

    // Главный игровой цикл
    while (window.isOpen()) {
        float dt = animationClock.restart().asSeconds();
        
        // Обновление анимаций
        for (auto& anim : animations) {
            anim.update(dt);
        }
        
        // Удаление завершенных анимаций
        animations.erase(
            std::remove_if(animations.begin(), animations.end(),
                [](const Animation& anim) { return !anim.active; }),
            animations.end()
        );

        sf::Event event;
        // Обработка всех событий в очереди
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close(); // Закрытие окна

            // Обработка кликов по меню
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                // Лямбда для проверки попадания в область
                auto hit = [&](int x,int y,int w,int h){ return mx>=x && mx<x+w && my>=y && my<y+h; };
                
                // Обработка кнопок меню (всегда активны)
                if (hit(20, 60, 260, 40)) { 
                    // Переключение цвета игрока
                    playerIsWhite = !playerIsWhite; 
                    ai = DragonAI(!playerIsWhite, aiLevel); // Обновление ИИ
                    resetGame(); // Сброс игры при смене стороны
                }
                if (hit(20, 120, 260, 40)) {
                    // Циклическое переключение сложности математических задач
                    if (mathDiff == MathProblem::Difficulty::Green) mathDiff = MathProblem::Difficulty::Yellow;
                    else if (mathDiff == MathProblem::Difficulty::Yellow) mathDiff = MathProblem::Difficulty::Red;
                    else if (mathDiff == MathProblem::Difficulty::Red) mathDiff = MathProblem::Difficulty::Black;
                    else if (mathDiff == MathProblem::Difficulty::Black) mathDiff = MathProblem::Difficulty::Rainbow;
                    else mathDiff = MathProblem::Difficulty::Green;
                }
                if (hit(20, 180, 260, 40)) { 
                    // Переключение уровня ИИ (циклически 0-3)
                    aiLevel = (aiLevel+1)%4; 
                    ai = DragonAI(!playerIsWhite, aiLevel); // Обновление ИИ
                    resetGame(); // Автоматический сброс игры при смене уровня
                }
                // Кнопка ресета (внизу экрана)
                if (hit(20, 500, 260, 40)) {
                    resetGame(); // Ручной сброс игры
                }
            }

            // Обработка ходов на доске (только если игра активна и не ожидается ответ)
            if (!gameOver && !awaitingAnswer && event.type == sf::Event::MouseButtonPressed) {
                // Проверка чей сейчас ход (только ход игрока)
                if (whiteTurn == playerIsWhite) {
                    // Расчет размеров и позиции доски
                    float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
                    float availH = static_cast<float>(window.getSize().y);
                    float boardSize = std::min(availH, availW);
                    float cellW = boardSize / SIZE;
                    float cellH = cellW;
                    float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
                    float offY = (availH - boardSize) * 0.5f;
                    
                    // Проверка что клик внутри доски
                    if (event.mouseButton.x >= offX && event.mouseButton.x < offX + boardSize &&
                        event.mouseButton.y >= offY && event.mouseButton.y < offY + boardSize) {
                        
                        // Преобразование координат экрана в координаты доски
                        int x = static_cast<int>((event.mouseButton.x - offX) / cellW);
                        int y = 7 - static_cast<int>((event.mouseButton.y - offY) / cellH);
                        std::pair<int,int> pos{x,y};

                        if (selected.first == -1) {
                            // Выбор фигуры (первый клик)
                            Chess_Piece* piece = board.getPiece(pos);
                            if (piece && piece->collor == whiteTurn) {
                                selected = pos; // Запоминаем выбранную клетку
                                legalMoves = board.getLegalMoves(selected); // Получаем допустимые ходы
                            }
                        } else {
                            // Второй клик - попытка сделать ход или переключить выбор
                            bool found = false;
                            for (auto &m : legalMoves) if (m == pos) { found = true; break; }
                            
                            if (found) {
                                // Создание математической задачи для хода
                                currentProblem = MathProblem(mathDiff, moveCount);
                                awaitingAnswer = true; // Переход в режим ожидания ответа
                                pendingFrom = selected; // Запоминаем планируемый ход
                                pendingTo = pos;
                                answerStr.clear(); // Очищаем предыдущий ответ
                            } else {
                                // Если кликнули на другую свою фигуру - переключаем выбор
                                Chess_Piece* piece = board.getPiece(pos);
                                if (piece && piece->collor == whiteTurn) {
                                    selected = pos; // Выбираем новую фигуру
                                    legalMoves = board.getLegalMoves(selected); // Получаем новые допустимые ходы
                                } else {
                                    // Если кликнули на пустую клетку или чужую фигуру - сбрасываем выбор
                                    selected = {-1,-1};
                                    legalMoves.clear();
                                }
                            }
                        }
                    } else {
                        // Клик вне доски - сбрасываем выбор
                        selected = {-1,-1};
                        legalMoves.clear();
                    }
                }
            }

            // Обработка ввода ответа на математическую задачу
            if (!gameOver && awaitingAnswer) {
                // Лямбда для обработки отправки ответа
                auto submit = [&](){
                    double userAns = 0.0;
                    try { userAns = std::stod(answerStr); } catch(...) { userAns = 0.0; } // Парсинг ответа
                    bool ok = currentProblem.check(userAns); // Проверка правильности
                    std::pair<int,int> finalTo = pendingTo;
                    
                    // Если ответ неверный - случайный ход из допустимых
                    if (!ok) {
                        if (!legalMoves.empty()) {
                            int idx = rand() % legalMoves.size();
                            finalTo = legalMoves[idx];
                        }
                    }
                    
                    // Получаем информацию о фигуре для анимации
                    Chess_Piece* movingPiece = board.getPiece(pendingFrom);
                    std::string pieceKey = "";
                    if (movingPiece) {
                        std::string color = movingPiece->collor ? "white_" : "black_";
                        if (dynamic_cast<Pawn*>(movingPiece)) pieceKey = color + "pawn";
                        else if (dynamic_cast<Rook*>(movingPiece)) pieceKey = color + "rook";
                        else if (dynamic_cast<Horse*>(movingPiece)) pieceKey = color + "horse";
                        else if (dynamic_cast<Bishop*>(movingPiece)) pieceKey = color + "bishop";
                        else if (dynamic_cast<Queen*>(movingPiece)) pieceKey = color + "queen";
                        else if (dynamic_cast<King*>(movingPiece)) pieceKey = color + "king";
                    }
                    
                    // Выполнение хода
                    if (board.movePiece(pendingFrom, finalTo)) {
                        // Добавляем анимацию если есть информация о фигуре
                        if (!pieceKey.empty()) {
                            float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
                            float availH = static_cast<float>(window.getSize().y);
                            float boardSize = std::min(availH, availW);
                            float cellW = boardSize / SIZE;
                            float cellH = cellW;
                            float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
                            float offY = (availH - boardSize) * 0.5f;
                            
                            addMoveAnimation(pendingFrom, finalTo, cellW, cellH, offX, offY, pieceKey);
                        }
                        
                        whiteTurn = !whiteTurn; // Смена хода
                        moveCount++; // Увеличение счетчика ходов
                        
                        // Проверка на мат
                        if (board.isCheckmate(whiteTurn)) {
                            gameOver = true;
                            playerWon = (whiteTurn != playerIsWhite); // Игрок победил если мат противнику
                        }
                    }
                    
                    // Сброс состояний после хода
                    selected = {-1,-1};
                    legalMoves.clear();
                    awaitingAnswer = false;
                    pendingFrom = pendingTo = {-1,-1};
                    answerStr.clear();
                };

                // Обработка ввода текста
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) { // Backspace
                        if (!answerStr.empty()) answerStr.pop_back();
                    } else if (event.text.unicode == 13) { // Enter
                        submit(); // Отправка ответа
                    } else if ((event.text.unicode>='0' && event.text.unicode<='9') || 
                              event.text.unicode=='-' || event.text.unicode=='.') {
                        // Допустимые символы: цифры, минус, точка
                        if (answerStr.size() < 32) answerStr.push_back(static_cast<char>(event.text.unicode));
                    }
                }
                
                // Обработка нажатий клавиш
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) 
                        submit(); // Отправка по Enter
                    if (event.key.code == sf::Keyboard::Escape) { 
                        // Отмена ввода - возврат к выбору фигуры
                        awaitingAnswer = false; 
                        // Не сбрасываем selected и legalMoves, чтобы можно было выбрать другой ход
                        pendingFrom = pendingTo = {-1,-1}; 
                        answerStr.clear(); 
                    }
                }
            }
        }

        // Ход ИИ (если игра активна и не ожидается ответ, и ход ИИ)
        if (!gameOver && !awaitingAnswer && whiteTurn != playerIsWhite && animations.empty()) {
            AIMove mv = ai.chooseMove(board); // Выбор хода ИИ
            if (mv.from.first != -1) {
                // Получаем информацию о фигуре для анимации
                Chess_Piece* movingPiece = board.getPiece(mv.from);
                std::string pieceKey = "";
                if (movingPiece) {
                    std::string color = movingPiece->collor ? "white_" : "black_";
                    if (dynamic_cast<Pawn*>(movingPiece)) pieceKey = color + "pawn";
                    else if (dynamic_cast<Rook*>(movingPiece)) pieceKey = color + "rook";
                    else if (dynamic_cast<Horse*>(movingPiece)) pieceKey = color + "horse";
                    else if (dynamic_cast<Bishop*>(movingPiece)) pieceKey = color + "bishop";
                    else if (dynamic_cast<Queen*>(movingPiece)) pieceKey = color + "queen";
                    else if (dynamic_cast<King*>(movingPiece)) pieceKey = color + "king";
                }
                
                // Выполнение хода
                if (board.movePiece(mv.from, mv.to)) {
                    // Добавляем анимацию если есть информация о фигуре
                    if (!pieceKey.empty()) {
                        float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
                        float availH = static_cast<float>(window.getSize().y);
                        float boardSize = std::min(availH, availW);
                        float cellW = boardSize / SIZE;
                        float cellH = cellW;
                        float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
                        float offY = (availH - boardSize) * 0.5f;
                        
                        addMoveAnimation(mv.from, mv.to, cellW, cellH, offX, offY, pieceKey);
                    }
                }
            }
            whiteTurn = !whiteTurn; // Смена хода
            
            // Проверка на мат после хода ИИ
            if (board.isCheckmate(whiteTurn)) {
                gameOver = true;
                playerWon = (whiteTurn != playerIsWhite); // Игрок победил если мат противнику
            }
        }

        // Начало отрисовки
        window.clear();

        // Расчет размеров и позиции доски
        float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
        float availH = static_cast<float>(window.getSize().y);
        float boardSize = std::min(availH, availW);
        float cellW = boardSize / SIZE;
        float cellH = cellW;
        float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
        float offY = (availH - boardSize) * 0.5f;

        // Отрисовка меню-колонки слева
        {
            auto mouse = sf::Mouse::getPosition(window);
            // Проверка наведения мыши на элементы
            auto isHover = [&](int x,int y,int w,int h){ return mouse.x>=x && mouse.x<x+w && mouse.y>=y && mouse.y<y+h; };
            
            // Функция отрисовки кнопки
            auto drawBtn = [&](int x,int y,int w,int h, sf::Color fill, bool selected = true){
                sf::RectangleShape r(sf::Vector2f((float)w,(float)h));
                r.setPosition((float)x,(float)y);
                if (isHover(x,y,w,h)) fill = sf::Color(std::min(255,(int)fill.r+20), 
                                                      std::min(255,(int)fill.g+20), 
                                                      std::min(255,(int)fill.b+20)); // Эффект наведения
                r.setFillColor(fill);
                r.setOutlineThickness(selected ? 4.f : 2.f);
                r.setOutlineColor(selected ? sf::Color(255,215,0) : sf::Color(200,200,220)); // Золотая обводка для выбранных
                window.draw(r);
            };

            // Параметры кнопок меню
            int x = 20; int w = BOARD_LEFT-60; int h = 40; int gap = 16; int y = 60;

            // Кнопка выбора цвета
            sf::Color colorFill = playerIsWhite ? sf::Color(70,120,240) : sf::Color(40,40,40);
            drawBtn(x, y, w, h, colorFill, true);

            // Кнопка выбора сложности математики
            sf::Color diffFill = sf::Color(80,80,80);
            if (mathDiff==MathProblem::Difficulty::Green) diffFill = sf::Color(60,160,80);
            else if (mathDiff==MathProblem::Difficulty::Yellow) diffFill = sf::Color(200,180,60);
            else if (mathDiff==MathProblem::Difficulty::Red) diffFill = sf::Color(200,70,70);
            else if (mathDiff==MathProblem::Difficulty::Black) diffFill = sf::Color(30,30,30);
            else diffFill = sf::Color(120,60,160); // Rainbow
            drawBtn(x, y+(h+gap), w, h, diffFill, true);

            // Кнопка выбора уровня ИИ
            int base = 60 + aiLevel*50; if (base>220) base=220;
            sf::Color aiFill(base, base, 100);
            drawBtn(x, y+(h+gap)*2, w, h, aiFill, true);

            // Отрисовка дракона (визуализация ИИ)
            std::string dragonColor = getDragonColor(aiLevel);
            std::string dragonState = getDragonState(gameOver, playerWon);
            std::string dragonKey = dragonColor + "_" + dragonState;
            
            if (dragonSprites.find(dragonKey) != dragonSprites.end()) {
                sf::Sprite& dragonSprite = dragonSprites[dragonKey];
                // Позиционирование дракона по центру меню
                float dragonX = x + w/2 - 100;
                float dragonY = y + (h+gap)*3 + 20;
                dragonSprite.setPosition(dragonX, dragonY);
                
                // Масштабирование дракона
                sf::FloatRect bounds = dragonSprite.getLocalBounds();
                float scale = std::min(150.0f/bounds.width, 150.0f/bounds.height);
                dragonSprite.setScale(scale, scale);
                
                window.draw(dragonSprite);
            }

            // Кнопка ресета (внизу экрана)
            drawBtn(x, 500, w, h, sf::Color(200, 60, 60), true);
        }

        // Отрисовка шахматной доски
        for (int i=0; i<SIZE; i++) {
            for (int j=0; j<SIZE; j++) {
                sf::RectangleShape cell(sf::Vector2f(cellW, cellH));
                cell.setPosition(offX + i*cellW, offY + j*cellH);
                // Чередование цветов клеток
                cell.setFillColor((i+j)%2==0 ? sf::Color(240,217,181) : sf::Color(181,136,99));
                window.draw(cell);
            }
        }

        // Подсветка выбранной клетки
        if (selected.first != -1) {
            sf::RectangleShape sel(sf::Vector2f(cellW, cellH));
            auto sp = toScreenCoords(selected, cellW, cellH, offX, offY);
            sel.setPosition(sp.x, sp.y);
            sel.setFillColor(sf::Color(0, 120, 255, 70)); // Полупрозрачный синий
            window.draw(sel);
        }

        // Подсветка короля при шахе
        if (isKingInCheck(whiteTurn)) {
            std::pair<int, int> kingPos = getKingPosition(whiteTurn);
            if (kingPos.first != -1 && !isCellAnimating(kingPos)) {
                sf::RectangleShape check(sf::Vector2f(cellW, cellH));
                auto kingScreenPos = toScreenCoords(kingPos, cellW, cellH, offX, offY);
                check.setPosition(kingScreenPos.x, kingScreenPos.y);
                check.setFillColor(sf::Color(255, 0, 0, 100)); // Полупрозрачный красный
                window.draw(check);
            }
        }

        // Отрисовка допустимых ходов (зеленые кружки)
        for (auto &m : legalMoves) {
            float r = std::min(cellW, cellH) / 6.f;
            sf::CircleShape hint(r);
            hint.setFillColor(sf::Color(0,255,0,150)); // Полупрозрачный зеленый
            auto screenPos = toScreenCoords(m, cellW, cellH, offX, offY);
            hint.setPosition(screenPos.x + cellW/3.f, screenPos.y + cellH/3.f);
            window.draw(hint);
        }

        // Отрисовка шахматных фигур с использованием единой текстуры
        for (auto &[coord, piece] : board.board) {
            // Пропускаем фигуры, которые в процессе анимации
            if (isCellAnimating(coord)) {
                continue;
            }
            
            std::string name;
            // Определение типа фигуры через dynamic_cast
            if (dynamic_cast<Pawn*>(piece.get())) name = "pawn";
            else if (dynamic_cast<Rook*>(piece.get())) name = "rook";
            else if (dynamic_cast<Horse*>(piece.get())) name = "horse";
            else if (dynamic_cast<Bishop*>(piece.get())) name = "bishop";
            else if (dynamic_cast<Queen*>(piece.get())) name = "queen";
            else if (dynamic_cast<King*>(piece.get())) name = "king";

            // Формирование ключа для спрайта
            std::string key = (piece->collor ? "white_" : "black_") + name;
            
            if (pieceSprites.find(key) != pieceSprites.end()) {
                sf::Sprite& sprite = pieceSprites[key];
                
                // Масштабирование и позиционирование спрайта
                sprite.setScale(cellW / PIECE_WIDTH, cellH / PIECE_HEIGHT);
                sprite.setPosition(toScreenCoords(coord, cellW, cellH, offX, offY));
                window.draw(sprite);
            }
        }

        // Отрисовка анимированных фигур поверх всех остальных
        for (const auto& anim : animations) {
            if (pieceSprites.find(anim.pieceKey) != pieceSprites.end()) {
                sf::Sprite sprite = pieceSprites[anim.pieceKey];
                sprite.setScale(cellW / PIECE_WIDTH, cellH / PIECE_HEIGHT);
                sprite.setPosition(anim.currentPos);
                window.draw(sprite);
            }
        }

        // Обновление заголовка окна при вводе ответа
        if (awaitingAnswer) window.setTitle(answerStr.empty() ? currentProblem.question() : answerStr);
        else window.setTitle("");

        // Отображение всего отрисованного
        window.display();
    }

    return 0;
}
