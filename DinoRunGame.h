#ifndef DINORUNGAME_H
#define DINORUNGAME_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QVector>

class DinoRunGame : public QWidget {
    Q_OBJECT

public:
    explicit DinoRunGame(QWidget *parent = nullptr);
    ~DinoRunGame();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Game constants
    static const int GAME_WIDTH = 800;
    static const int GAME_HEIGHT = 400;
    static const int GROUND_HEIGHT = 50;
    static const int MOUNTAIN_COUNT = 4;

    // Game states
    enum GameState { START, PLAYING, GAME_OVER };
    GameState gameState;

    // Game objects
    struct Dino {
        int x, y, baseY;
        int width, height;
        float velocity;
        bool isJumping;
        int frame;
    } dino;

    struct Cactus {
        int x, y;
        int width, height;
        int type; // 0: small, 1: large, 2: cluster
    };

    struct Cloud {
        float x, y;
        float speed;
        float scale;
    };

    struct Mountain {
        int x;
        int width, height;
        bool isBig;
    };

    struct Tree {
        int x;
        int width, height;
        bool isBig;
    };

    // Game variables
    QVector<Cactus> cacti;
    QVector<Cloud> clouds;
    QVector<Tree> trees;  // Changed from array to QVector
    Mountain mountains[MOUNTAIN_COUNT];
    QTimer *updateTimer;
    QElapsedTimer gameTimer;
    float gameSpeed;
    int score;
    int highScore;
    qint64 lastCactusTime;
    qint64 lastCloudTime;
    qint64 lastTreeTime;  // Added missing declaration

    // Game methods
    void initializeDino();
    void initializeMountains();
    void initializeGame();
    void gameLoop();

    void updateDino();
    void updateCacti();
    void updateClouds();
    void updateTrees();
    void checkCollisions();

    void generateCactus();
    void generateCloud();
    void generateTree();  // Added missing declaration

    void drawDino(QPainter &painter);
    void drawCactus(QPainter &painter, const Cactus &cactus);
    void drawCloud(QPainter &painter, const Cloud &cloud);
    void drawMountain(QPainter &painter, const Mountain &mountain);
    void drawTree(QPainter &painter, const Tree &tree);
    void drawGround(QPainter &painter);
    void drawUI(QPainter &painter);
    void drawStartScreen(QPainter &painter);
    void drawGameOverScreen(QPainter &painter);
};

#endif // DINORUNGAME_H
