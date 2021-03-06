#include "core/appevents.h"
#include "core/appconfig.h"
#include "gui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    qRegisterMetaType<PredictionResult>("PredictionResult");
    qRegisterMetaType<ImageResult>("ImageResult");

    QApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath());
    app.setApplicationVersion("1.1.0.0");
    app.setApplicationName("Collaboratively optimizing deep learning via Collective Knowledge");

    AppEvents::instance()->init();

    qApp->setStyleSheet(AppConfig::styleSheet());

    (new MainWindow())->show();

    return app.exec();
}
