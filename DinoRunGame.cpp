#include "DinoRunGame.h"
#include <QApplication>
#include <cmath>

DinoRunGame::DinoRunGame(QWidget *parent)
    : QWidget(parent),
    gameState(START),
    gameSpeed(8.0f),
    score(0),
    highScore(0),
    lastCactusTime(0),
    lastCloudTime(0),
    lastTreeTime(0) {  // Initialize lastTreeTime

    setFixedSize(GAME_WIDTH, GAME_HEIGHT + GROUND_HEIGHT);
    setWindowTitle("Dino Run");

    // Initialize game
    initializeDino();
    initializeMountains();
    initializeGame();

    // Setup game timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &DinoRunGame::gameLoop);
    updateTimer->start(16);

    gameTimer.start();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    // Generate initial clouds
    for (int i = 0; i < 4; i++) {
        generateCloud();
        clouds.last().x = QRandomGenerator::global()->bounded(GAME_WIDTH - 100, GAME_WIDTH + 50);
    }

    // Generate initial trees
    for (int i = 0; i < 3; i++) {
        generateTree();
    }
}

DinoRunGame::~DinoRunGame() {
    delete updateTimer;
}

void DinoRunGame::initializeDino() {
    dino.x = 80;
    dino.baseY = GAME_HEIGHT - GROUND_HEIGHT - 40;
    dino.y = dino.baseY;
    dino.width = 40;
    dino.height = 50;
    dino.velocity = 0.0f;
    dino.isJumping = false;
    dino.frame = 0;
}

void DinoRunGame::initializeMountains() {
    // Initialize 4 fixed triangle-shaped mountains - BIGGER SIZES, NO SNOW
    // Mountain 1 (Big) - Far left
    mountains[0].x = 30;
    mountains[0].width = 400;   // Increased size
    mountains[0].height = 220;  // Increased size
    mountains[0].isBig = true;

    // Mountain 2 (Small) - Left-middle
    mountains[1].x = 200;
    mountains[1].width = 280;   // Increased size
    mountains[1].height = 160;  // Increased size
    mountains[1].isBig = false;

    // Mountain 3 (Small) - Right-middle
    mountains[2].x = 420;
    mountains[2].width = 270;   // Increased size
    mountains[2].height = 140;  // Increased size
    mountains[2].isBig = false;

    // Mountain 4 (Big) - Far right
    mountains[3].x = 550;
    mountains[3].width = 380;   // Increased size
    mountains[3].height = 200;  // Increased size
    mountains[3].isBig = true;
}

void DinoRunGame::initializeGame() {
    cacti.clear();
    clouds.clear();
    trees.clear();
    score = 0;
    gameSpeed = 8.0f;
    lastCactusTime = gameTimer.elapsed();
    lastCloudTime = gameTimer.elapsed();
    lastTreeTime = gameTimer.elapsed();
}

void DinoRunGame::updateDino() {
    // Update animation frame
    dino.frame = (dino.frame + 1) % 10;

    // Handle jumping physics
    if (dino.isJumping) {
        dino.velocity += 1.0f;  // Gravity
        dino.y += dino.velocity;

        // Check if landed
        if (dino.y >= dino.baseY) {
            dino.y = dino.baseY;
            dino.velocity = 0.0f;
            dino.isJumping = false;
        }
    }
}

void DinoRunGame::updateCacti() {
    // Move cacti
    for (int i = cacti.size() - 1; i >= 0; i--) {
        cacti[i].x -= gameSpeed * 0.8f;

        // Remove cacti that are off screen
        if (cacti[i].x + cacti[i].width < 0) {
            cacti.removeAt(i);
            score += 1;
        }
    }

    // Generate new cacti
    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastCactusTime > 1500 + QRandomGenerator::global()->bounded(1500)) {
        generateCactus();
        lastCactusTime = currentTime;

        // Increase game speed gradually
        if (score % 150 == 0 && gameSpeed < 18.0f) {
            gameSpeed += 0.3f;
        }
    }
}

void DinoRunGame::updateClouds() {
    // Move clouds from right to left
    for (int i = clouds.size() - 1; i >= 0; i--) {
        clouds[i].x -= clouds[i].speed;

        // Remove clouds when they completely leave the left side
        float cloudWidth = 90 * clouds[i].scale;
        if (clouds[i].x + cloudWidth < 0) {
            clouds.removeAt(i);
        }
    }

    // Generate new clouds from the right
    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastCloudTime > 3000 + QRandomGenerator::global()->bounded(3000)) {
        generateCloud();
        lastCloudTime = currentTime;
    }
}

