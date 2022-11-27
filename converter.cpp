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
        QDir().mkpath(unzippedDirectoryPath_);
    }

    if ((zip = zip_open(zipFilePath.c_str(), 0, &error)) == NULL) {
        zip_error_to_str(bufferStr, sizeof(bufferStr), error, errno);
        std::cout << "Can not open the zip file: " << error <<  ":\n" << bufferStr;
    }

    for (index = 0; index < zip_get_num_entries(zip, 0); index++) {
        if (zip_stat_index(zip, index, 0, &zipFileStats) == 0) {
            length = strlen(zipFileStats.name);

            // qDebug() << "Name:" << zipFileStats.name;

            if (zipFileStats.name[length - 1] == '/') {
                // qDebug() << "Making" << unzippedDirectoryPath_ + "/" + zipFileStats.name << "directory from zip file.";
                QDir().mkpath(unzippedDirectoryPath_ + "/" + zipFileStats.name);
            } else {
                zipFile = zip_fopen_index(zip, index, 0);

                if (!zipFile) {
                    qDebug() << "Can not open the file in zip archive.";
                    continue;
                }

                fd = open((unzippedDirectoryPath + "/" + zipFileStats.name).c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
                if (fd < 0) {
                    qDebug() << "Can not create the file (into which zipped data is to be extracted).";
                    continue;
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
void Converter::zipDirectory(QString srcDirPath_, QString zippedFilePath_) {
    std::string srcDirPath = srcDirPath_.toStdString();
    std::string zippedFilePath = zippedFilePath_.toStdString();

    QDir srcDir = QDir(srcDirPath_);

    char bufferStr[100];
    int error = 0;

    struct zip *zipArchive = zip_open(zippedFilePath.c_str(), ZIP_CREATE, &error);
    zip_source_t *zipBuffer;

    if (zipArchive == NULL) {
        zip_error_to_str(bufferStr, sizeof(bufferStr), error, errno);
        std::cout << "Can not create the zip file: " << error <<  ":\n" << bufferStr;
    }

    foreach (QFileInfo fileInfo, srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fileInfo.isFile()) {
            QFile file = QFile(fileInfo.absoluteFilePath());
            file.open(QFile::ReadOnly);

            zipBuffer = zip_source_buffer_create(file.readAll().toStdString().c_str(), file.readAll().length(), 0, 0);
            if (zipBuffer == NULL) {
                qDebug() << "Failed to create source buffer for" << fileInfo.fileName() + ".";
            }

            int fileIndex = zip_file_add(
                zipArchive,
                srcDir.relativeFilePath(fileInfo.canonicalFilePath()).toStdString().c_str(),
                zipBuffer,
                ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8
            );
            if (fileIndex < 0) {
                qDebug() << "Failed to add" << fileInfo.fileName() << "to archive.";
            }

            file.close();
        } else if (fileInfo.isDir()) {
            this->addDirectoryToZip(zipArchive, fileInfo, srcDir);
        }
    }

    if (zip_close(zipArchive) == -1) {
        qDebug() << "[DEBUG] Cannot close the zip file.";
    }
}
void Converter::addDirectoryToZip(struct zip* zipArchive, QFileInfo srcDirInfo, QDir rootSrcDir) {
    QDir srcDir = QDir(srcDirInfo.canonicalFilePath());

    zip_dir_add(zipArchive, rootSrcDir.relativeFilePath(srcDirInfo.canonicalFilePath()).toStdString().c_str(), ZIP_FL_ENC_UTF_8);

    zip_source_t *zipBuffer;

    foreach (QFileInfo fileInfo, srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fileInfo.isFile()) {
            QFile file = QFile(fileInfo.canonicalFilePath());
            file.open(QFile::ReadOnly);

            zipBuffer = zip_source_buffer_create(file.readAll().toStdString().c_str(), file.readAll().length(), 0, 0);
            if (zipBuffer == NULL) {
                qDebug() << "Failed to create source buffer for" << fileInfo.fileName() + ".";
            }

            int fileIndex = zip_file_add(
                zipArchive,
                rootSrcDir.relativeFilePath(fileInfo.canonicalFilePath()).toStdString().c_str(),
                zipBuffer,
                ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8
            );
            if (fileIndex < 0) {
                qDebug() << "Failed to add" << fileInfo.fileName() << "to archive.";
            }

            file.close();
        } else if (fileInfo.isDir()) {
            this->addDirectoryToZip(zipArchive, fileInfo, rootSrcDir);
        }
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

        QStringList _ = this->inputFilePath.split('/').last().split('.');
        _.removeLast();

        this->resourcePackName = _.join('/');
        this->unzipFile(this->inputFilePath, tmpDir.path() + "/Java Resource Pack");
    } else {
        qDebug() << "[DEBUG] Input resource pack type is folder.";

        this->resourcePackName = this->inputFilePath.split('/').last();
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
    qDebug() << "[DEBUG] Starting conversion.";
    foreach(const QFileInfo info, QDir(this->inputResourcePackPath).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, this->javaIdentityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, this->javaIdentityMap[info.fileName()]);
    }

    if (this->outputResourcePackType == 0) {
        qDebug() << "[DEBUG] Zipping the output resource pack into .mcpack zip archive.";
        this->zipDirectory(this->outputResourcePackPath, this->outputDirPath + "/FFDFD" + this->resourcePackName + ".zip");
    } else {
        // TODO:  copy directory
    }
}

void Converter::convertFile(QFileInfo fileInfo, QJsonValueRef identityRef) {
    if (identityRef.isNull() || identityRef.isUndefined()) {
        return;
    }
    QString identity = identityRef.toString();
    QString relBedrockPath = this->bedrockIdentityMap[identity].toString();

    QDir().mkpath(this->outputResourcePackPath + "/" + relBedrockPath.split('/').first(relBedrockPath.split('/').length() - 1).join('/'));

    QFile file = QFile(fileInfo.absoluteFilePath());
    file.copy(this->outputResourcePackPath + "/" + relBedrockPath);

    // qDebug() << "Converted:" << fileInfo.fileName() << "| ID:" << identity << " | To: " << this->outputResourcePackPath + "/" + this->bedrockIdentityMap[identity].toString();
}
void Converter::convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef) {
    // qDebug() << "Converting directory:" << dirInfo.absoluteFilePath();

    if (identityMapRef.isNull() || identityMapRef.isUndefined()) {
        return;
    }
    QJsonObject identityMap = identityMapRef.toObject();

    foreach(const QFileInfo info, QDir(dirInfo.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, identityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, identityMap[info.fileName()]);
    }
}
