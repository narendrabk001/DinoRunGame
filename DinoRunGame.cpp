#ifndef DINORUNGAME_H
#define DINORUNGAME_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QLinearGradient>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QPolygon>

class DinoRunGame : public QWidget
{
    Q_OBJECT

public:
    explicit DinoRunGame(QWidget *parent = nullptr);
    ~DinoRunGame();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    // Game constants
    enum { GAME_WIDTH = 1200, GAME_HEIGHT = 400, GROUND_HEIGHT = 40 };
    enum { MOUNTAIN_COUNT = 4 };
    enum { JUMP_VELOCITY = -20 };
    static constexpr float GRAVITY = 1.2f;
    static constexpr float INITIAL_GAME_SPEED = 8.0f;
    static constexpr float MAX_GAME_SPEED = 18.0f;
    static constexpr float SPEED_INCREMENT = 0.3f;

    enum GameState { START, PLAYING, GAME_OVER };
    enum DinoState { RUNNING, JUMPING, DUCKING, DEAD };

    // Game objects
    struct Dino {
        int x, y, baseY;
        int width, height;
        float velocity;
        DinoState state;
        int frame;
        float animationTimer;

        QRect hitbox() const {
            return QRect(x + 15, y + 10, width - 30, height - 15);
        }
    };

    struct Cactus {
        int x, y;
        int width, height;
        int type;

        QRect hitbox() const {
            return QRect(x + 5, y + 5, width - 10, height - 10);
        }
    };

    struct Cloud {
        float x, y;
        float scale;
        float speed;
    };

    struct Tree {
        int x;
        int width, height;
        bool isBig;
    };

    struct Mountain {
        int x;
        int width, height;
        bool isBig;
    };

    // Game variables
    GameState gameState;
    Dino dino;
    QList<Cactus> cacti;
    QList<Cloud> clouds;
    QList<Tree> trees;
    Mountain mountains[MOUNTAIN_COUNT];

    float gameSpeed;
    int score;
    int highScore;
    bool isNewHighScore;

    qint64 lastCactusTime;
    qint64 lastCloudTime;
    qint64 lastTreeTime;

    QTimer *updateTimer;
    QElapsedTimer gameTimer;

    // Initialization methods
    void initializeGame();
    void initializeDino();
    void initializeMountains();

    // Update methods
    void updateDino();
    void updateCacti();
    void updateClouds();
    void updateTrees();
    void checkCollisions();

    // Generation methods
    void generateCactus();
    void generateCloud();
    void generateTree();

    // Drawing methods
    void drawBackground(QPainter &painter);
    void drawSun(QPainter &painter);
    void drawMountain(QPainter &painter, const Mountain &mountain);
    void drawCloud(QPainter &painter, const Cloud &cloud);
    void drawTree(QPainter &painter, const Tree &tree);
    void drawGround(QPainter &painter);
    void drawDino(QPainter &painter);
    void drawCactus(QPainter &painter, const Cactus &cactus);
    void drawUI(QPainter &painter);
    void drawStartScreen(QPainter &painter);
    void drawGameOverScreen(QPainter &painter);

    // Utility methods
    void loadHighScore();
    void saveHighScore();

    // Text drawing helper
    void drawTextWithShadow(QPainter &painter, int x, int y, const QString &text,
                            const QFont &font, const QColor &textColor,
                            const QColor &shadowColor = Qt::black, int shadowOffset = 2);
};

#endif // DINORUNGAME_H
