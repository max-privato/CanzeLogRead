#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QTextStream>
#include <QTreeWidgetItem>

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

    void on_selectBtn_clicked();

    void on_unselectBtn_clicked();

private:
    int items,// numero di campi che scriverò nel file di uscita:
    lineCount;
    QStringList treeCodes, treeNames;
    QString inFileName,  initialLabel3Text;
    QFile * outFiles;
    QList<QTreeWidgetItem *> treeItems;

    QTextStream outStreams;
    //pari a quelli di input -2 in quanto gli ultimi 2 non li riporto
    Ui::MainWindow *ui;
    QString processLine(QString line_, double iniSecs_);
    void updateClickedState();

};
#endif // MAINWINDOW_H
