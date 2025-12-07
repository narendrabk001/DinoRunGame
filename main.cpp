#include "DinoRunGame.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DinoRunGame game;
    game.show();

    return app.exec();
}