void DinoRunGame::updateTrees() {
    // Move trees from right to left - VERY SLOW SPEED
    for (int i = trees.size() - 1; i >= 0; i--) {
        trees[i].x -= gameSpeed * 0.15f; // 15% of game speed - VERY SLOW

        // Remove trees when they completely leave the left side
        if (trees[i].x + trees[i].width < 0) {
            trees.removeAt(i);
        }
    }

    // Generate new trees from the right
    qint64 currentTime = gameTimer.elapsed();
    if (currentTime - lastTreeTime > 4000 + QRandomGenerator::global()->bounded(2000)) {
        generateTree();
        lastTreeTime = currentTime;
    }
}

void DinoRunGame::generateCactus() {
    Cactus cactus;
    cactus.type = QRandomGenerator::global()->bounded(3);

    switch (cactus.type) {
    case 0: // Small cactus
        cactus.width = 25;
        cactus.height = 45;
        break;
    case 1: // Large cactus
        cactus.width = 35;
        cactus.height = 65;
        break;
    case 2: // Cactus cluster
        cactus.width = 70;
        cactus.height = 55;
        break;
    }

    cactus.y = GAME_HEIGHT - GROUND_HEIGHT - cactus.height;
    cactus.x = GAME_WIDTH;
    cacti.append(cactus);
}

void DinoRunGame::generateCloud() {
    Cloud cloud;
    cloud.scale = 0.6f + QRandomGenerator::global()->generateDouble() * 0.8f;
    cloud.speed = 0.4f + QRandomGenerator::global()->generateDouble() * 0.8f;
    cloud.y = QRandomGenerator::global()->bounded(50, 150);

    // Start from right edge
    cloud.x = GAME_WIDTH + 20;

    clouds.append(cloud);
}

void DinoRunGame::generateTree() {
    Tree tree;

    // Randomize tree size
    if (QRandomGenerator::global()->bounded(2) == 0) {
        tree.isBig = true;
        tree.width = 30 + QRandomGenerator::global()->bounded(15);
        tree.height = 70 + QRandomGenerator::global()->bounded(20);
    } else {
        tree.isBig = false;
        tree.width = 20 + QRandomGenerator::global()->bounded(10);
        tree.height = 50 + QRandomGenerator::global()->bounded(15);
    }

    // Position on ground, start from right side
    tree.x = GAME_WIDTH + QRandomGenerator::global()->bounded(0, 50);

    trees.append(tree);
}

void DinoRunGame::checkCollisions() {
    // Dino hitbox
    QRect dinoRect(dino.x + 10, dino.y + 10, dino.width - 20, dino.height - 15);

    for (const Cactus &cactus : cacti) {
        // Cactus hitbox
        QRect cactusRect(cactus.x + 5, cactus.y + 5, cactus.width - 10, cactus.height - 10);

        if (dinoRect.intersects(cactusRect)) {
            gameState = GAME_OVER;
            updateTimer->stop();

            if (score > highScore) {
                highScore = score;
            }

            return;
        }
    }
}

void DinoRunGame::gameLoop() {
    if (gameState == PLAYING) {
        updateDino();
        updateCacti();
        updateClouds();
        updateTrees();  // Update trees (right to left movement)
        checkCollisions();
        update();
    }
}

void DinoRunGame::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw sky with gradient
    QLinearGradient skyGradient(0, 0, 0, GAME_HEIGHT);
    skyGradient.setColorAt(0, QColor(140, 200, 255));
    skyGradient.setColorAt(1, QColor(120, 190, 240));
    painter.fillRect(0, 0, width(), GAME_HEIGHT, skyGradient);

    // Draw sun
    painter.setBrush(QColor(255, 255, 180));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(width() - 100, 50, 35, 35);

    // Draw mountains (BIGGER, NO SNOW)
    for (int i = 0; i < MOUNTAIN_COUNT; i++) {
        drawMountain(painter, mountains[i]);
    }

    // Draw clouds
    for (const Cloud &cloud : clouds) {
        drawCloud(painter, cloud);
    }

    // Draw trees (moving right to left, very slow)
    for (const Tree &tree : trees) {
        drawTree(painter, tree);
    }

    // Draw ground
    drawGround(painter);

    // Draw cacti
    for (const Cactus &cactus : cacti) {
        drawCactus(painter, cactus);
    }

    // Draw dino
    drawDino(painter);

    // Draw UI with better fonts
    drawUI(painter);

    // Draw screens with better fonts
    if (gameState == START) {
        drawStartScreen(painter);
    } else if (gameState == GAME_OVER) {
        drawGameOverScreen(painter);
    }
}

