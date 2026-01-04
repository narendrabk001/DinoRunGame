#include "DinoRunGame.h"
#include <cmath>

DinoRunGame::DinoRunGame(QWidget *parent)
    : QWidget(parent)
    , gameState(START)
    , gameSpeed(INITIAL_GAME_SPEED)
    , score(0)
    , highScore(0)
    , isNewHighScore(false)
    , lastCactusTime(0)
    , lastCloudTime(0)
    , lastTreeTime(0)
    , updateTimer(nullptr)
{
    // Set window properties
    setFixedSize(GAME_WIDTH, GAME_HEIGHT + GROUND_HEIGHT);
    setWindowTitle("Dino Run Game - Qt Creator");

    // Load high score from file
    loadHighScore();

    // Initialize game elements
    initializeDino();
    initializeMountains();
    initializeGame();

    // Setup game timer
    updateTimer = new QTimer(this);
    updateTimer->setInterval(16); // ~60 FPS
    connect(updateTimer, &QTimer::timeout, this, &DinoRunGame::gameLoop);

    // Start game timer
    gameTimer.start();

    // Generate initial clouds
    for (int i = 0; i < 4; ++i) {
        generateCloud();
        clouds.last().x = QRandomGenerator::global()->bounded(GAME_WIDTH - 100, GAME_WIDTH + 50);
    }

    // Generate initial trees
    for (int i = 0; i < 3; ++i) {
        generateTree();
    }

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

DinoRunGame::~DinoRunGame()
{
    // Timer is automatically deleted by Qt's parent-child system
}

// Initialization Methods
void DinoRunGame::initializeGame()
{
    cacti.clear();
    clouds.clear();
    trees.clear();

    score = 0;
    gameSpeed = INITIAL_GAME_SPEED;
    lastCactusTime = gameTimer.elapsed();
    lastCloudTime = gameTimer.elapsed();
    lastTreeTime = gameTimer.elapsed();
    isNewHighScore = false;
}

void DinoRunGame::initializeDino()
{
    dino.x = 80;
    dino.baseY = GAME_HEIGHT - GROUND_HEIGHT - 50;
    dino.y = dino.baseY;
    dino.width = 60;
    dino.height = 60;
    dino.velocity = 0.0f;
    dino.state = RUNNING;
    dino.frame = 0;
    dino.animationTimer = 0;
}

void DinoRunGame::initializeMountains()
{
    mountains[0] = Mountain{30, 400, 220, true};
    mountains[1] = Mountain{200, 280, 160, false};
    mountains[2] = Mountain{420, 270, 140, false};
    mountains[3] = Mountain{550, 380, 200, true};
}

// Update Methods
void DinoRunGame::updateDino()
{
    dino.animationTimer += 0.1f;
    dino.frame = static_cast<int>(dino.animationTimer) % 4;

    if (dino.state == JUMPING) {
        dino.velocity += GRAVITY;
        dino.y += static_cast<int>(dino.velocity);

        if (dino.y >= dino.baseY) {
            dino.y = dino.baseY;
            dino.velocity = 0.0f;
            dino.state = RUNNING;
        }
    }
}

void DinoRunGame::updateCacti()
{
    // Update existing cacti
    for (int i = cacti.size() - 1; i >= 0; --i) {
        cacti[i].x -= static_cast<int>(gameSpeed * 0.8f);

        if (cacti[i].x + cacti[i].width < 0) {
            cacti.removeAt(i);
            ++score;

            // Increase speed every 100 points
            if (score % 100 == 0 && gameSpeed < MAX_GAME_SPEED) {
                gameSpeed += SPEED_INCREMENT;
            }
        }
    }

    // Generate new cactus
    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastCactusTime > 1200 + QRandomGenerator::global()->bounded(1800)) {
        generateCactus();
        lastCactusTime = currentTime;
    }
}

void DinoRunGame::updateClouds()
{
    for (int i = clouds.size() - 1; i >= 0; --i) {
        clouds[i].x -= clouds[i].speed;

        float cloudWidth = 90.0f * clouds[i].scale;
        if (clouds[i].x + cloudWidth < 0) {
            clouds.removeAt(i);
        }
    }

    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastCloudTime > 3000 + QRandomGenerator::global()->bounded(3000)) {
        generateCloud();
        lastCloudTime = currentTime;
    }
}

