//
// Created by esorochinskiy on 20.12.22.
//

#ifndef LINION_TERMINALWINDOW_H
#define LINION_TERMINALWINDOW_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class TerminalWindow; }
QT_END_NAMESPACE

class TerminalWindow : public QDialog {
Q_OBJECT

public:
    explicit TerminalWindow(QWidget *parent = nullptr);
    ~TerminalWindow() override;

    void setWIndowData(const QString &, const QString &) const;

private:
    Ui::TerminalWindow *ui;
};


#endif //LINION_TERMINALWINDOW_H