void DinoRunGame::drawDino(QPainter &painter) {
    // Draw dino shadow
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(dino.x + 5, dino.y + dino.height - 5, dino.width - 10, 10);

    // Draw dino body
    QColor bodyColor, bellyColor, eyeColor;

    if (gameState == GAME_OVER) {
        bodyColor = QColor(100, 150, 100);
        bellyColor = QColor(120, 170, 120);
        eyeColor = QColor(200, 200, 200);
    } else {
        bodyColor = QColor(60, 100, 60);
        bellyColor = QColor(100, 150, 100);
        eyeColor = QColor(255, 255, 200);
    }

    // Draw main body
    painter.setBrush(bodyColor);
    painter.drawEllipse(dino.x, dino.y, dino.width, dino.height);

    // Draw belly
    painter.setBrush(bellyColor);
    painter.drawEllipse(dino.x + 5, dino.y + 5, dino.width - 10, dino.height - 10);

    // Draw head
    painter.setBrush(bodyColor);
    QPolygon head;
    head << QPoint(dino.x + 25, dino.y + 10)
         << QPoint(dino.x + 35, dino.y - 5)
         << QPoint(dino.x + 45, dino.y + 5)
         << QPoint(dino.x + 35, dino.y + 20);
    painter.drawPolygon(head);

    // Draw jaw
    painter.setBrush(bellyColor);
    painter.drawEllipse(dino.x + 30, dino.y + 5, 10, 8);

    // Draw eyes
    painter.setBrush(eyeColor);
    if (gameState == GAME_OVER) {
        painter.setPen(QPen(Qt::white, 2));
        painter.drawLine(dino.x + 33, dino.y + 8, dino.x + 37, dino.y + 12);
        painter.drawLine(dino.x + 37, dino.y + 8, dino.x + 33, dino.y + 12);
        painter.setPen(Qt::NoPen);
    } else {
        if (dino.frame % 20 > 2) {
            painter.drawEllipse(dino.x + 33, dino.y + 8, 6, 6);

            painter.setBrush(Qt::black);
            painter.drawEllipse(dino.x + 35, dino.y + 10, 3, 3);

            painter.setBrush(Qt::white);
            painter.drawEllipse(dino.x + 36, dino.y + 9, 2, 2);
            painter.setBrush(eyeColor);
        }
    }

    // Draw mouth
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    if (gameState == GAME_OVER) {
        painter.drawArc(dino.x + 28, dino.y + 12, 8, 6, 0 * 16, 180 * 16);
    } else {
        painter.drawArc(dino.x + 28, dino.y + 12, 8, 6, 180 * 16, 180 * 16);
    }

    // Draw nostrils
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPoint(dino.x + 32, dino.y + 14);
    painter.drawPoint(dino.x + 36, dino.y + 14);

    // Draw legs
    painter.setPen(Qt::NoPen);
    painter.setBrush(bodyColor);

    if (!dino.isJumping && gameState == PLAYING) {
        int legOffset = sin(dino.frame * 0.3f) * 5;

        painter.drawRect(dino.x + 10, dino.y + dino.height - 15, 8, 20);
        painter.drawRect(dino.x + 25, dino.y + dino.height - 20 + legOffset, 8, 20);
        painter.drawRect(dino.x + 5, dino.y + dino.height - 18, 10, 23);
        painter.drawRect(dino.x + 30, dino.y + dino.height - 23 - legOffset, 10, 23);

        painter.setBrush(QColor(220, 180, 140));
        for (int i = 0; i < 2; i++) {
            painter.drawEllipse(dino.x + 8 + i * 20, dino.y + dino.height - 5, 6, 3);
        }
        for (int i = 0; i < 2; i++) {
            painter.drawEllipse(dino.x + 3 + i * 25, dino.y + dino.height - 8, 8, 4);
        }
    } else {
        painter.drawRect(dino.x + 10, dino.y + dino.height - 15, 8, 20);
        painter.drawRect(dino.x + 25, dino.y + dino.height - 15, 8, 20);
        painter.drawRect(dino.x + 5, dino.y + dino.height - 18, 10, 23);
        painter.drawRect(dino.x + 30, dino.y + dino.height - 18, 10, 23);

        painter.setBrush(QColor(220, 180, 140));
        for (int i = 0; i < 4; i++) {
            int x = dino.x + 8 + (i % 2) * 15;
            if (i >= 2) x = dino.x + 3 + (i % 2) * 25;
            int y = dino.y + dino.height - 5;
            int w = (i >= 2) ? 8 : 6;
            int h = (i >= 2) ? 4 : 3;
            painter.drawEllipse(x, y, w, h);
        }
    }

    // Draw tail
    painter.setBrush(bodyColor);
    QPolygon tail;
    tail << QPoint(dino.x + dino.width - 5, dino.y + dino.height/2)
         << QPoint(dino.x + dino.width + 15, dino.y + dino.height/2 - 10)
         << QPoint(dino.x + dino.width + 10, dino.y + dino.height/2 + 5)
         << QPoint(dino.x + dino.width - 10, dino.y + dino.height/2 + 15);
    painter.drawPolygon(tail);

    // Draw spikes
    painter.setBrush(QColor(40, 70, 40));
    for (int i = 0; i < 5; i++) {
        int x = dino.x + 8 + i * 6;
        QPolygon spike;
        spike << QPoint(x, dino.y + 10)
              << QPoint(x - 2, dino.y)
              << QPoint(x + 2, dino.y);
        painter.drawPolygon(spike);
    }
}

