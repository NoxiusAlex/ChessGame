#include <SFML/Graphics.hpp>
#include <iostream>
#include "../include/Chess_Board.h"
#include "../include/Pawn.h"
#include "../include/Rook.h"
#include "../include/Bishop.h"
#include "../include/Queen.h"
#include "../include/King.h"
#include "../include/Horse.h"
#include "../include/MathProblem.h"
#include "../include/DragonAI.h"

const int SIZE = 8;
const int BOARD_LEFT = 320;

sf::Vector2f toScreenCoords(std::pair<int,int> pos, float cellW, float cellH, float boardOffsetX, float boardOffsetY) {
    return sf::Vector2f(boardOffsetX + pos.first * cellW, boardOffsetY + (7 - pos.second) * cellH);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(900, 600), "Chess Game", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(true);

    Chess_Board board;
    board.setupBoard();

    // загрузка текстур шахматных фигур
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Sprite> sprites;

    auto loadChessTexture = [&](const std::string& name, const std::string& color) {
        std::string key = color + "_" + name;
        std::string path = "textures/" + color + "/" + key + ".jpg";
        if (!textures[key].loadFromFile(path)) {
            std::cerr << "Не удалось загрузить: " << path << std::endl;
        }
        sprites[key].setTexture(textures[key]);
    };

    for (auto& color : {"white", "black"}) {
        loadChessTexture("pawn", color);
        loadChessTexture("rook", color);
        loadChessTexture("horse", color);
        loadChessTexture("bishop", color);
        loadChessTexture("queen", color);
        loadChessTexture("king", color);
    }

    // Загрузка текстур драконов
    std::map<std::string, sf::Texture> dragonTextures;
    std::map<std::string, sf::Sprite> dragonSprites;
    
    auto loadDragonTexture = [&](const std::string& color, const std::string& state) {
        std::string key = color + "_" + state;
        std::string path = "textures/dracons/" + color + "_drakon_" + state + ".png";
        if (!dragonTextures[key].loadFromFile(path)) {
            std::cerr << "Не удалось загрузить дракона: " << path << std::endl;
            // Создаем placeholder если текстура не загрузилась
            sf::Image placeholder;
            placeholder.create(200, 200, sf::Color::Transparent);
            dragonTextures[key].loadFromImage(placeholder);
        }
        dragonSprites[key].setTexture(dragonTextures[key]);
    };

    // Загружаем текстуры драконов для всех состояний
    std::vector<std::string> dragonColors = {"green", "yellow", "red", "black"};
    std::vector<std::string> dragonStates = {"afk", "loos", "win"};
    
    for (const auto& color : dragonColors) {
        for (const auto& state : dragonStates) {
            loadDragonTexture(color, state);
        }
    }

    // Настройки по умолчанию
    MathProblem::Difficulty mathDiff = MathProblem::Difficulty::Green;
    bool playerIsWhite = true;
    int aiLevel = 1; // 0-3 соответствуют драконам: green, yellow, red, black
    DragonAI ai(!playerIsWhite, aiLevel);

    // Игровые состояния
    bool whiteTurn = true;
    std::pair<int,int> selected{-1,-1};
    std::vector<std::pair<int,int>> legalMoves;
    int moveCount = 1;
    bool gameOver = false;
    bool playerWon = false; // true если игрок победил

    // Состояния ввода
    bool awaitingAnswer = false;
    MathProblem currentProblem(mathDiff, moveCount);
    std::pair<int,int> pendingFrom{-1,-1};
    std::pair<int,int> pendingTo{-1,-1};
    std::string answerStr;

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

    // Функция получения состояния дракона (убрали неиспользуемый параметр)
    auto getDragonState = [](bool gameOver, bool playerWon) -> std::string {
        if (!gameOver) return "afk"; // обычное состояние
        if (playerWon) return "loos"; // дракон проиграл
        return "win"; // дракон выиграл
    };

    // Функция ресета игры
    auto resetGame = [&]() {
        board.setupBoard();
        whiteTurn = true;
        selected = {-1,-1};
        legalMoves.clear();
        moveCount = 1;
        gameOver = false;
        playerWon = false;
        awaitingAnswer = false;
        pendingFrom = {-1,-1};
        pendingTo = {-1,-1};
        answerStr.clear();
        ai = DragonAI(!playerIsWhite, aiLevel);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Обработка кликов по меню
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                auto hit = [&](int x,int y,int w,int h){ return mx>=x && mx<x+w && my>=y && my<y+h; };
                
                // Кнопки меню (обрабатываются всегда)
                if (hit(20, 60, 260, 40)) { 
                    playerIsWhite = !playerIsWhite; 
                    ai = DragonAI(!playerIsWhite, aiLevel);
                    resetGame();
                }
                if (hit(20, 120, 260, 40)) {
                    if (mathDiff == MathProblem::Difficulty::Green) mathDiff = MathProblem::Difficulty::Yellow;
                    else if (mathDiff == MathProblem::Difficulty::Yellow) mathDiff = MathProblem::Difficulty::Red;
                    else if (mathDiff == MathProblem::Difficulty::Red) mathDiff = MathProblem::Difficulty::Black;
                    else if (mathDiff == MathProblem::Difficulty::Black) mathDiff = MathProblem::Difficulty::Rainbow;
                    else mathDiff = MathProblem::Difficulty::Green;
                }
                if (hit(20, 180, 260, 40)) { 
                    aiLevel = (aiLevel+1)%4; 
                    ai = DragonAI(!playerIsWhite, aiLevel);
                    // При смене уровня ИИ автоматически ресетим игру
                    resetGame();
                }
                // Кнопка ресета (внизу экрана)
                if (hit(20, 500, 260, 40)) {
                    resetGame();
                }
            }

            // Обработка ходов на доске
            if (!gameOver && !awaitingAnswer && event.type == sf::Event::MouseButtonPressed) {
                if (whiteTurn == playerIsWhite) {
                    float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
                    float availH = static_cast<float>(window.getSize().y);
                    float boardSize = std::min(availH, availW);
                    float cellW = boardSize / SIZE;
                    float cellH = cellW;
                    float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
                    float offY = (availH - boardSize) * 0.5f;
                    
                    if (event.mouseButton.x >= offX && event.mouseButton.x < offX + boardSize &&
                        event.mouseButton.y >= offY && event.mouseButton.y < offY + boardSize) {
                        
                        int x = static_cast<int>((event.mouseButton.x - offX) / cellW);
                        int y = 7 - static_cast<int>((event.mouseButton.y - offY) / cellH);
                        std::pair<int,int> pos{x,y};

                        if (selected.first == -1) {
                            Chess_Piece* piece = board.getPiece(pos);
                            if (piece && piece->collor == whiteTurn) {
                                selected = pos;
                                legalMoves = board.getLegalMoves(selected);
                            }
                        } else {
                            bool found = false;
                            for (auto &m : legalMoves) if (m == pos) { found = true; break; }
                            if (found) {
                                currentProblem = MathProblem(mathDiff, moveCount);
                                awaitingAnswer = true;
                                pendingFrom = selected;
                                pendingTo = pos;
                                answerStr.clear();
                            }
                        }
                    }
                }
            }

            // Набор ответа
            if (!gameOver && awaitingAnswer) {
                auto submit = [&](){
                    double userAns = 0.0;
                    try { userAns = std::stod(answerStr); } catch(...) { userAns = 0.0; }
                    bool ok = currentProblem.check(userAns);
                    std::pair<int,int> finalTo = pendingTo;
                    if (!ok) {
                        if (!legalMoves.empty()) {
                            int idx = rand() % legalMoves.size();
                            finalTo = legalMoves[idx];
                        }
                    }
                    if (board.movePiece(pendingFrom, finalTo)) {
                        whiteTurn = !whiteTurn;
                        moveCount++;
                        if (board.isCheckmate(whiteTurn)) {
                            gameOver = true;
                            playerWon = (whiteTurn != playerIsWhite); // Игрок победил если мат противнику
                        }
                    }
                    selected = {-1,-1};
                    legalMoves.clear();
                    awaitingAnswer = false;
                    pendingFrom = pendingTo = {-1,-1};
                    answerStr.clear();
                };

                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) {
                        if (!answerStr.empty()) answerStr.pop_back();
                    } else if (event.text.unicode == 13) {
                        submit();
                    } else if ((event.text.unicode>='0' && event.text.unicode<='9') || event.text.unicode=='-' || event.text.unicode=='.') {
                        if (answerStr.size() < 32) answerStr.push_back(static_cast<char>(event.text.unicode));
                    }
                }
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) submit();
                    if (event.key.code == sf::Keyboard::Escape) { 
                        awaitingAnswer = false; 
                        pendingFrom = pendingTo = {-1,-1}; 
                        answerStr.clear(); 
                    }
                }
            }
        }

        // Ход ИИ
        if (!gameOver && !awaitingAnswer && whiteTurn != playerIsWhite) {
            AIMove mv = ai.chooseMove(board);
            if (mv.from.first != -1) {
                board.movePiece(mv.from, mv.to);
            }
            whiteTurn = !whiteTurn;
            if (board.isCheckmate(whiteTurn)) {
                gameOver = true;
                playerWon = (whiteTurn != playerIsWhite); // Игрок победил если мат противнику
            }
        }

        // Отрисовка
        window.clear();

        float availW = static_cast<float>(window.getSize().x - BOARD_LEFT);
        float availH = static_cast<float>(window.getSize().y);
        float boardSize = std::min(availH, availW);
        float cellW = boardSize / SIZE;
        float cellH = cellW;
        float offX = static_cast<float>(BOARD_LEFT) + (availW - boardSize) * 0.5f;
        float offY = (availH - boardSize) * 0.5f;

        // Меню-колонка слева
        {
            auto mouse = sf::Mouse::getPosition(window);
            auto isHover = [&](int x,int y,int w,int h){ return mouse.x>=x && mouse.x<x+w && mouse.y>=y && mouse.y<y+h; };
            auto drawBtn = [&](int x,int y,int w,int h, sf::Color fill, bool selected = true){
                sf::RectangleShape r(sf::Vector2f((float)w,(float)h));
                r.setPosition((float)x,(float)y);
                if (isHover(x,y,w,h)) fill = sf::Color(std::min(255,(int)fill.r+20), std::min(255,(int)fill.g+20), std::min(255,(int)fill.b+20));
                r.setFillColor(fill);
                r.setOutlineThickness(selected ? 4.f : 2.f);
                r.setOutlineColor(selected ? sf::Color(255,215,0) : sf::Color(200,200,220));
                window.draw(r);
            };

            int x = 20; int w = BOARD_LEFT-60; int h = 40; int gap = 16; int y = 60;

            // Color toggle
            sf::Color colorFill = playerIsWhite ? sf::Color(70,120,240) : sf::Color(40,40,40);
            drawBtn(x, y, w, h, colorFill, true);

            // Difficulty
            sf::Color diffFill = sf::Color(80,80,80);
            if (mathDiff==MathProblem::Difficulty::Green) diffFill = sf::Color(60,160,80);
            else if (mathDiff==MathProblem::Difficulty::Yellow) diffFill = sf::Color(200,180,60);
            else if (mathDiff==MathProblem::Difficulty::Red) diffFill = sf::Color(200,70,70);
            else if (mathDiff==MathProblem::Difficulty::Black) diffFill = sf::Color(30,30,30);
            else diffFill = sf::Color(120,60,160);
            drawBtn(x, y+(h+gap), w, h, diffFill, true);

            // AI level
            int base = 60 + aiLevel*50; if (base>220) base=220;
            sf::Color aiFill(base, base, 100);
            drawBtn(x, y+(h+gap)*2, w, h, aiFill, true);

            // Отображение дракона между кнопками уровня ИИ и ресета
            std::string dragonColor = getDragonColor(aiLevel);
            std::string dragonState = getDragonState(gameOver, playerWon);
            std::string dragonKey = dragonColor + "_" + dragonState;
            
            if (dragonSprites.find(dragonKey) != dragonSprites.end()) {
                sf::Sprite& dragonSprite = dragonSprites[dragonKey];
                // Позиционируем дракона по центру меню
                float dragonX = x + w/2 - 100; // Центрируем по ширине кнопки
                float dragonY = y + (h+gap)*3 + 20; // Под кнопкой уровня ИИ
                dragonSprite.setPosition(dragonX, dragonY);
                
                // Масштабируем дракона чтобы он вписывался в доступное пространство
                sf::FloatRect bounds = dragonSprite.getLocalBounds();
                float scale = std::min(150.0f/bounds.width, 150.0f/bounds.height);
                dragonSprite.setScale(scale, scale);
                
                window.draw(dragonSprite);
            }

            // Кнопка ресета (внизу)
            drawBtn(x, 500, w, h, sf::Color(200, 60, 60), true);
        }

        // Отрисовка доски и фигур (остальной код без изменений)
        for (int i=0; i<SIZE; i++) {
            for (int j=0; j<SIZE; j++) {
                sf::RectangleShape cell(sf::Vector2f(cellW, cellH));
                cell.setPosition(offX + i*cellW, offY + j*cellH);
                cell.setFillColor((i+j)%2==0 ? sf::Color(240,217,181) : sf::Color(181,136,99));
                window.draw(cell);
            }
        }

        if (selected.first != -1) {
            sf::RectangleShape sel(sf::Vector2f(cellW, cellH));
            auto sp = toScreenCoords(selected, cellW, cellH, offX, offY);
            sel.setPosition(sp.x, sp.y);
            sel.setFillColor(sf::Color(0, 120, 255, 70));
            window.draw(sel);
        }

        for (auto &m : legalMoves) {
            float r = std::min(cellW, cellH) / 6.f;
            sf::CircleShape hint(r);
            hint.setFillColor(sf::Color(0,255,0,150));
            auto screenPos = toScreenCoords(m, cellW, cellH, offX, offY);
            hint.setPosition(screenPos.x + cellW/3.f, screenPos.y + cellH/3.f);
            window.draw(hint);
        }

        for (auto &[coord, piece] : board.board) {
            std::string name;
            if (dynamic_cast<Pawn*>(piece.get())) name = "pawn";
            else if (dynamic_cast<Rook*>(piece.get())) name = "rook";
            else if (dynamic_cast<Horse*>(piece.get())) name = "horse";
            else if (dynamic_cast<Bishop*>(piece.get())) name = "bishop";
            else if (dynamic_cast<Queen*>(piece.get())) name = "queen";
            else if (dynamic_cast<King*>(piece.get())) name = "king";

            std::string key = (piece->collor ? "white_" : "black_") + name;
            auto sp = sprites[key];
            const sf::Texture* tex = sp.getTexture();
            if (!tex) continue;

            sp.setScale(cellW / tex->getSize().x, cellH / tex->getSize().y);
            sp.setPosition(toScreenCoords(coord, cellW, cellH, offX, offY));
            window.draw(sp);
        }

        if (awaitingAnswer) window.setTitle(answerStr.empty() ? currentProblem.question() : answerStr);
        else window.setTitle("");

        window.display();
    }

    return 0;
}
