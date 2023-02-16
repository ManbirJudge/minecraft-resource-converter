#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

#include <converter.h>
#include <settings_dialog.h>

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
    void bedrockResourcePackMCMetaUUIDTypeChanged(int index);

    void convertBtnClicked();

    void settingsBtnClicked();
    void aboutBtnClicked();
    void aboutDevBtnClicked();
    void helpBtnClicked();

    void settingsDialogClosed(int result);

private:
    void loadSettings();

    Ui::MainWindow *ui;

    const QStringList JAVA_RESOURCE_PACK_TYPES = { "Zip File (.zip)", "Folder" };
    const QStringList BEDROCK_RESOURCE_PACK_TYPES = { "McPack (.mcpack)", "Folder" };
    const QStringList BEDROCK_RESOURCE_PACK_MCMETA_UUID_OPTIONS = { "Random", "Custom" };

    QString javaResourcePackPath = "C:/Users/manbi/Downloads/Faithful 64x.zip";  // TEMP
    QString bedrockResourcePackOutputPath = "D:/Manbir";  // TEMP

    QString javaResourcePackType = this->BEDROCK_RESOURCE_PACK_TYPES[0];
    QString bedrockResourcePackType = this->JAVA_RESOURCE_PACK_TYPES[0];

    QJsonObject settings;
};
#endif
