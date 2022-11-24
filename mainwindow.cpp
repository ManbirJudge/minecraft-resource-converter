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

    ui->javaResourcePackTypeCombo->setCurrentIndex(1);
    ui->bedrockResourcePackTypeCombo->setCurrentIndex(0);

    ui->javaResourcePackLineEdit->setText(this->javaResourcePackPath);
    ui->bedrockResourcePackLineEdit->setText(this->bedrockResourcePackPath);

    ui->conversionStatusLabel->hide();
    ui->conversionStatusProgressBar->hide();

    // event listeners
    connect(ui->javaResourcePackBrowseBtn, SIGNAL(clicked()), this, SLOT(javaResourcePackBrowseBtnClicked()));
    connect(ui->bedrockResourcePackBrowseBtn, SIGNAL(clicked()), this, SLOT(bedrockResourcePackBrowseBtnClicked()));

    connect(ui->javaResourcePackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(javaResourcePackTypeChanged(int)));
    connect(ui->bedrockResourcePackTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(bedrockResourcePackTypeChanged(int)));

    connect(ui->startConversionBtn, SIGNAL(clicked()), this, SLOT(convertBtnClicked()));

    connect(ui->settingsBtn, SIGNAL(clicked()), this, SLOT(settingsBtnClicked()));
    connect(ui->aboutBtn, SIGNAL(clicked()), this, SLOT(aboutBtnClicked()));
    connect(ui->aboutDevBtn, SIGNAL(clicked()), this, SLOT(aboutDevBtnClicked()));
    connect(ui->helpBtn, SIGNAL(clicked()), this, SLOT(helpBtnClicked()));
}

void MainWindow::javaResourcePackBrowseBtnClicked() {
    QString newPath;

    if (ui->javaResourcePackTypeCombo->currentIndex() == 0) {
        newPath = QFileDialog::getOpenFileName(
            this,
            "Select java resource pack",
            QString(),
            "Zip Files (*.zip);;All (*)"
        );
    } else {
        newPath = QFileDialog::getExistingDirectory(
            this,
            "Select java resource pack"
        );
    }

    if (!newPath.isEmpty() && !newPath.isNull()) {
        javaResourcePackPath = newPath;
        ui->javaResourcePackLineEdit->setText(javaResourcePackPath);
    }
}
void MainWindow::bedrockResourcePackBrowseBtnClicked() {
    QString newPath = QFileDialog::getExistingDirectory(
        this,
        "Select output directory"
    );

    if (!newPath.isEmpty() && !newPath.isNull()) {
        bedrockResourcePackPath = newPath;
        ui->bedrockResourcePackLineEdit->setText(bedrockResourcePackPath);
    }
}

void MainWindow::javaResourcePackTypeChanged(int index) {
    this->javaResourcePackType = this->JAVA_RESOURCE_PACK_TYPES[index];
}
void MainWindow::bedrockResourcePackTypeChanged(int index) {
    this->bedrockResourcePackType = this->BEDROCK_RESOURCE_PACK_TYPES[index];
}

void MainWindow::convertBtnClicked() {
    Converter *converter = new Converter(this->javaResourcePackPath, this->bedrockResourcePackPath, ui->javaResourcePackTypeCombo->currentIndex(), ui->bedrockResourcePackTypeCombo->currentIndex());
    converter->startConversion();
    delete converter;
}

void MainWindow::settingsBtnClicked() {

    qDebug() << "Settings button clicked.";
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

MainWindow::~MainWindow()
{
    delete ui;
}

