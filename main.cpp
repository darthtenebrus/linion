#include <QApplication>
#include <QTranslator>
#include <mainwindow.h>
int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

    QTranslator mainTranslator;
    // look up e.g. :/i18n/myapp_de.qm
    if (mainTranslator.load(
            QLocale(),
            QLatin1String("mainwindow"),
            QLatin1String("_"),
            QLatin1String(":/i18n"))) {
        QApplication::installTranslator(&mainTranslator);
    }

    auto *w(new MainWindow());
    w->show();
    int res = QApplication::exec();
    delete w;
    return res;

}