void DinoRunGame::updateTrees()
{
    for (int i = trees.size() - 1; i >= 0; --i) {
        trees[i].x -= static_cast<int>(gameSpeed * 0.15f);

        if (trees[i].x + trees[i].width < 0) {
            trees.removeAt(i);
        }
    }

    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastTreeTime > 4000 + QRandomGenerator::global()->bounded(2000)) {
        generateTree();
        lastTreeTime = currentTime;
    }
}

void DinoRunGame::checkCollisions()
{
    QRect dinoRect = dino.hitbox();

    for (const Cactus &cactus : cacti) {
        QRect cactusRect = cactus.hitbox();

        if (dinoRect.intersects(cactusRect)) {
            gameState = GAME_OVER;
            dino.state = DEAD;
            updateTimer->stop();

            if (score > highScore) {
                highScore = score;
                isNewHighScore = true;
                saveHighScore();
            }

            return;
        }
    }
}

// Generation Methods
void DinoRunGame::generateCactus()
{
    Cactus cactus;
    cactus.type = QRandomGenerator::global()->bounded(4);

    switch (cactus.type) {
    case 0: // Small single
        cactus.width = 20;
        cactus.height = 45;
        break;
    case 1: // Medium single
        cactus.width = 25;
        cactus.height = 65;
        break;
    case 2: // Large single
        cactus.width = 30;
        cactus.height = 80;
        break;
    case 3: // Double cactus
        cactus.width = 45;
        cactus.height = 60;
        break;
    }

    cactus.y = GAME_HEIGHT - GROUND_HEIGHT - cactus.height;
    cactus.x = GAME_WIDTH;
    cacti.append(cactus);
}

void DinoRunGame::generateCloud()
{
    Cloud cloud;
    cloud.scale = 0.5f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 0.7f;
    cloud.speed = 0.3f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 0.6f;
    cloud.y = static_cast<float>(QRandomGenerator::global()->bounded(50, 150));
    cloud.x = static_cast<float>(GAME_WIDTH + 20);
    clouds.append(cloud);
}

void DinoRunGame::generateTree()
{
    Tree tree;
    tree.isBig = QRandomGenerator::global()->bounded(2) == 0;

    if (tree.isBig) {
        tree.width = 35 + QRandomGenerator::global()->bounded(20);
        tree.height = 80 + QRandomGenerator::global()->bounded(30);
    } else {
        tree.width = 25 + QRandomGenerator::global()->bounded(15);
        tree.height = 60 + QRandomGenerator::global()->bounded(20);
    }

    tree.x = GAME_WIDTH + QRandomGenerator::global()->bounded(0, 100);
    trees.append(tree);
}

// Drawing Methods
void DinoRunGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw all game elements in correct order
    drawBackground(painter);
    drawSun(painter);

    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        drawMountain(painter, mountains[i]);
    }

    for (const Cloud &cloud : clouds) {
        drawCloud(painter, cloud);
    }

    for (const Tree &tree : trees) {
        drawTree(painter, tree);
    }

    drawGround(painter);

    for (const Cactus &cactus : cacti) {
        drawCactus(painter, cactus);
    }

    drawDino(painter);
    drawUI(painter);

    if (gameState == START) {
        drawStartScreen(painter);
    } else if (gameState == GAME_OVER) {
        drawGameOverScreen(painter);
    }
}

void DinoRunGame::drawBackground(QPainter &painter)
{
    QLinearGradient skyGradient(0, 0, 0, GAME_HEIGHT);
    skyGradient.setColorAt(0, QColor(135, 206, 235));
    skyGradient.setColorAt(1, QColor(100, 180, 220));
    painter.fillRect(0, 0, width(), GAME_HEIGHT, skyGradient);
}

void DinoRunGame::drawSun(QPainter &painter)
{
    painter.setBrush(QColor(255, 255, 180, 200));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(width() - 120, 40, 45, 45);
}