void DinoRunGame::drawCactus(QPainter &painter, const Cactus &cactus) {
    painter.setPen(Qt::NoPen);

    switch (cactus.type) {
    case 0: { // Small cactus
        QColor cactusColor(85, 145, 85);
        QColor spineColor(70, 130, 70);

        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y, cactus.width, cactus.height, 4, 4);

        painter.setBrush(spineColor);
        for (int i = 0; i < 5; i++) {
            int y = cactus.y + 10 + i * 8;
            painter.drawEllipse(cactus.x + cactus.width/2 - 1, y, 2, 2);
            painter.drawEllipse(cactus.x + 5, y + 2, 2, 2);
            painter.drawEllipse(cactus.x + cactus.width - 7, y - 2, 2, 2);
        }
        break;
    }

    case 1: { // Large cactus
        QColor cactusColor(80, 140, 80);
        QColor armColor(75, 135, 75);
        QColor spineColor(65, 125, 65);

        painter.setBrush(cactusColor);
        painter.drawRoundedRect(cactus.x, cactus.y, cactus.width, cactus.height, 6, 6);

        painter.setBrush(armColor);
        QPolygon leftArm;
        leftArm << QPoint(cactus.x - 10, cactus.y + 25)
                << QPoint(cactus.x, cactus.y + 20)
                << QPoint(cactus.x, cactus.y + 35)
                << QPoint(cactus.x - 8, cactus.y + 40);
        painter.drawPolygon(leftArm);

        QPolygon rightArm;
        rightArm << QPoint(cactus.x + cactus.width, cactus.y + 35)
                 << QPoint(cactus.x + cactus.width + 10, cactus.y + 40)
                 << QPoint(cactus.x + cactus.width + 8, cactus.y + 45)
                 << QPoint(cactus.x + cactus.width - 2, cactus.y + 40);
        painter.drawPolygon(rightArm);

        painter.setBrush(spineColor);
        for (int i = 0; i < 8; i++) {
            int y = cactus.y + 10 + i * 7;
            painter.drawEllipse(cactus.x + cactus.width/2 - 1, y, 2, 2);
        }
        break;
    }

    case 2: { // Cactus cluster
        QColor cactusColor1(90, 150, 90);
        QColor cactusColor2(85, 145, 85);
        QColor cactusColor3(80, 140, 80);
        QColor spineColor(70, 130, 70);

        painter.setBrush(cactusColor1);
        painter.drawRoundedRect(cactus.x, cactus.y + 5, 22, cactus.height - 5, 4, 4);

        painter.setBrush(cactusColor2);
        painter.drawRoundedRect(cactus.x + 20, cactus.y, 25, cactus.height, 5, 5);

        painter.setBrush(cactusColor3);
        painter.drawRoundedRect(cactus.x + 45, cactus.y + 10, 20, cactus.height - 10, 3, 3);

        painter.setBrush(spineColor);
        for (int i = 0; i < 3; i++) {
            int baseX = cactus.x + (i == 0 ? 11 : (i == 1 ? 32 : 55));
            int baseY = cactus.y + (i == 0 ? 15 : (i == 1 ? 10 : 20));
            int height = cactus.height - (i == 0 ? 10 : (i == 1 ? 0 : 15));

            for (int j = 0; j < 5; j++) {
                int y = baseY + j * 8;
                if (y < cactus.y + height) {
                    painter.drawEllipse(baseX - 1, y, 2, 2);
                }
            }
        }
        break;
    }
    }

    // Draw cactus shadow
    painter.setBrush(QColor(0, 0, 0, 25));
    painter.drawEllipse(cactus.x, cactus.y + cactus.height - 4, cactus.width, 8);
}

