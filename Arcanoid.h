#pragma once
#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <locale>

using namespace sf;
using namespace std;

extern const Color COL_RED, COL_YELLOW, COL_GREEN, COL_GREY,
COL_ORANGE, COL_PURPLE, COL_CYAN,
COL_HOTPINK, COL_WHITE;

constexpr int   WIN_W = 800, WIN_H = 600;
constexpr float BALL_R = 8.f;
constexpr float BALL_START = 5.5f;
constexpr float MAX_BALL_S = 7.5f;

constexpr float PAD_SPEED = 8.f;
constexpr float PAD_H = 20.f;
constexpr float PAD_SHR = 20.f;
constexpr float PAD_MIN_W = 40.f;
constexpr float PAD_MAX_W = 150.f;

constexpr float BONUS_SPD = 2.f;

constexpr float BLOCK_W = 60.f, BLOCK_H = 20.f;
const     Vector2f BLOCK_SZ{ BLOCK_W, BLOCK_H };
constexpr float GAP_X = 15.f, GAP_Y = 5.f;
constexpr int   COLS = 10, ROWS = 4;
const float FIELD_W = COLS * BLOCK_W + (COLS - 1) * GAP_X;
const float OFF_X = (WIN_W - FIELD_W) / 2.f;

class Game;

class Ball {
public:
    CircleShape shape;
    Vector2f    vel, prev;

    Ball();
    void reset();
    void reflectX();
    void reflectY();
    void limit();
    void randomizeDirection();
};

class Paddle {
public:
    RectangleShape shp;
    Paddle();
    void input();
};

class Bonus {
public:
    CircleShape shape;
    bool active = true;

    virtual ~Bonus() = default;
    virtual void apply(Game&) = 0;

    void update();
    void deactivate();
    bool isActive() const;
};

class PadSizeBonus : public Bonus {
public:
    PadSizeBonus(Vector2f);
    void apply(Game&) override;
};

class BallSpeedBonus : public Bonus {
public:
    BallSpeedBonus(Vector2f);
    void apply(Game&) override;
};

class StickyBonus : public Bonus {
public:
    StickyBonus(Vector2f);
    void apply(Game&) override;
};

class BottomSaveBonus : public Bonus {
public:
    BottomSaveBonus(Vector2f);
    void apply(Game&) override;
};

class TrajectoryBonus : public Bonus {
public:
    TrajectoryBonus(Vector2f);
    void apply(Game&) override;
};

class Block {
protected:
    RectangleShape shp;
public:
    bool dead = false, bonus = false, destructible = false;
    virtual ~Block() = default;
    virtual bool onHit(Game&, Ball&, std::vector<Bonus*>&) = 0;
    const RectangleShape& getShape() const;
};

class IndestructibleBlock : public Block {
public:
    explicit IndestructibleBlock(Vector2f);
    bool onHit(Game&, Ball&, std::vector<Bonus*>&) override;
};

class SpeedUpBlock : public Block {
public:
    explicit SpeedUpBlock(Vector2f);
    bool onHit(Game&, Ball&, std::vector<Bonus*>&) override;
};

class HealthBlock : public Block {
    int hp;
public:
    HealthBlock(Vector2f, int, bool);
    void updateCol();
    bool onHit(Game&, Ball&, std::vector<Bonus*>&) override;
};

class Game {
public:
    RenderWindow win;
    Ball    ball;
    Paddle  pad;
    std::vector<Block*> blocks;
    std::vector<Bonus*> bonuses;

    int  score = 0, destroyableLeft = 0;
    bool stuck = false, willStick = false, bottom = false, over = false;

    Game();
    ~Game();

    void update();
    void draw();

    static float sign(float);

    void resolve(const RectangleShape&);
    void walls();
    void bottomCol();
    void paddleCol();
    void blockCol();

    void bonusUpdate();
    void spawnBonus(Bonus*);
};
