#include "Arcanoid.h"

const Color COL_RED(255, 0, 0);
const Color COL_YELLOW(255, 255, 0);
const Color COL_GREEN(0, 255, 0);
const Color COL_GREY(100, 100, 100);
const Color COL_ORANGE(255, 165, 0);
const Color COL_PURPLE(128, 0, 128);
const Color COL_CYAN(0, 255, 255);
const Color COL_HOTPINK(255, 105, 180);
const Color COL_WHITE(255, 255, 255);

Ball::Ball() {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_RED);
    reset();
}
void Ball::reset() {
    shape.setPosition(WIN_W / 2.f - BALL_R, WIN_H / 2.f - BALL_R);
    vel = { BALL_START, -BALL_START };
    prev = shape.getPosition();
}
void Ball::reflectX() { vel.x = -vel.x; }
void Ball::reflectY() { vel.y = -vel.y; }
void Ball::limit() {
    vel.x = clamp(vel.x, -MAX_BALL_S, MAX_BALL_S);
    vel.y = clamp(vel.y, -MAX_BALL_S, MAX_BALL_S);
}
void Ball::randomizeDirection() {
    float a = static_cast<float>(rand() % 360) * 3.14159f / 180.f;
    float s = hypot(vel.x, vel.y);
    vel = { s * std::cos(a), s * std::sin(a) };
}

Paddle::Paddle() {
    shp.setSize({ 150.f, PAD_H });
    shp.setFillColor(COL_GREEN);
    shp.setPosition((WIN_W - 150.f) / 2, WIN_H - 50);
}
void Paddle::input() {
    if (Keyboard::isKeyPressed(Keyboard::Left) && shp.getPosition().x > 0)
        shp.move(-PAD_SPEED, 0);
    if (Keyboard::isKeyPressed(Keyboard::Right) &&
        shp.getPosition().x + shp.getSize().x < WIN_W)
        shp.move(PAD_SPEED, 0);
}

void Bonus::update() { shape.move(0.f, BONUS_SPD); }
void Bonus::deactivate() { active = false; }
bool Bonus::isActive() const { return active; }

PadSizeBonus::PadSizeBonus(Vector2f pos) {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_CYAN);
    shape.setPosition(pos);
}
void PadSizeBonus::apply(Game& g) {
    float w = g.pad.shp.getSize().x;
    g.pad.shp.setSize({ std::min(w + PAD_SHR, PAD_MAX_W), PAD_H });
}

BallSpeedBonus::BallSpeedBonus(Vector2f pos) {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_ORANGE);
    shape.setPosition(pos);
}
void BallSpeedBonus::apply(Game& g) {
    g.ball.vel *= 1.25f;
    g.ball.limit();
}

StickyBonus::StickyBonus(Vector2f pos) {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_PURPLE);
    shape.setPosition(pos);
}
void StickyBonus::apply(Game& g) {
    g.willStick = true;
}

BottomSaveBonus::BottomSaveBonus(Vector2f pos) {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_WHITE);
    shape.setPosition(pos);
}
void BottomSaveBonus::apply(Game& g) {
    g.bottom = true;
}

TrajectoryBonus::TrajectoryBonus(Vector2f pos) {
    shape.setRadius(BALL_R);
    shape.setFillColor(COL_HOTPINK);
    shape.setPosition(pos);
}
void TrajectoryBonus::apply(Game& g) {
    g.ball.randomizeDirection();
    g.ball.limit();
}

const RectangleShape& Block::getShape() const { return shp; }

IndestructibleBlock::IndestructibleBlock(Vector2f p) {
    shp.setSize(BLOCK_SZ); shp.setOrigin(BLOCK_SZ / 2.f);
    shp.setPosition(p);    shp.setFillColor(COL_GREY);
}
bool IndestructibleBlock::onHit(Game&, Ball&, vector<Bonus*>&) { return false; }