void DinoRunGame::drawMountain(QPainter &painter, const Mountain &mountain) {
    painter.setPen(Qt::NoPen);

    // Simple triangle-shaped mountains - BIGGER, NO SNOW
    if (mountain.isBig) {
        QLinearGradient mountainGrad(mountain.x, GAME_HEIGHT - mountain.height,
                                     mountain.x, GAME_HEIGHT);
        mountainGrad.setColorAt(0, QColor(110, 120, 140));  // Lighter at top
        mountainGrad.setColorAt(1, QColor(80, 90, 110));    // Darker at bottom
        painter.setBrush(mountainGrad);
    } else {
        QLinearGradient mountainGrad(mountain.x, GAME_HEIGHT - mountain.height,
                                     mountain.x, GAME_HEIGHT);
        mountainGrad.setColorAt(0, QColor(130, 140, 160));  // Lighter at top
        mountainGrad.setColorAt(1, QColor(100, 110, 130));  // Darker at bottom
        painter.setBrush(mountainGrad);
    }

    // Draw larger triangle shape
    QPolygonF mountainShape;
    mountainShape << QPointF(mountain.x, GAME_HEIGHT)
                  << QPointF(mountain.x + mountain.width/2, GAME_HEIGHT - mountain.height)
                  << QPointF(mountain.x + mountain.width, GAME_HEIGHT)
                  << QPointF(mountain.x, GAME_HEIGHT);

    painter.drawPolygon(mountainShape);

    // Mountain shadow (NO SNOW CAP)
    painter.setBrush(QColor(0, 0, 0, 20));
    painter.drawRect(mountain.x, GAME_HEIGHT - 5, mountain.width, 5);
}

void DinoRunGame::drawTree(QPainter &painter, const Tree &tree) {
    painter.setPen(Qt::NoPen);

    // Tree base is ATTACHED TO GROUND - position it on the ground line
    int groundY = GAME_HEIGHT - GROUND_HEIGHT;
    int treeBaseY = groundY;  // Tree base starts at ground level

    // Calculate tree dimensions
    int treeTrunkHeight = tree.height / 3;
    int treeCanopyHeight = tree.height - treeTrunkHeight;
    int treeTopY = treeBaseY - tree.height;

    // Draw tree trunk - ATTACHED TO GROUND
    QColor trunkColor(120, 80, 60);
    painter.setBrush(trunkColor);

    int trunkWidth = tree.width / 3;
    int trunkX = tree.x + (tree.width - trunkWidth) / 2;

    // Draw trunk from ground up
    painter.drawRect(trunkX, treeBaseY - treeTrunkHeight, trunkWidth, treeTrunkHeight);

    // Draw tree canopy (leaves) - positioned above trunk
    QColor canopyColor;
    if (tree.isBig) {
        canopyColor = QColor(60, 120, 70);  // Darker green for big trees
    } else {
        canopyColor = QColor(80, 140, 90);  // Lighter green for small trees
    }

    painter.setBrush(canopyColor);

    // Draw main canopy (ellipse) - centered above trunk
    int canopyY = treeTopY;
    painter.drawEllipse(tree.x, canopyY, tree.width, treeCanopyHeight);

    // Draw some canopy details for bigger trees
    if (tree.isBig) {
        painter.setBrush(QColor(70, 130, 80));  // Slightly different green
        // Draw smaller overlapping ellipses for texture
        painter.drawEllipse(tree.x - 5, canopyY + 5, tree.width/2, treeCanopyHeight/2);
        painter.drawEllipse(tree.x + tree.width/2, canopyY + 10, tree.width/2, treeCanopyHeight/3);
    }

    // Draw tree shadow on the ground
    painter.setBrush(QColor(0, 0, 0, 20));
    painter.drawEllipse(tree.x, groundY - 2, tree.width, 8);

    // Draw ground connection (roots/base)
    painter.setBrush(QColor(140, 100, 80));
    int rootWidth = tree.width / 2;
    int rootX = tree.x + (tree.width - rootWidth) / 2;
    painter.drawEllipse(rootX, groundY - 5, rootWidth, 10);
}