void DinoRunGame::drawMountain(QPainter &painter, const Mountain &mountain)
{
    painter.setPen(Qt::NoPen);

    if (mountain.isBig) {
        QLinearGradient mountainGrad(mountain.x, GAME_HEIGHT - mountain.height,
                                     mountain.x, GAME_HEIGHT);
        mountainGrad.setColorAt(0, QColor(139, 152, 174));
        mountainGrad.setColorAt(0.7, QColor(108, 122, 144));
        mountainGrad.setColorAt(1, QColor(80, 94, 116));
        painter.setBrush(mountainGrad);
    } else {
        QLinearGradient mountainGrad(mountain.x, GAME_HEIGHT - mountain.height,
                                     mountain.x, GAME_HEIGHT);
        mountainGrad.setColorAt(0, QColor(152, 165, 186));
        mountainGrad.setColorAt(0.7, QColor(122, 135, 156));
        mountainGrad.setColorAt(1, QColor(92, 105, 126));
        painter.setBrush(mountainGrad);
    }

    QPolygonF mountainShape;
    mountainShape << QPointF(static_cast<qreal>(mountain.x), static_cast<qreal>(GAME_HEIGHT))
                  << QPointF(static_cast<qreal>(mountain.x + mountain.width/2),
                             static_cast<qreal>(GAME_HEIGHT - mountain.height))
                  << QPointF(static_cast<qreal>(mountain.x + mountain.width),
                             static_cast<qreal>(GAME_HEIGHT));
    painter.drawPolygon(mountainShape);
}

void DinoRunGame::drawCloud(QPainter &painter, const Cloud &cloud)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 230));

    float width = 100.0f * cloud.scale;
    float height = 40.0f * cloud.scale;

    painter.drawEllipse(static_cast<int>(cloud.x), static_cast<int>(cloud.y),
                        static_cast<int>(width), static_cast<int>(height));
    painter.drawEllipse(static_cast<int>(cloud.x + width/4),
                        static_cast<int>(cloud.y - height/3),
                        static_cast<int>(width * 0.6f),
                        static_cast<int>(height * 0.8f));
}

void DinoRunGame::drawTree(QPainter &painter, const Tree &tree)
{
    painter.setPen(Qt::NoPen);

    int groundY = GAME_HEIGHT - GROUND_HEIGHT;
    int trunkHeight = tree.height / 4;
    int trunkWidth = tree.width / 4;
    int trunkX = tree.x + (tree.width - trunkWidth) / 2;

    // Draw trunk
    QLinearGradient trunkGrad(0, groundY - trunkHeight, 0, groundY);
    trunkGrad.setColorAt(0, QColor(139, 90, 43));
    trunkGrad.setColorAt(1, QColor(101, 67, 33));
    painter.setBrush(trunkGrad);
    painter.drawRect(trunkX, groundY - trunkHeight, trunkWidth, trunkHeight);

    // Draw canopy
    QRadialGradient canopyGrad(tree.x + tree.width/2, groundY - tree.height + trunkHeight/2,
                               tree.width/2);
    if (tree.isBig) {
        canopyGrad.setColorAt(0, QColor(76, 175, 80));
        canopyGrad.setColorAt(1, QColor(56, 142, 60));
    } else {
        canopyGrad.setColorAt(0, QColor(129, 199, 132));
        canopyGrad.setColorAt(1, QColor(102, 159, 105));
    }
    painter.setBrush(canopyGrad);
    painter.drawEllipse(tree.x, groundY - tree.height, tree.width, tree.height - trunkHeight);
}

void DinoRunGame::drawGround(QPainter &painter)
{
    // Ground base
    QLinearGradient groundGrad(0, GAME_HEIGHT - GROUND_HEIGHT, 0, GAME_HEIGHT + GROUND_HEIGHT);
    groundGrad.setColorAt(0, QColor(210, 180, 140));
    groundGrad.setColorAt(1, QColor(180, 150, 110));
    painter.fillRect(0, GAME_HEIGHT - GROUND_HEIGHT,
                     width(), GROUND_HEIGHT, groundGrad);

    // Ground line
    painter.setPen(QPen(QColor(160, 130, 90), 3));
    painter.drawLine(0, GAME_HEIGHT - GROUND_HEIGHT,
                     width(), GAME_HEIGHT - GROUND_HEIGHT);
}