SpeedUpBlock::SpeedUpBlock(Vector2f p) {
    shp.setSize(BLOCK_SZ); shp.setOrigin(BLOCK_SZ / 2.f);
    shp.setPosition(p);    shp.setFillColor(COL_ORANGE);
    destructible = true;
}
bool SpeedUpBlock::onHit(Game&, Ball& b, vector<Bonus*>&) {
    b.vel *= 1.15f; b.limit(); dead = true; return true;
}

HealthBlock::HealthBlock(Vector2f p, int h, bool hasB) : hp(h) {
    bonus = hasB; destructible = true;
    shp.setSize(BLOCK_SZ); shp.setOrigin(BLOCK_SZ / 2.f);
    shp.setPosition(p);    updateCol();
}
void HealthBlock::updateCol() {
    shp.setFillColor(hp == 3 ? COL_RED : hp == 2 ? COL_YELLOW : COL_GREEN);
}
bool HealthBlock::onHit(Game& g, Ball&, vector<Bonus*>& bonuses) {
    ++g.score; --hp;
    if (hp == 0) {
        g.score++; dead = true;
        if (bonus) {
            Vector2f pos{ shp.getPosition().x - BALL_R, shp.getPosition().y + BLOCK_H / 2 };
            int type = rand() % 5;
            Bonus* b = nullptr;
            switch (type) {
            case 0: b = new PadSizeBonus(pos); break;
            case 1: b = new BallSpeedBonus(pos); break;
            case 2: b = new StickyBonus(pos); break;
            case 3: b = new BottomSaveBonus(pos); break;
            case 4: b = new TrajectoryBonus(pos); break;
            }
            if (b) bonuses.push_back(b);
        }
    }
    else updateCol();
    return dead;
}

Game::Game() : win({ WIN_W, WIN_H }, "Arkanoid") {
    win.setFramerateLimit(60);
    srand(static_cast<unsigned>(time(nullptr)));

    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x < COLS; ++x) {
            Vector2f pos{ OFF_X + x * (BLOCK_W + GAP_X) + BLOCK_W / 2.f,
                          40.f + y * (BLOCK_H + GAP_Y) + BLOCK_H / 2.f };
            int t = rand() % 10;
            if (t == 0) {
                blocks.push_back(new IndestructibleBlock(pos));
            }
            else if (t == 1 && rand() % 2 == 0) {
                blocks.push_back(new SpeedUpBlock(pos));
                ++destroyableLeft;
            }
            else {
                int hp = rand() % 3 + 1;
                bool hasB = (rand() % 3 == 0);
                blocks.push_back(new HealthBlock(pos, hp, hasB));
                ++destroyableLeft;
            }
        }
}

Game::~Game() {
    for (Bonus* b : bonuses) delete b;
    for (Block* b : blocks) delete b;

}

void Game::spawnBonus(Bonus* b) {
    bonuses.push_back(b);
}

float Game::sign(float v) { return v < 0 ? -1.f : 1.f; }

void Game::resolve(const RectangleShape& rect) {
    FloatRect r = rect.getGlobalBounds();
    Vector2f centreBall = ball.shape.getPosition() + Vector2f{ BALL_R, BALL_R };
    Vector2f centreRect{ r.left + r.width / 2.f, r.top + r.height / 2.f };

    float diffX = centreBall.x - centreRect.x;
    float diffY = centreBall.y - centreRect.y;

    float overlapX = (r.width / 2.f + BALL_R) - std::abs(diffX);
    float overlapY = (r.height / 2.f + BALL_R) - std::abs(diffY);

    if (overlapX < overlapY) {
        ball.reflectX();
        ball.shape.move(sign(diffX) * overlapX, 0.f);
    }
    else {
        ball.reflectY();
        ball.shape.move(0.f, sign(diffY) * overlapY);
    }
}

void Game::walls() {
    Vector2f p = ball.shape.getPosition();
    if (p.x < 0) { ball.reflectX(); ball.shape.setPosition(0, p.y); }
    else if (p.x + 2 * BALL_R > WIN_W) {
        ball.reflectX(); ball.shape.setPosition(WIN_W - 2 * BALL_R, p.y);
    }
    if (p.y < 0) { ball.reflectY(); ball.shape.setPosition(p.x, 0); }
}

