#pragma once
#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>

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

struct Bonus {
    enum class Type { PadSize, BallSpeed, Sticky, BottomSave, Trajectory };

    CircleShape shape;
    Type  type = Type::PadSize;
    bool  active = true;

    void update();
    void deactivate();
    bool isActive() const;

    static Bonus make(Type t, Vector2f pos);
};

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

class Game;

class Block {
protected:
    RectangleShape shp;
public:
    bool dead = false, bonus = false, destructible = false;
    virtual ~Block() = default;
    virtual bool onHit(Game&, Ball&, std::vector<std::unique_ptr<Bonus>>&) = 0;
    const RectangleShape& getShape() const;
};

class IndestructibleBlock : public Block {
public:
    explicit IndestructibleBlock(Vector2f p);
    bool onHit(Game&, Ball&, std::vector<std::unique_ptr<Bonus>>&) override;
};

class SpeedUpBlock : public Block {
public:
    explicit SpeedUpBlock(Vector2f p);
    bool onHit(Game&, Ball&, std::vector<std::unique_ptr<Bonus>>&) override;
};

class HealthBlock : public Block {
    int hp;
public:
    HealthBlock(Vector2f p, int h, bool hasB);
    void updateCol();
    bool onHit(Game&, Ball&, std::vector<std::unique_ptr<Bonus>>&) override;
};

class Paddle {
public:
    RectangleShape shp;
    Paddle();
    void input();
};

class Game {
public:
    RenderWindow win;
    Ball    ball;
    Paddle  pad;
    std::vector<std::unique_ptr<Block>> blocks;
    std::vector<std::unique_ptr<Bonus>> bonuses;

    int  score = 0, destroyableLeft = 0;
    bool stuck = false, willStick = false, bottom = false, over = false;

    Game();

    void update();
    void draw();

    static float sign(float v);
    void resolve(const RectangleShape&);

    void walls();
    void bottomCol();
    void paddleCol();
    void blockCol();

    void bonusUpdate();
    void applyBonus(Bonus::Type);
    void spawnBonus(Vector2f pos);

private:
    void initBlocks();
};