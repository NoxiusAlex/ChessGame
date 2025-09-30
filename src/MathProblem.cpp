#include "../include/MathProblem.h"
#include <cmath>

static std::mt19937& rng() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

int MathProblem::randInt(int a, int b) { std::uniform_int_distribution<int> d(a,b); return d(rng()); }

MathProblem::MathProblem(Difficulty diff, int moveNumber) : m_diff(diff), m_answer(0.0) {
    switch (diff) {
        case Difficulty::Green: genGreen(moveNumber); break;
        case Difficulty::Yellow: genYellow(moveNumber); break;
        case Difficulty::Red: genRed(moveNumber); break;
        case Difficulty::Black: genBlack(moveNumber); break;
        case Difficulty::Rainbow: genRainbow(moveNumber); break;
    }
}

bool MathProblem::check(double user) const {
    // допускаем небольшую погрешность
    return std::abs(user - m_answer) < 1e-6;
}

void MathProblem::genGreen(int move) {
    // сложение/вычитание, числа растут с ходом
    int maxN = std::min(999, 2 + move * 5);
    int a = randInt(0, maxN);
    int b = randInt(0, maxN);
    bool sub = randInt(0,1);
    if (sub) { m_question = std::to_string(a) + " - " + std::to_string(b) + " = ?"; m_answer = a - b; }
    else { m_question = std::to_string(a) + " + " + std::to_string(b) + " = ?"; m_answer = a + b; }
}

void MathProblem::genYellow(int move) {
    // умножение/деление
    int a = randInt(1, 9 + move % 10);
    int b = randInt(1, 9 + (move+3) % 10);
    bool div = randInt(0,1);
    if (div) {
        int prod = a * b;
        m_question = std::to_string(prod) + " / " + std::to_string(a) + " = ?";
        m_answer = static_cast<double>(prod) / a;
    } else {
        m_question = std::to_string(a) + " * " + std::to_string(b) + " = ?";
        m_answer = a * b;
    }
}

void MathProblem::genRed(int move) {
    // степени/корни/простые дроби
    int type = randInt(0,2);
    if (type==0) {
        int a = randInt(2, 9);
        int p = randInt(2, 4 + (move%3));
        m_question = std::to_string(a) + "^" + std::to_string(p) + " = ?";
        int val = 1; for (int i=0;i<p;i++) val *= a; m_answer = val;
    } else if (type==1) {
        int a = randInt(1, 15);
        m_question = "sqrt(" + std::to_string(a*a) + ") = ?";
        m_answer = a;
    } else {
        int a = randInt(1, 9);
        int b = randInt(1, 9);
        m_question = std::to_string(a) + "/" + std::to_string(b) + " (десятичное) = ?";
        m_answer = static_cast<double>(a)/b;
    }
}

void MathProblem::genBlack(int move) {
    // логарифмы (простые) и пределы (простые)
    int type = randInt(0,1);
    if (type==0) {
        // log2(2^k)
        int k = randInt(1, 6);
        int val = 1 << k;
        m_question = "log2(" + std::to_string(val) + ") = ?";
        m_answer = k;
    } else {
        // lim(x->c) x+n
        int c = randInt(-10, 10);
        int n = randInt(-10, 10);
        m_question = "lim(x->" + std::to_string(c) + ") (x" + (n>=0?"+":"") + std::to_string(n) + ") = ?";
        m_answer = c + n;
    }
}

void MathProblem::genRainbow(int move) {
    int pick = randInt(0,3);
    switch (pick){
        case 0: genGreen(move); break;
        case 1: genYellow(move); break;
        case 2: genRed(move); break;
        case 3: genBlack(move); break;
    }
}