void DinoRunGame::drawCloud(QPainter &painter, const Cloud &cloud) {
    painter.setPen(Qt::NoPen);

    // Soft cloud color
    QColor cloudColor(255, 255, 255, 210);
    painter.setBrush(cloudColor);

    float width = 90 * cloud.scale;
    float height = 35 * cloud.scale;

    // Simple cloud shape - three overlapping circles
    painter.drawEllipse(cloud.x, cloud.y, width, height);
    painter.drawEllipse(cloud.x + width/3, cloud.y - height/4, width * 0.8f, height);
    painter.drawEllipse(cloud.x - width/6, cloud.y + height/6, width * 0.7f, height * 0.8f);
}

void DinoRunGame::drawGround(QPainter &painter) {
    // Ground color
    QColor groundColor(185, 165, 145);
    painter.fillRect(0, GAME_HEIGHT - GROUND_HEIGHT,
                     width(), GROUND_HEIGHT,
                     groundColor);

    // Ground line - where trees attach
    painter.setPen(QPen(QColor(160, 140, 120), 2));
    painter.drawLine(0, GAME_HEIGHT - GROUND_HEIGHT,
                     width(), GAME_HEIGHT - GROUND_HEIGHT);

    // Ground texture
    painter.setPen(QPen(QColor(170, 150, 130), 1));
    for (int i = 0; i < width(); i += 25) {
        int y = GAME_HEIGHT - GROUND_HEIGHT + 12;
        painter.drawLine(i, y, i + 12, y + 2);
    }

    // Draw some grass/ground details near trees
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(130, 150, 110, 150));
    for (const Tree &tree : trees) {
        int grassX = tree.x + tree.width/2 - 5;
        int grassY = GAME_HEIGHT - GROUND_HEIGHT;
        painter.drawEllipse(grassX, grassY - 3, 10, 6);
    }
}

void DinoRunGame::drawUI(QPainter &painter) {
    // Use nicer fonts with better styling
    QFont scoreFont("Arial", 18, QFont::Bold);
    scoreFont.setStyleHint(QFont::SansSerif);
    painter.setFont(scoreFont);

    // Draw score with shadow effect - IMPROVED ALIGNMENT
    QString scoreText = QString("Score: %1").arg(score);
    QFontMetrics scoreMetrics(scoreFont);
    int scoreWidth = scoreMetrics.horizontalAdvance(scoreText);

    painter.setPen(QPen(QColor(0, 0, 0, 150), 2));
    painter.drawText(12, 32, scoreText);
    painter.setPen(QPen(QColor(255, 255, 255), 2));
    painter.drawText(10, 30, scoreText);

    if (highScore > 0) {
        QString highScoreText = QString("High: %1").arg(highScore);
        QFontMetrics highScoreMetrics(scoreFont);
        int highScoreWidth = highScoreMetrics.horizontalAdvance(highScoreText);

        painter.setPen(QPen(QColor(0, 0, 0, 150), 2));
        painter.drawText(12, 57, highScoreText);
        painter.setPen(QPen(QColor(255, 220, 100), 2)); // Gold color for high score
        painter.drawText(10, 55, highScoreText);
    }

    // Draw speed with smaller font - IMPROVED ALIGNMENT
    QFont speedFont("Arial", 11, QFont::Normal);
    painter.setFont(speedFont);
    QString speedText = QString("Speed: %1").arg(gameSpeed, 0, 'f', 1);

    painter.setPen(QPen(QColor(0, 0, 0, 150), 1));
    painter.drawText(12, 82, speedText);
    painter.setPen(QPen(QColor(200, 230, 255), 1)); // Light blue
    painter.drawText(10, 80, speedText);
}