void DinoRunGame::drawDino(QPainter &painter)
{
    // Shadow
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(dino.x + 5, dino.y + dino.height - 8,
                        dino.width - 10, 15);

    QColor bodyColor, bellyColor;
    if (dino.state == DEAD) {
        bodyColor = QColor(100, 100, 100);
        bellyColor = QColor(130, 130, 130);
    } else {
        bodyColor = QColor(83, 130, 83);
        bellyColor = QColor(110, 160, 110);
    }

    // Body (main oval)
    painter.setBrush(bodyColor);
    painter.drawEllipse(dino.x, dino.y, dino.width, dino.height);

    // Belly (inner oval)
    painter.setBrush(bellyColor);
    painter.drawEllipse(dino.x + 10, dino.y + 10,
                        dino.width - 20, dino.height - 20);

    // Head
    painter.setBrush(bodyColor);
    QPolygon head;
    int headX = dino.x + 35;
    int headY = dino.y + 10;
    head << QPoint(headX, headY)
         << QPoint(headX + 20, headY - 10)
         << QPoint(headX + 40, headY)
         << QPoint(headX + 20, headY + 25);
    painter.drawPolygon(head);

    // Eye
    painter.setBrush(Qt::white);
    painter.drawEllipse(dino.x + 45, dino.y + 15, 10, 10);

    if (dino.state == DEAD) {
        // Crossed eyes when dead
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(dino.x + 46, dino.y + 16, dino.x + 49, dino.y + 19);
        painter.drawLine(dino.x + 49, dino.y + 16, dino.x + 46, dino.y + 19);
        painter.setPen(Qt::NoPen);
    } else {
        // Normal eye
        painter.setBrush(Qt::black);
        painter.drawEllipse(dino.x + 48, dino.y + 17, 4, 4);
    }

    // Smile
    painter.setPen(QPen(bodyColor.darker(150), 2));
    painter.drawArc(dino.x + 40, dino.y + 25, 15, 10, 0, 180 * 16);

    // Legs
    painter.setPen(Qt::NoPen);
    painter.setBrush(bodyColor);

    int legHeight = 20;
    int legWidth = 12;

    if (dino.state == RUNNING && gameState == PLAYING) {
        // Animated running legs
        float legOffset = std::sin(dino.animationTimer * 10.0f) * 8.0f;
        painter.drawRect(dino.x + 15, dino.y + dino.height - legHeight,
                         legWidth, legHeight + static_cast<int>(legOffset));
        painter.drawRect(dino.x + dino.width - 27, dino.y + dino.height - legHeight,
                         legWidth, legHeight - static_cast<int>(legOffset));
    } else {
        // Stationary legs
        painter.drawRect(dino.x + 15, dino.y + dino.height - legHeight,
                         legWidth, legHeight);
        painter.drawRect(dino.x + dino.width - 27, dino.y + dino.height - legHeight,
                         legWidth, legHeight);
    }

    // Tail
    QPolygon tail;
    tail << QPoint(dino.x - 5, dino.y + 30)
         << QPoint(dino.x - 25, dino.y + 20)
         << QPoint(dino.x - 20, dino.y + 40)
         << QPoint(dino.x, dino.y + 45);
    painter.drawPolygon(tail);
}

void DinoRunGame::drawCactus(QPainter &painter, const Cactus &cactus)
{
    painter.setPen(QPen(QColor(60, 100, 60), 2));

    QColor cactusColor(85, 145, 85);

    switch (cactus.type) {
    case 0: // Small single cactus
        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y, cactus.width, cactus.height, 5, 5);
        break;
    case 1: // Medium single cactus
        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y, cactus.width, cactus.height, 6, 6);
        break;
    case 2: // Large single cactus
        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y, cactus.width, cactus.height, 8, 8);
        break;
    case 3: // Double cactus
        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y + 15, 20, cactus.height - 15, 4, 4);
        painter.drawRoundedRect(cactus.x + 25, cactus.y, 20, cactus.height, 4, 4);
        break;
    }

    // Shadow
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cactus.x, cactus.y + cactus.height - 5,
                        cactus.width, 10);
}

void DinoRunGame::drawTextWithShadow(QPainter &painter, int x, int y, const QString &text,
                                     const QFont &font, const QColor &textColor,
                                     const QColor &shadowColor, int shadowOffset)
{
    painter.setFont(font);

    // Draw shadow
    painter.setPen(shadowColor);
    painter.drawText(x + shadowOffset, y + shadowOffset, text);

    // Draw main text
    painter.setPen(textColor);
    painter.drawText(x, y, text);
}