void Game::bottomCol() {
    if (bottom) {
        bottom = false; ball.reflectY();
        ball.shape.setPosition(ball.shape.getPosition().x, WIN_H - 2 * BALL_R - 1);
        return;
    }
    float w = pad.shp.getSize().x;
    if (w <= PAD_MIN_W) {
        over = true;
        std::cout << "Вы проиграли. Баллы: " << score << std::endl;
        win.close();
        return;
    }
    pad.shp.setSize({ w - PAD_SHR, PAD_H });
    stuck = willStick = bottom = false;
    ball.reset();
}

void Game::paddleCol() {
    if (ball.shape.getGlobalBounds().intersects(pad.shp.getGlobalBounds())) {
        if (willStick) { stuck = true; willStick = false; }
        else           ball.reflectY();
    }
}

void Game::blockCol() {
    FloatRect br = ball.shape.getGlobalBounds();
    for (std::vector<Block*>::iterator it = blocks.begin(); it != blocks.end(); ) {
        if ((*it)->getShape().getGlobalBounds().intersects(br)) {
            resolve((*it)->getShape());
            bool del = (*it)->onHit(*this, ball, bonuses);
            if (del) {
                if ((*it)->destructible) --destroyableLeft;
                delete* it;
                it = blocks.erase(it);
            }
            else ++it;
            break;
        }
        else ++it;
    }

    if (destroyableLeft == 0 && !over) {
        over = true;
        std::cout << "Вы победили. Баллы: " << score << std::endl;
        win.close();
    }
}

void Game::bonusUpdate() {
    for (std::vector<Bonus*>::iterator it = bonuses.begin(); it != bonuses.end();) {
        Bonus* p = *it;
        if (!p || !p->isActive()) { delete p; it = bonuses.erase(it); continue; }
        p->update();

        if (p->shape.getGlobalBounds().intersects(pad.shp.getGlobalBounds())) {
            p->apply(*this);
            p->deactivate();
        }
        else if (p->shape.getPosition().y > WIN_H) {
            p->deactivate();
        }
        ++it;
    }
}

void Game::update() {
    pad.input();

    if (stuck) {
        ball.shape.setPosition(pad.shp.getPosition().x + pad.shp.getSize().x / 2 - BALL_R,
            pad.shp.getPosition().y - 2 * BALL_R);
        bonusUpdate();
        if (Keyboard::isKeyPressed(Keyboard::Space)) stuck = false;
        return;
    }

    float dist = hypot(ball.vel.x, ball.vel.y);
    int   steps = std::max(1, int(std::ceil(dist / (BALL_R * 0.5f))));

    for (int i = 0; i < steps; ++i) {
        Vector2f d = ball.vel / float(steps);
        ball.prev = ball.shape.getPosition();
        ball.shape.move(d);

        walls();
        if (ball.shape.getPosition().y <= 0) {
            ball.shape.setPosition(ball.shape.getPosition().x, 0); break;
        }
        if (ball.shape.getPosition().y > WIN_H) {
            bottomCol(); break;
        }

        paddleCol();
        blockCol();
        if (over) break;
    }
    bonusUpdate();
}

void Game::draw() {
    win.clear(Color::Black);
    win.draw(ball.shape); win.draw(pad.shp);
    for (size_t i = 0; i < blocks.size(); ++i)
        win.draw(blocks[i]->getShape());
    for (size_t i = 0; i < bonuses.size(); ++i)
        if (bonuses[i] && bonuses[i]->isActive())
            win.draw(bonuses[i]->shape);
    win.display();
}

int main() {
    std::setlocale(0, "");
    Game ark;
    while (ark.win.isOpen()) {
        for (Event e; ark.win.pollEvent(e);)
            if (e.type == Event::Closed) ark.win.close();
        if (!ark.over) ark.update();
        ark.draw();
    }
    return 0;
}
