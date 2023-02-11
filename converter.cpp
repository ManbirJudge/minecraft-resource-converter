#include "converter.h"

std::string get_uuid() {
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

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

void Converter::unzipFile(QString srcFilePath_, QString destinationDirectoryPath_) {
    std::string srcFilePath = srcFilePath_.toStdString();
    std::string destinationDirectoryPath = destinationDirectoryPath_.toStdString();

    char bufferStr[100];
    int error = 0;
    int fileHandle;

    struct zip *zipArchive = zip_open(srcFilePath.c_str(), 0, &error);
    struct zip_file *zippedFile;
    struct zip_stat zippedFileStats;

    if (not QDir().exists(destinationDirectoryPath_)) {
        QDir().mkpath(destinationDirectoryPath_);
    }

    if (zipArchive == NULL) {
        zip_error_to_str(bufferStr, sizeof(bufferStr), error, errno);
        std::cout << "[ERROR] Can not open the zip file: " << error <<  ":\n" << bufferStr;
    }

    for (int index = 0; index < zip_get_num_entries(zipArchive, 0); index++) {
        if (zip_stat_index(zipArchive, index, 0, &zippedFileStats) == 0) {
            int zipFileNameLength = strlen(zippedFileStats.name);

            if (zippedFileStats.name[zipFileNameLength - 1] == '/') {  // i.e. folder
                QDir().mkpath(destinationDirectoryPath_ + "/" + zippedFileStats.name);
            } else {  // i.e. file
                QDir().mkpath(destinationDirectoryPath_ + "/" + QString(zippedFileStats.name).split("/").first(QString(zippedFileStats.name).split("/").length() - 1).join("/"));

                zippedFile = zip_fopen_index(zipArchive, index, 0);
                if (zippedFile == NULL) {
                    qDebug() << "[ERROR] Can not open the file in zip archive.";
                    continue;
                }

                fileHandle = open((destinationDirectoryPath + "/" + zippedFileStats.name).c_str(), O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
                if (fileHandle < 0) {
                    qDebug() << "[ERROR] Can not create the file (into which zipped data is to be extracted).";
                    continue;
                }

                // zip_uint64_t totalFileDataLength = 0;
                while (true) {
                    zip_int64_t fileDataLength = zip_fread(zippedFile, bufferStr, 100);
                    if (fileDataLength == 0) { // EOF character
                        break;
                    }

                    if (fileDataLength < 0) {
                        qDebug() << "[ERROR] Can not read the zipped file.";
                        exit(1);
                    }

                    write(fileHandle, bufferStr, (size_t) fileDataLength);
                }

                close(fileHandle);
                zip_fclose(zippedFile);
            }
        } else {
            qDebug() << "IDK what is here 🫥.";
        }
    }

    if (zip_close(zipArchive) == -1) {
        qDebug() << "[ERROR] Cannot close the zip file.";
    }
}
void Converter::zipDirectory(QString srcDirPath_, QString zippedFilePath_) {
    std::string srcDirPath = srcDirPath_.toStdString();
    std::string zippedFilePath = zippedFilePath_.toStdString();

    QDir srcDir = QDir(srcDirPath_);

    char bufferStr[100];
    int error = 0;

    zip_t* zipArchive = zip_open(zippedFilePath.c_str(), ZIP_CREATE, &error);
    if (zipArchive == NULL) {
        zip_error_to_str(bufferStr, sizeof(bufferStr), error, errno);
        std::cout << "[ERROR] Can not create the zip file: " << error <<  ":\n" << bufferStr;
    }

    zip_source_t* zipSource;

    foreach (QFileInfo fileInfo, srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fileInfo.isFile()) {
            zipSource = zip_source_file(zipArchive, fileInfo.canonicalFilePath().toStdString().c_str(), 0, 0);
            if (zipSource == NULL) {
                qDebug() << "[ERROR] Failed to create source buffer for" << fileInfo.fileName() + ".";
                continue;
            }

            int fileIndex = zip_file_add(
                zipArchive,
                srcDir.relativeFilePath(fileInfo.canonicalFilePath()).toStdString().c_str(),
                zipSource,
                ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8
            );
            if (fileIndex < 0) {
                qDebug() << "[ERROR] Failed to add" << fileInfo.fileName() << "to archive.";
                continue;
            }
        } else if (fileInfo.isDir()) {
            this->addDirectoryToZip(zipArchive, zipSource, fileInfo, srcDir);
        }
    }

    if (zip_close(zipArchive) == -1) {
        qDebug() << "[DEBUG] Cannot close the zip file.";
    }
}
void Converter::addDirectoryToZip(zip_t* zipArchive, zip_source_t* zipSource, QFileInfo srcDirInfo, QDir rootSrcDir) {
    QDir srcDir = QDir(srcDirInfo.canonicalFilePath());

    zip_dir_add(zipArchive, rootSrcDir.relativeFilePath(srcDirInfo.canonicalFilePath()).toStdString().c_str(), ZIP_FL_ENC_UTF_8);

    foreach (QFileInfo fileInfo, srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fileInfo.isFile()) {
            zipSource = zip_source_file(zipArchive, fileInfo.canonicalFilePath().toStdString().c_str(), 0, 0);
            if (zipSource == NULL) {
                qDebug() << "Failed to create source buffer for" << fileInfo.fileName() + ".";
                continue;
            }

            int fileIndex = zip_file_add(
                zipArchive,
                rootSrcDir.relativeFilePath(fileInfo.canonicalFilePath()).toStdString().c_str(),
                zipSource,
                ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8
            );
            if (fileIndex < 0) {
                qDebug() << "Failed to add" << fileInfo.fileName() << "to archive.";
                continue;
            }

        } else if (fileInfo.isDir()) {
            this->addDirectoryToZip(zipArchive, zipSource, fileInfo, rootSrcDir);
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
    QDir tmpDir = QDir("tmp");
    qDebug() << "[DEBUG] Removed temperary directory with result:" << tmpDir.removeRecursively();
    QDir().mkdir("tmp");

    // loading input resource pack into temperary directory
    this->inputResourcePackTempPath = tmpDir.path() + "/Java Resource Pack";
    if (this->inputResourcePackType == 0) {
        qDebug() << "[DEBUG] Input resource pack type is zip file.";

        QStringList _ = this->inputFilePath.split('/').last().split('.');
        _.removeLast();

        qDebug() << "[DEBUG] Unzipping input resource pack to temperary directory.";
        this->resourcePackName = _.join('/');
        this->unzipFile(this->inputFilePath, tmpDir.path() + "/Java Resource Pack");
    } else {
        qDebug() << "[DEBUG] Input resource pack type is folderI would say that.";

        qDebug() << "[DEBUG] Coppying input resource pack to temperary directory.";
        this->resourcePackName = this->inputFilePath.split('/').last();
        this->copyDir(this->inputFilePath, tmpDir.path() + "/Java Resource Pack");
    }

    // output resource pack stuff
    this->outputResourcePackTempPath = tmpDir.path() + "/Bedrock Resource Pack";
    QDir().mkdir(this->outputResourcePackTempPath);

    // loading resource pack configuration
    QFile packConfigFile = QFile(this->inputResourcePackTempPath + "/pack.mcmeta");
    packConfigFile.open(QFile::ReadOnly);
    QJsonDocument packConfigJsonDoc = QJsonDocument::fromJson(packConfigFile.readAll());
    packConfigFile.close();

    if (packConfigJsonDoc.isObject()) {
        this->resourcePackConfig = packConfigJsonDoc.object();
    }

    // TODO: this->resourcePackConfigFormat = this->resourcePackConfig["pack"].toObject()["pack_format"].toInt(0);
    this->resourcePackDesc = this->resourcePackConfig["pack"].toObject()["description"].toString();

    // loading conversion pattern
    this->loadIdentityPatterns();
}
void Converter::loadIdentityPatterns() {
    QFile javaIdentityMapFile = QFile(":/identity_maps_java/assets/identity_maps/identity_maps_java/identity_map_java_" + QString::number(this->resourcePackConfigFormat) + ".json");
    QFile bedrockIdentityMapFile = QFile(":/identity_maps_bedrock/assets/identity_maps/identity_maps_bedrock/identity_map_bedrock_" + QString::number(this->resourcePackConfigFormat) + ".json");

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

    this->convert();

    if (this->outputResourcePackType == 0) {
        qDebug() << "[DEBUG] Zipping the output resource pack into .mcpack zip archive.";
        this->zipDirectory(this->outputResourcePackTempPath, this->outputDirPath + "/" + this->resourcePackName + " (converted).mcpack");
    } else {
        qDebug() << "[DEBUG] Copying the output resource pack to output directory.";
        this->copyDir(this->outputResourcePackTempPath, this->outputDirPath + "/" + this->resourcePackName + " (converted)");
    }

    qDebug() << "[DEBUG] Conversion done.";
}

void Converter::convert() {
    QDir inputResourcePactTempDir = QDir(this->inputResourcePackTempPath);

    foreach(const QFileInfo info, inputResourcePactTempDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, this->javaIdentityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, this->javaIdentityMap[info.fileName()]);
    }
}
void Converter::convertFile(QFileInfo fileInfo, QJsonValueRef identityRef) {
    if (identityRef.isNull() || identityRef.isUndefined()) {
        return;
    }
    QString identity = identityRef.toString();
    QString bedrockEquivelent = this->bedrockIdentityMap[identity].toString();

    if (bedrockEquivelent.startsWith("$")) {
         bedrockEquivelent = bedrockEquivelent.remove("$");

         if (bedrockEquivelent == "convert_meta") {

             QJsonObject bedrockManifest = QJsonObject();
             QJsonObject bedrockManifestHeader = QJsonObject();
             QJsonArray bedrockManifestModules = QJsonArray();
             QJsonObject bedrockManifestModule = QJsonObject();

             QJsonArray bedrockManifestResourceVersion = QJsonArray();
             QJsonArray bedrockManifestMinEngineVersion = QJsonArray();

             bedrockManifestResourceVersion.append(1);
             bedrockManifestResourceVersion.append(0);
             bedrockManifestResourceVersion.append(0);

             bedrockManifestMinEngineVersion.append(1);
             bedrockManifestMinEngineVersion.append(19);
             bedrockManifestMinEngineVersion.append(40);

             bedrockManifestModule.insert("description", this->resourcePackDesc);
             bedrockManifestModule.insert("type", "resources");
             bedrockManifestModule.insert("uuid", QString(get_uuid().c_str()));
             bedrockManifestModule.insert("version", bedrockManifestResourceVersion);

             bedrockManifestModules.append(bedrockManifestModule);

             bedrockManifestHeader.insert("description", this->resourcePackDesc);
             bedrockManifestHeader.insert("name", this->resourcePackName);
             bedrockManifestHeader.insert("uuid", QString(get_uuid().c_str()));
             bedrockManifestHeader.insert("version", bedrockManifestResourceVersion);
             bedrockManifestHeader.insert("min_engine_version", bedrockManifestMinEngineVersion);

             bedrockManifest.insert("format_version", 2);
             bedrockManifest.insert("header", bedrockManifestHeader);
             bedrockManifest.insert("modules", bedrockManifestModules);

             QFile bedrockManifestFile = QFile(this->outputResourcePackTempPath + "/manifest.json");
             bedrockManifestFile.open(QFile::WriteOnly);

             bedrockManifestFile.write(QJsonDocument(bedrockManifest).toJson());

             bedrockManifestFile.close();
         }
    } else {
        QStringList newPaths = bedrockEquivelent.split("%and%");
        QFile file = QFile(fileInfo.absoluteFilePath());

        foreach(QString newPath, newPaths) {
            QDir().mkpath(this->outputResourcePackTempPath + "/" + newPath.split('/').first(newPath.split('/').length() - 1).join('/'));
            file.copy(this->outputResourcePackTempPath + "/" + newPath);
        }
    }
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
