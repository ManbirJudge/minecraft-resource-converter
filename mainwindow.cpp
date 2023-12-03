#include "mainwindow.h"
#include "ui_mainwindow.h"

// initialization
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // setting up ui
    ui->setupUi(this);

    ui->javaResPackTypeCombo->addItems(JAVA_RESOURCE_PACK_TYPES);
    ui->bedResPackTypeCombo->addItems(BEDROCK_RESOURCE_PACK_TYPES);
    ui->bedResPackMCMetaUUIDComboBox->addItems(BEDROCK_RESOURCE_PACK_MCMETA_UUID_OPTIONS);

    ui->javaResPackTypeCombo->setCurrentIndex(0);
    ui->bedResPackTypeCombo->setCurrentIndex(0);
    // ui->bedResPackMCMetaUUIDComboBox->setCurrentIndex(0);
    ui->bedResPackMCMetaUUIDComboBox->setCurrentIndex(1);
    bedResPackMCMetaUUIDTypeChanged(1);

    ui->javaResPackLineEdit->setText(javaResPackPath);
    ui->bedResPackLineEdit->setText(bedResPackOutputPath);

    ui->conversionStatusLabel->hide();
    ui->conversionStatusProgressBar->hide();

    setFixedHeight(sizeHint().height());

    // loading settings
    loadSettings();

    // event listeners
    connect(ui->javaResPackBrowseBtn, SIGNAL(clicked()), this, SLOT(javaResPackBrowseBtnClicked()));
    connect(ui->bedResPackBrowseBtn, SIGNAL(clicked()), this, SLOT(bedResPackBrowseBtnClicked()));

    connect(ui->javaResPackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(javaResPackTypeChanged(int)));
    connect(ui->bedResPackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(bedResPackTypeChanged(int)));
    connect(ui->bedResPackMCMetaUUIDComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(bedResPackMCMetaUUIDTypeChanged(int)));

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
            settings = settingsJsonDoc.object();
        }
    } else {
        settings.insert("show_conversion_warning_dialog", false);
        settings.insert("use_system_explorer", true);
        settings.insert("explorer_default_path", QDir::homePath());
    }
}

// event handlers
void MainWindow::javaResPackBrowseBtnClicked() {
    QString newPath;
    QFileDialog::Options dialogOptions;

    if (not settings.find("use_system_explorer")->toBool()) {
        dialogOptions = QFileDialog::Option::DontUseNativeDialog;
    }

    if (ui->javaResPackTypeCombo->currentIndex() == 0) {
        newPath = QFileDialog::getOpenFileName(
            this,
            "Select java resource pack",
            settings.find("explorer_default_path")->toString(),
            "Zip Files (*.zip);;All (*)",
            nullptr,
            dialogOptions
        );
    } else {
        newPath = QFileDialog::getExistingDirectory(
            this,
            "Select java resource pack",
            settings.find("explorer_default_path")->toString(),
            dialogOptions
        );
    }

    if (!newPath.isEmpty() && !newPath.isNull()) {
        javaResPackPath = newPath;
        ui->javaResPackLineEdit->setText(javaResPackPath);
    }
}
void MainWindow::bedResPackBrowseBtnClicked() {
    QFileDialog::Options dialogOptions;
    if (not settings.find("use_system_explorer")->toBool()) {
        dialogOptions = QFileDialog::Option::DontUseNativeDialog;
    }

    QString newPath = QFileDialog::getExistingDirectory(
        this,
        "Select output directory",
        settings.find("default_explorer_path")->toString(),
        dialogOptions
    );

    if (!newPath.isEmpty() && !newPath.isNull()) {
        bedResPackOutputPath = newPath;
        ui->bedResPackLineEdit->setText(bedResPackOutputPath);
    }
}

void MainWindow::javaResPackTypeChanged(int index) {
    javaResPackType = JAVA_RESOURCE_PACK_TYPES[index];
}
void MainWindow::bedResPackTypeChanged(int index) {
    bedResPackType = BEDROCK_RESOURCE_PACK_TYPES[index];
}
void MainWindow::bedResPackMCMetaUUIDTypeChanged(int index) {
    if (index == 0) ui->bedResPackMCMetaUUIDLineEdit->setEnabled(false);
    else ui->bedResPackMCMetaUUIDLineEdit->setEnabled(true);
}

// conversion functions
void MainWindow::convertBtnClicked() {
    if (settings["show_conversion_warning_dialog"].toBool()) {
        QMessageBox* warningDialog = new QMessageBox();

        warningDialog->setWindowTitle("Warning");
        warningDialog->setText("The programme might not work or might break your game or some textures might not get converted as intended. Use it at your own risk.");
        warningDialog->setInformativeText("Do you want to continue?");
        warningDialog->setStandardButtons(QMessageBox::Button::Yes | QMessageBox::Button::No);
        warningDialog->setIcon(QMessageBox::Icon::Warning);

        if (warningDialog->exec() == QMessageBox::DialogCode::Accepted) {
            startConversion();
        }

    } else {
        startConversion();
    }
}
void MainWindow::startConversion() {
    Converter *converter = new Converter(
        javaResPackPath,
        bedResPackOutputPath,
        ui->javaResPackTypeCombo->currentIndex(),
        ui->bedResPackTypeCombo->currentIndex(),
        ui->bedResPackMCMetaUUIDComboBox->currentIndex(),
        ui->bedResPackMCMetaUUIDLineEdit->text()
    );
    converter->startConversion();
    delete converter;
}

// help/utility buttons
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
    loadSettings();
}

// destruction
MainWindow::~MainWindow()
{
    delete ui;
}