void DinoRunGame::drawUI(QPainter &painter)
{
    QFont scoreFont("Arial", 20, QFont::Bold);
    QFont smallFont("Arial", 14);

    // Score with shadow
    drawTextWithShadow(painter, 20, 35,
                       QString("SCORE: %1").arg(score, 5, 10, QChar('0')),
                       scoreFont, Qt::white, Qt::black);

    // High score
    drawTextWithShadow(painter, 20, 65,
                       QString("HIGH: %1").arg(highScore, 5, 10, QChar('0')),
                       scoreFont, QColor(255, 215, 0), Qt::black);

    // Speed indicator
    drawTextWithShadow(painter, 20, 95,
                       QString("SPEED: %1x").arg(gameSpeed / INITIAL_GAME_SPEED, 0, 'f', 1),
                       smallFont, QColor(200, 200, 255), Qt::black);
}

void DinoRunGame::drawStartScreen(QPainter &painter)
{
    // Semi-transparent overlay
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    QFont titleFont("Arial", 48, QFont::Bold);
    QFont subtitleFont("Arial", 24);
    QFont instructionFont("Arial", 18);

    // Calculate center positions
    int centerX = width() / 2;

    // Title
    drawTextWithShadow(painter, centerX - 200, 150, "DINO RUN",
                       titleFont, QColor(76, 175, 80), Qt::black, 3);

    // Subtitle
    drawTextWithShadow(painter, centerX - 150, 230, "PRESS SPACE TO START",
                       subtitleFont, QColor(255, 215, 0), Qt::black);

    // Instructions
    drawTextWithShadow(painter, centerX - 180, 280,
                       "SPACE = Jump  •  R = Restart  •  ESC = Quit",
                       instructionFont, Qt::white, Qt::black);

    // High score display
    drawTextWithShadow(painter, centerX - 120, 330,
                       QString("High Score: %1").arg(highScore),
                       instructionFont, QColor(255, 215, 0), Qt::black);
}

void DinoRunGame::drawGameOverScreen(QPainter &painter)
{
    // Semi-transparent overlay
    painter.fillRect(rect(), QColor(0, 0, 0, 200));

    QFont gameOverFont("Arial", 64, QFont::Bold);
    QFont scoreFont("Arial", 36, QFont::Bold);
    QFont instructionFont("Arial", 24);

    // Calculate center positions
    int centerX = width() / 2;

    // Game Over text
    drawTextWithShadow(painter, centerX - 250, 150, "GAME OVER",
                       gameOverFont, Qt::red, Qt::black, 4);

    // Score
    drawTextWithShadow(painter, centerX - 150, 230,
                       QString("SCORE: %1").arg(score),
                       scoreFont, Qt::white, Qt::black);

    // High score message if achieved
    if (isNewHighScore) {
        QFont highScoreFont("Arial", 28, QFont::Bold);
        drawTextWithShadow(painter, centerX - 180, 290, "NEW HIGH SCORE!",
                           highScoreFont, QColor(255, 215, 0), Qt::black, 2);
    } else {
        QFont highScoreMsgFont("Arial", 24);
        drawTextWithShadow(painter, centerX - 180, 290,
                           QString("HIGH SCORE: %1").arg(highScore),
                           highScoreMsgFont, Qt::yellow, Qt::black);
    }

    // Restart instruction
    drawTextWithShadow(painter, centerX - 180, 360, "PRESS R TO RESTART",
                       instructionFont, QColor(100, 200, 255), Qt::black);
}

// Game Loop
void DinoRunGame::gameLoop()
{
    if (gameState == PLAYING) {
        updateDino();
        updateCacti();
        updateClouds();
        updateTrees();
        checkCollisions();
        update();
    }
}

// Key Events
void DinoRunGame::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        if (gameState == START) {
            gameState = PLAYING;
            updateTimer->start();
        } else if (gameState == PLAYING && dino.state == RUNNING) {
            dino.state = JUMPING;
            dino.velocity = JUMP_VELOCITY;
        }
        break;

    case Qt::Key_R:
        if (gameState == GAME_OVER || gameState == PLAYING) {
            gameState = PLAYING;
            initializeGame();
            initializeDino();

            // Generate initial trees
            for (int i = 0; i < 3; ++i) {
                generateTree();
            }

            if (!updateTimer->isActive()) {
                updateTimer->start();
            }
            update();
        }
        break;

    case Qt::Key_Escape:
        close();
        break;

    default:
        QWidget::keyPressEvent(event);
    }
}

// High Score Management
void DinoRunGame::loadHighScore()
{
    QFile file("dino_highscore.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in >> highScore;
        file.close();
    }
}

void DinoRunGame::saveHighScore()
{
    QFile file("dino_highscore.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << highScore;
        file.close();
    }
}
