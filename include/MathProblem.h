#pragma once
#include <string>
#include <random>

class MathProblem {
public:
    enum class Difficulty { Green, Yellow, Red, Black, Rainbow };

    MathProblem(Difficulty diff, int moveNumber);
    const std::string& question() const { return m_question; }
    double correctAnswer() const { return m_answer; }
    bool check(double user) const;

private:
    Difficulty m_diff;
    std::string m_question;
    double m_answer;

    void genGreen(int move);
    void genYellow(int move);
    void genRed(int move);
    void genBlack(int move);
    void genRainbow(int move);
    static int randInt(int a, int b);
};


