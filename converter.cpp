#include "converter.h"

Converter::Converter(
    QString inputFilePath_,
    QString outputDirPath_,

    int inputResourcePackType_,
    int outputResourcePackType_
) : inputFilePath(inputFilePath_),
    outputDirPath(outputDirPath_),
    inputResourcePackType(inputResourcePackType_),
    outputResourcePackType(outputResourcePackType_)
{
    if (this->inputFilePath.isEmpty() or this->inputFilePath.isNull()) throw std::invalid_argument("Input file path was not given.");
    if (this->outputDirPath.isEmpty() or this->outputDirPath.isNull()) throw std::invalid_argument("Output directory was not given.");

    this->loadData();
}

void Converter::unzipFile(QString zipFilePath_, QString unzippedDirectoryPath_) {
    std::string zipFilePath = zipFilePath_.toStdString();
    std::string unzippedDirectoryPath = unzippedDirectoryPath_.toStdString();

    struct zip *zip;
    struct zip_file *zipFile;
    struct zip_stat zipFileStats;

    char bufferStr[100];
    int error;
    int index, length;
    int fd;
    long long sum;

    if (not QDir().exists(unzippedDirectoryPath_)) {
        QDir().mkdir(unzippedDirectoryPath_);
    }

    if ((zip = zip_open(zipFilePath.c_str(), 0, &error)) == NULL) {
        zip_error_to_str(bufferStr, sizeof(bufferStr), error, errno);
        std::cout << "Can not open the zip file: " << error <<  ":\n" << bufferStr;
    }

    for (index = 0; index < zip_get_num_entries(zip, 0); index++) {
        if (zip_stat_index(zip, index, 0, &zipFileStats) == 0) {
            length = strlen(zipFileStats.name);

            // qDebug() << "==================";
            // qDebug() << "Name:" << zipFileStats.name;
            // qDebug() << "Size: " << zipFileStats.size;

            if (zipFileStats.name[length - 1] == '/') {
                QDir().mkdir(unzippedDirectoryPath_ + "/" + zipFileStats.name);
            } else {
                zipFile = zip_fopen_index(zip, index, 0);
                if (!zipFile) {
                    fprintf(stderr, "boese, boese/n");
                    exit(100);
                }

                fd = open((unzippedDirectoryPath + "/" + zipFileStats.name).c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
                if (fd < 0) {
                    fprintf(stderr, "boese, boese/n");
                    exit(101);
                }

                sum = 0;
                while (sum != (long long) zipFileStats.size) {
                    length = zip_fread(zipFile, bufferStr, 100);
                    if (length < 0) {
                        fprintf(stderr, "boese, boese/n");
                        exit(102);
                    }
                    write(fd, bufferStr, length);
                    sum += length;
                }


                close(fd);
                zip_fclose(zipFile);
            }
        } else {
            printf("File[%s] Line[%d]/n", __FILE__, __LINE__);
        }
    }

    if (zip_close(zip) == -1) {
        qDebug() << "[DEBUG] Cannot close the zip file.";
    }
}
void Converter::copyDir(QString srcPath, QString dstPath) {
    QDir srcDir (srcPath);
    QDir parentDstDir(QFileInfo(dstPath).path());

    parentDstDir.mkdir(QFileInfo(dstPath).fileName());

    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();

        if (info.isDir()) {
            copyDir(srcItemPath, dstItemPath);
        } else if (info.isFile()) {
            QFile::copy(srcItemPath, dstItemPath);
        }
    }
}

void Converter::loadData() {
    // making temperary directory
    QDir().rmdir("tmp");
    QDir().mkdir("tmp");

    QDir tmpDir = QDir("tmp");

    // loading input resource pack into temperary directory
    this->inputResourcePackPath = tmpDir.path() + "/Java Resource Pack";
    if (this->inputResourcePackType == 0) {
        qDebug() << "[DEBUG] Input resource pack type is zip file.";
        this->unzipFile(this->inputFilePath, tmpDir.path() + "/Java Resource Pack");
    } else {
        qDebug() << "[DEBUG] Input resource pack type is folder.";
        this->copyDir(this->inputFilePath, tmpDir.path() + "/Java Resource Pack");
    }

    // output resource pack stuff
    this->outputResourcePackPath = tmpDir.path() + "/Bedrock Resource Pack";
    QDir().mkdir(this->outputResourcePackPath);

    // loading resource pack configuration
    QFile packConfigFile = QFile(this->inputResourcePackPath + "/pack.mcmeta");
    packConfigFile.open(QFile::ReadOnly);
    QJsonDocument packConfigJsonDoc = QJsonDocument::fromJson(packConfigFile.readAll());
    packConfigFile.close();

    if (packConfigJsonDoc.isObject()) {
        this->resourcePackConfig = packConfigJsonDoc.object();
    }

    this->resourcePackName = this->resourcePackConfig["pack"].toObject()["pack_format"].toString();
    this->resourcePackConfigFormat = this->resourcePackConfig["pack"].toObject()["pack_format"].toInt(0);

    // loading conversion pattern
    this->loadIdentityPatterns();
}
void Converter::loadIdentityPatterns() {
    QFile javaIdentityMapFile = QFile("identity_pattern_java_" + QString::number(this->resourcePackConfigFormat) + ".json");
    QFile bedrockIdentityMapFile = QFile("identity_pattern_bedrock_" + QString::number(this->resourcePackConfigFormat) + ".json");

    javaIdentityMapFile.open(QFile::ReadOnly);
    bedrockIdentityMapFile.open(QFile::ReadOnly);

    QJsonDocument javaIdentityMapJsonDoc = QJsonDocument::fromJson(javaIdentityMapFile.readAll());
    QJsonDocument bedrockIdentityMapJsonDoc = QJsonDocument::fromJson(bedrockIdentityMapFile.readAll());

    javaIdentityMapFile.close();
    bedrockIdentityMapFile.close();

    if (javaIdentityMapJsonDoc.isObject()) {
        this->javaIdentityMap = javaIdentityMapJsonDoc.object();
    }
    if (bedrockIdentityMapJsonDoc.isObject()) {
        this->bedrockIdentityMap = bedrockIdentityMapJsonDoc.object();
    }
}

void Converter::startConversion() {
    foreach(const QFileInfo info, QDir(this->inputResourcePackPath).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, this->javaIdentityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, this->javaIdentityMap[info.fileName()]);
    }
}

void Converter::convertFile(QFileInfo fileInfo, QJsonValueRef identityRef) {
    if (identityRef.isNull() || identityRef.isUndefined()) {
        return;
    }
    QString identity = identityRef.toString();

    qDebug() << "Converting file:" << fileInfo.fileName() << "| ID:" << identity;
}
void Converter::convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef) {
    qDebug() << "Converting directory:" << dirInfo.absoluteFilePath();

    if (identityMapRef.isNull() || identityMapRef.isUndefined()) {
        return;
    }
    QJsonObject identityMap = identityMapRef.toObject();

    foreach(const QFileInfo info, QDir(dirInfo.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, identityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, identityMap[info.fileName()]);
    }
}
