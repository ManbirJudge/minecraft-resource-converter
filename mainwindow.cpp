#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    // setting up ui
    ui->setupUi(this);

    ui->javaResourcePackTypeCombo->addItems(this->JAVA_RESOURCE_PACK_TYPES);
    ui->bedrockResourcePackTypeCombo->addItems(this->BEDROCK_RESOURCE_PACK_TYPES);
    ui->bedrockResourcePackMCMetaUUIDComboBox->addItems(this->BEDROCK_RESOURCE_PACK_MCMETA_UUID_OPTIONS);

    ui->javaResourcePackTypeCombo->setCurrentIndex(0);
    ui->bedrockResourcePackTypeCombo->setCurrentIndex(0);
    ui->bedrockResourcePackMCMetaUUIDComboBox->setCurrentIndex(1);  // TEMP
    this->bedrockResourcePackMCMetaUUIDTypeChanged(1);  // TEMP

    ui->javaResourcePackLineEdit->setText(this->javaResourcePackPath);
    ui->bedrockResourcePackLineEdit->setText(this->bedrockResourcePackOutputPath);

    ui->conversionStatusLabel->hide();
    ui->conversionStatusProgressBar->hide();

    this->setFixedHeight(sizeHint().height());

    // loading settings
    this->loadSettings();

    // event listeners
    connect(ui->javaResourcePackBrowseBtn, SIGNAL(clicked()), this, SLOT(javaResourcePackBrowseBtnClicked()));
    connect(ui->bedrockResourcePackBrowseBtn, SIGNAL(clicked()), this, SLOT(bedrockResourcePackBrowseBtnClicked()));

    connect(ui->javaResourcePackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(javaResourcePackTypeChanged(int)));
    connect(ui->bedrockResourcePackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(bedrockResourcePackTypeChanged(int)));
    connect(ui->bedrockResourcePackMCMetaUUIDComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(bedrockResourcePackMCMetaUUIDTypeChanged(int)));

    connect(ui->startConversionBtn, SIGNAL(clicked()), this, SLOT(convertBtnClicked()));

    connect(ui->settingsBtn, SIGNAL(clicked()), this, SLOT(settingsBtnClicked()));
    connect(ui->aboutBtn, SIGNAL(clicked()), this, SLOT(aboutBtnClicked()));
    connect(ui->aboutDevBtn, SIGNAL(clicked()), this, SLOT(aboutDevBtnClicked()));
    connect(ui->helpBtn, SIGNAL(clicked()), this, SLOT(helpBtnClicked()));
}

void MainWindow::loadSettings() {
    QFile settingsJsonFile = QFile("settigns.json");

    if (settingsJsonFile.open(QFile::ReadOnly)) {
        QJsonDocument settingsJsonDoc = QJsonDocument::fromJson(settingsJsonFile.readAll());

        if (settingsJsonDoc.isObject()) {
            this->settings = settingsJsonDoc.object();
        }
    } else {
        this->settings.insert("show_conversion_warning_dialog", false);
        this->settings.insert("use_system_explorer", true);
        this->settings.insert("explorer_default_path", QDir::homePath());
    }
}

