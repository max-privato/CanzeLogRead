#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    ~MainWindow() override;

private slots:
    void on_okButton_clicked();
    void on_batCurrCB_clicked();
    void on_batVoltCB_clicked();
    void on_eleBrakeTorque1CB_clicked();
    void on_coastTorqueCB_clicked();

    void on_vhSpeedCB_clicked();

    void on_batSocCB_clicked();

    void on_eleBrakeTorque2CB_clicked();

private:
    int items,// numero di campi che scriverò nel file di uscita:
    lineCount;
    QString inFileName,  initialLabel3Text;
    QFile * outFiles;
    QTextStream outStreams;
    //pari a quelli di input -2 in quanto gli ultimi 2 non li riporto
    Ui::MainWindow *ui;
    QByteArray processLine(QByteArray line_, double iniSecs_);
    void updateClickedState();

};
#endif // MAINWINDOW_H