void DinoRunGame::drawStartScreen(QPainter &painter) {
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    // Title with nice font and shadow - IMPROVED ALIGNMENT
    QFont titleFont("Impact", 48, QFont::Bold);
    titleFont.setStyleHint(QFont::Fantasy);
    painter.setFont(titleFont);

    QString titleText = "DINO RUN";
    QFontMetrics titleMetrics(titleFont);
    int titleWidth = titleMetrics.horizontalAdvance(titleText);

    // Calculate centered position
    int titleX = (width() - titleWidth) / 2;
    int titleY = height() / 2 - 50;

    // Title shadow - IMPROVED POSITIONING
    painter.setPen(QPen(QColor(0, 0, 0, 200), 4));
    painter.drawText(titleX + 2, titleY + 2, titleText);

    // Title main text with gradient - IMPROVED POSITIONING
    QLinearGradient titleGrad(titleX, titleY - 20, titleX + titleWidth, titleY + 20);
    titleGrad.setColorAt(0, QColor(255, 220, 100));
    titleGrad.setColorAt(1, QColor(255, 150, 50));
    painter.setPen(QPen(titleGrad, 3));
    painter.drawText(titleX, titleY, titleText);

    // Draw dino in center - IMPROVED POSITIONING
    painter.save();
    painter.translate(width() / 2 - 20, height() / 2);
    drawDino(painter);
    painter.restore();

    // Instructions with better font - IMPROVED ALIGNMENT
    QFont instructionFont("Verdana", 16, QFont::Bold);
    painter.setFont(instructionFont);
    QString instructionText = "PRESS SPACE TO START";
    QFontMetrics instructionMetrics(instructionFont);
    int instructionWidth = instructionMetrics.horizontalAdvance(instructionText);
    int instructionX = (width() - instructionWidth) / 2;
    int instructionY = height() / 2 + 80;

    painter.setPen(Qt::white);
    painter.drawText(instructionX, instructionY, instructionText);

    // Controls text - IMPROVED ALIGNMENT
    QFont controlsFont("Arial", 12, QFont::Normal);
    painter.setFont(controlsFont);
    QString controlsText = "SPACE = Jump   R = Restart   ESC = Quit";
    QFontMetrics controlsMetrics(controlsFont);
    int controlsWidth = controlsMetrics.horizontalAdvance(controlsText);
    int controlsX = (width() - controlsWidth) / 2;
    int controlsY = instructionY + 30;

    painter.setPen(QColor(200, 200, 200));
    painter.drawText(controlsX, controlsY, controlsText);

    // Draw some decorative elements - IMPROVED POSITIONING
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 30));

    int lineWidth = 300;
    int lineX = (width() - lineWidth) / 2;

    // Line above instructions
    painter.drawRect(lineX, instructionY - 20, lineWidth, 2);
    // Line below controls
    painter.drawRect(lineX, controlsY + 10, lineWidth, 1);
}

