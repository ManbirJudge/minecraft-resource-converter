#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include <converter.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void javaResourcePackBrowseBtnClicked();
    void bedrockResourcePackBrowseBtnClicked();

    void javaResourcePackTypeChanged(int index);
    void bedrockResourcePackTypeChanged(int index);

    void convertBtnClicked();

    void settingsBtnClicked();
    void aboutBtnClicked();
    void aboutDevBtnClicked();
    void helpBtnClicked();

private:
    Ui::MainWindow *ui;

    const QStringList JAVA_RESOURCE_PACK_TYPES = { "Zip File (.zip)", "Folder" };
    const QStringList BEDROCK_RESOURCE_PACK_TYPES = { "McPack (.mcpack)", "Folder" };

    QString javaResourcePackPath = "C:/Users/manbi/Downloads/Faithful 64x";
    QString bedrockResourcePackPath = "C:/Users/manbi/Downloads/GGLDFJD";

    QString javaResourcePackType = this->BEDROCK_RESOURCE_PACK_TYPES[0];
    QString bedrockResourcePackType = this->JAVA_RESOURCE_PACK_TYPES[0];
};
#endif
