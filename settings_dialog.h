#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
class settings_dialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

public slots:
    void explorerDefaultPathBrowseBtnClicked();
    void saveSettingsBtnClicked();

private:
    void loadSettings();

    Ui::settings_dialog *ui;
};

#endif
