#include "settings_dialog.h"
#include "ui_settings_dialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_dialog)
{
    ui->setupUi(this);

    this->setFixedHeight(sizeHint().height());

    connect(ui->explorerDefaultPathSelectBtn, SIGNAL(clicked()), this, SLOT(explorerDefaultPathBrowseBtnClicked()));
    connect(ui->saveSettingsBtn, SIGNAL(clicked()), this, SLOT(saveSettingsBtnClicked()));

    this->loadSettings();
}

void SettingsDialog::explorerDefaultPathBrowseBtnClicked() {
    QString newPath = QFileDialog::getExistingDirectory(this, "Select output directory");

    if (!newPath.isEmpty() && !newPath.isNull()) {
        ui->explorerDefaultPathEdit->setText(newPath);
    }
}
void SettingsDialog::saveSettingsBtnClicked() {
    QFile settingsJsonFile = QFile("settigns.json");
    QJsonObject settingsJson;

    settingsJson.insert("show_conversion_warning_dialog", ui->showConversionWarningDialogBox->isChecked());
    settingsJson.insert("use_system_explorer", ui->useSystemExplorerBox->isChecked());
    settingsJson.insert("explorer_default_path", ui->explorerDefaultPathEdit->text());

    QJsonDocument settingsJsonDoc (settingsJson);

    settingsJsonFile.open(QFile::WriteOnly);
    settingsJsonFile.write(settingsJsonDoc.toJson());
    settingsJsonFile.close();

    this->close();
}

void SettingsDialog::loadSettings() {
    QFile settingsJsonFile = QFile("settigns.json");

    if (settingsJsonFile.open(QFile::ReadOnly)) {
        QJsonDocument settingsJsonDoc = QJsonDocument::fromJson(settingsJsonFile.readAll());

        if (settingsJsonDoc.isObject()) {
            QJsonObject settingsJson = settingsJsonDoc.object();

            ui->showConversionWarningDialogBox->setChecked(settingsJson.find("show_conversion_warning_dialog")->toBool());
            ui->useSystemExplorerBox->setChecked(settingsJson.find("use_system_explorer")->toBool());
            ui->explorerDefaultPathEdit->setText(settingsJson.find("explorer_default_path")->toString());
        }
    } else {
        ui->showConversionWarningDialogBox->setChecked(false);
        ui->useSystemExplorerBox->setChecked(true);
        ui->explorerDefaultPathEdit->setText(QDir::homePath());
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