void DinoRunGame::drawGameOverScreen(QPainter &painter) {
    painter.fillRect(rect(), QColor(0, 0, 0, 200));

    // GAME OVER text with dramatic font - IMPROVED ALIGNMENT
    QFont gameOverFont("Impact", 56, QFont::Bold);
    gameOverFont.setStyleHint(QFont::Fantasy);
    painter.setFont(gameOverFont);

    QString gameOverText = "GAME OVER";
    QFontMetrics gameOverMetrics(gameOverFont);
    int gameOverWidth = gameOverMetrics.horizontalAdvance(gameOverText);
    int gameOverX = (width() - gameOverWidth) / 2;
    int gameOverY = height() / 2 - 80;

    // Text shadow - IMPROVED POSITIONING
    painter.setPen(QPen(QColor(100, 0, 0, 200), 6));
    painter.drawText(gameOverX + 3, gameOverY + 3, gameOverText);

    // Main text with gradient - IMPROVED POSITIONING
    QLinearGradient gameOverGrad(gameOverX, gameOverY - 25, gameOverX + gameOverWidth, gameOverY + 25);
    gameOverGrad.setColorAt(0, QColor(255, 50, 50));
    gameOverGrad.setColorAt(0.5, QColor(255, 150, 50));
    gameOverGrad.setColorAt(1, QColor(255, 50, 50));
    painter.setPen(QPen(gameOverGrad, 4));
    painter.drawText(gameOverX, gameOverY, gameOverText);

    // Score display - IMPROVED ALIGNMENT
    QFont scoreFont("Arial", 32, QFont::Bold);
    painter.setFont(scoreFont);

    QString scoreText = QString("Score: %1").arg(score);
    QFontMetrics scoreMetrics(scoreFont);
    int scoreWidth = scoreMetrics.horizontalAdvance(scoreText);
    int scoreX = (width() - scoreWidth) / 2;
    int scoreY = height() / 2;

    // Score shadow
    painter.setPen(QPen(QColor(0, 0, 0, 150), 3));
    painter.drawText(scoreX + 2, scoreY + 2, scoreText);

    // Score main text
    painter.setPen(QPen(QColor(255, 255, 255), 3));
    painter.drawText(scoreX, scoreY, scoreText);

    // High score notification - IMPROVED ALIGNMENT
    if (score == highScore && highScore > 0) {
        QFont highScoreFont("Arial", 24, QFont::Bold);
        painter.setFont(highScoreFont);

        QString highScoreText = "NEW HIGH SCORE!";
        QFontMetrics highScoreMetrics(highScoreFont);
        int highScoreWidth = highScoreMetrics.horizontalAdvance(highScoreText);
        int highScoreX = (width() - highScoreWidth) / 2;
        int highScoreY = scoreY + 50;

        // High score shadow
        painter.setPen(QPen(QColor(255, 215, 0, 200), 3));
        painter.drawText(highScoreX + 2, highScoreY + 2, highScoreText);

        // High score main text
        painter.setPen(QPen(QColor(255, 255, 100), 3));
        painter.drawText(highScoreX, highScoreY, highScoreText);
    }

    // Restart instructions - IMPROVED ALIGNMENT
    QFont restartFont("Verdana", 18, QFont::Bold);
    painter.setFont(restartFont);

    QString restartText = "PRESS R TO RESTART";
    QFontMetrics restartMetrics(restartFont);
    int restartWidth = restartMetrics.horizontalAdvance(restartText);
    int restartX = (width() - restartWidth) / 2;
    int restartY = height() / 2 + 110;

    painter.setPen(QColor(200, 230, 255)); // Light blue
    painter.drawText(restartX, restartY, restartText);

    // Quit instructions - IMPROVED ALIGNMENT
    QFont quitFont("Arial", 14, QFont::Normal);
    painter.setFont(quitFont);

    QString quitText = "Press ESC to quit";
    QFontMetrics quitMetrics(quitFont);
    int quitWidth = quitMetrics.horizontalAdvance(quitText);
    int quitX = (width() - quitWidth) / 2;
    int quitY = restartY + 30;

    painter.setPen(QColor(180, 180, 180));
    painter.drawText(quitX, quitY, quitText);

    // Decorative lines - IMPROVED POSITIONING
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 50));

    int line1Width = 240;
    int line1X = (width() - line1Width) / 2;
    painter.drawRect(line1X, restartY - 20, line1Width, 2);

    int line2Width = 200;
    int line2X = (width() - line2Width) / 2;
    painter.drawRect(line2X, quitY + 10, line2Width, 1);
}

void DinoRunGame::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Space:
        if (gameState == START) {
            gameState = PLAYING;
            updateTimer->start();
        } else if (gameState == PLAYING && !dino.isJumping) {
            dino.isJumping = true;
            dino.velocity = -15.0f;
        }
        break;

    case Qt::Key_R:
        if (gameState == GAME_OVER || gameState == PLAYING) {
            gameState = PLAYING;
            initializeGame();
            initializeDino();
            // Generate initial trees
            for (int i = 0; i < 3; i++) {
                generateTree();
            }
            if (!updateTimer->isActive()) {
                updateTimer->start();
            }
        }
        break;

    case Qt::Key_Escape:
        close();
        break;

    default:
        QWidget::keyPressEvent(event);
    }
}
