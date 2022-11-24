#include <QApplication>
#include <QTranslator>
#include <mainwindow.h>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <QAbstractItemModel>

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

    const QString &semaph_id = "linion_semaphore";
    const QString &shared_id = "linion_shared_mem";
    QSystemSemaphore semaphore(semaph_id, 1);
    semaphore.acquire();

#ifndef Q_OS_WIN32
    QSharedMemory nix_fix_shared_memory(shared_id);
    if (nix_fix_shared_memory.attach())
    {
        nix_fix_shared_memory.detach();
    }
#endif

    QSharedMemory sharedMemory(shared_id);
    bool is_running = false;
    if (sharedMemory.attach())
    {
        is_running = true;
    } else {
        sharedMemory.create(1);
    }
    semaphore.release();

    if (is_running) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(QObject::tr("Already Running"));
        msgBox.setText(QObject::tr("Linion Already Running"));
        msgBox.exec();
        return 1;
    }


    auto *w(new MainWindow());
    w->show();
    emit w->doRefresh();
    int res = QApplication::exec();
    delete w;
    return res;

}