void MainWindow::javaResourcePackBrowseBtnClicked() {
    QString newPath;
    QFileDialog::Options dialogOptions;

    if (not this->settings.find("use_system_explorer")->toBool()) {
        dialogOptions = QFileDialog::Option::DontUseNativeDialog;
    }

    if (ui->javaResourcePackTypeCombo->currentIndex() == 0) {
        newPath = QFileDialog::getOpenFileName(
            this,
            "Select java resource pack",
            this->settings.find("explorer_default_path")->toString(),
            "Zip Files (*.zip);;All (*)",
            nullptr,
            dialogOptions
        );
    } else {
        newPath = QFileDialog::getExistingDirectory(
            this,
            "Select java resource pack",
            this->settings.find("explorer_default_path")->toString(),
            dialogOptions
        );
    }

    if (!newPath.isEmpty() && !newPath.isNull()) {
        javaResourcePackPath = newPath;
        ui->javaResourcePackLineEdit->setText(javaResourcePackPath);
    }
}
void MainWindow::bedrockResourcePackBrowseBtnClicked() {
    QFileDialog::Options dialogOptions;
    if (not this->settings.find("use_system_explorer")->toBool()) {
        dialogOptions = QFileDialog::Option::DontUseNativeDialog;
    }

    QString newPath = QFileDialog::getExistingDirectory(
        this,
        "Select output directory",
        this->settings.find("default_explorer_path")->toString(),
        dialogOptions
    );

    if (!newPath.isEmpty() && !newPath.isNull()) {
        bedrockResourcePackOutputPath = newPath;
        ui->bedrockResourcePackLineEdit->setText(bedrockResourcePackOutputPath);
    }
}

void MainWindow::javaResourcePackTypeChanged(int index) {
    this->javaResourcePackType = this->JAVA_RESOURCE_PACK_TYPES[index];
}
void MainWindow::bedrockResourcePackTypeChanged(int index) {
    this->bedrockResourcePackType = this->BEDROCK_RESOURCE_PACK_TYPES[index];
}
void MainWindow::bedrockResourcePackMCMetaUUIDTypeChanged(int index) {
    if (index == 0) ui->bedrockResourcePackMCMetaUUIDLineEdit->setEnabled(false);
    else ui->bedrockResourcePackMCMetaUUIDLineEdit->setEnabled(true);
}

void MainWindow::convertBtnClicked() {
    if (this->settings["show_conversion_warning_dialog"].toBool()) {
        QMessageBox* warningDialog = new QMessageBox();

        warningDialog->setWindowTitle("Warning");
        warningDialog->setText("The programme might not work or might break your game and some textures might not get converted as intended. Use it at your own risk.");
        warningDialog->setInformativeText("Do you want to continue?");
        warningDialog->setStandardButtons(QMessageBox::Button::Yes | QMessageBox::Button::No);
        warningDialog->setIcon(QMessageBox::Icon::Warning);

        if (warningDialog->exec() == QMessageBox::DialogCode::Accepted) {
            Converter *converter = new Converter(
                this->javaResourcePackPath,
                this->bedrockResourcePackOutputPath,
                ui->javaResourcePackTypeCombo->currentIndex(),
                ui->bedrockResourcePackTypeCombo->currentIndex(),
                ui->bedrockResourcePackMCMetaUUIDComboBox->currentIndex(),
                ui->bedrockResourcePackMCMetaUUIDLineEdit->text()
            );
            converter->startConversion();
            delete converter;
        }

    } else {
        Converter *converter = new Converter(
            this->javaResourcePackPath,
            this->bedrockResourcePackOutputPath,
            ui->javaResourcePackTypeCombo->currentIndex(),
            ui->bedrockResourcePackTypeCombo->currentIndex(),
            ui->bedrockResourcePackMCMetaUUIDComboBox->currentIndex(),
            ui->bedrockResourcePackMCMetaUUIDLineEdit->text()
        );
        converter->startConversion();
        delete converter;
    }
}

void MainWindow::settingsBtnClicked() {
    SettingsDialog* settingsDialog = new SettingsDialog(this);

    connect(settingsDialog, SIGNAL(finished(int)), this, SLOT(settingsDialogClosed(int)));

    settingsDialog->show();
}
void MainWindow::aboutBtnClicked() {
    qDebug() << "About button clicked.";
}
void MainWindow::aboutDevBtnClicked() {
    qDebug() << "About developer button clicked.";
}
void MainWindow::helpBtnClicked() {
    qDebug() << "Help button clicked.";
}
void MainWindow::settingsDialogClosed(int result) {
    qDebug() << "[DEBUG] Settings dialog closed with result:" << result;
    this->loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

