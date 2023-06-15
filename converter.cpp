#include "converter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

std::string gen_uuid() {
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
    QString inputJavaResourcePackPath_,
    QString outputBedrockResourcePackDirPath_,

    int inputJavaResourcePackType_,
    int outputBedrockResourcePackType_,
    int bedrockResourcePackMCMetaUUIDType_,

    QString bedrockResourcePackMCMetaUUID_
) : inputJavaResourcePackPath(inputJavaResourcePackPath_),
    outputBedrockResourcePackDirPath(outputBedrockResourcePackDirPath_),
    inputJavaResourcePackType(inputJavaResourcePackType_),
    outputBedrockResourcePackType(outputBedrockResourcePackType_),
    bedrockResourcePackMCMetaUUIDType(bedrockResourcePackMCMetaUUIDType_),
    bedrockResourcePackMCMetaUUID(bedrockResourcePackMCMetaUUID_)
{
    this->conversionFunctions.insert("convert_item_clock", &Converter::convert_item_clock);
    this->conversionFunctions.insert("convert_item_compass", &Converter::convert_item_compass);
    this->conversionFunctions.insert("convert_item_recovery_compass", &Converter::convert_item_recovery_compass);

    if (this->inputJavaResourcePackPath.isEmpty() or this->inputJavaResourcePackPath.isNull()) throw std::invalid_argument("Input file path was not given.");
    if (this->outputBedrockResourcePackDirPath.isEmpty() or this->outputBedrockResourcePackDirPath.isNull()) throw std::invalid_argument("Output directory was not given.");

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
void Converter::zipDir(QString srcDirPath_, QString zippedFilePath_) {
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
            this->addDirToZip(zipArchive, zipSource, fileInfo, srcDir);
        }
    }

    if (zip_close(zipArchive) == -1) {
        qDebug() << "[DEBUG] Cannot close the zip file.";
    }
}
void Converter::addDirToZip(zip_t* zipArchive, zip_source_t* zipSource, QFileInfo srcDirInfo, QDir rootSrcDir) {
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

        } else if (fileInfo.isDir())       {
            this->addDirToZip(zipArchive, zipSource, fileInfo, rootSrcDir);
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
    this->javaResourcePackTempPath = tmpDir.path() + "/Java Resource Pack";
    if (this->inputJavaResourcePackType == 0) {
        qDebug() << "[DEBUG] Input resource pack type is zip file.";

        QStringList _ = this->inputJavaResourcePackPath.split('/').last().split('.');
        _.removeLast();

        qDebug() << "[DEBUG] Unzipping input resource pack to temperary directory.";
        this->javaResourcePackName = _.join('/');
        this->unzipFile(this->inputJavaResourcePackPath, tmpDir.path() + "/Java Resource Pack");
    } else {
        qDebug() << "[DEBUG] Input resource pack type is folderI would say that.";

        qDebug() << "[DEBUG] Coppying input resource pack to temperary directory.";
        this->javaResourcePackName = this->inputJavaResourcePackPath.split('/').last();
        this->copyDir(this->inputJavaResourcePackPath, tmpDir.path() + "/Java Resource Pack");
    }

    // output resource pack stuff
    this->bedrockResourcePackTempPath = tmpDir.path() + "/Bedrock Resource Pack";
    QDir().mkdir(this->bedrockResourcePackTempPath);

    // loading resource pack configuration
    QFile packConfigFile = QFile(this->javaResourcePackTempPath + "/pack.mcmeta");
    packConfigFile.open(QFile::ReadOnly);
    QJsonDocument packConfigJsonDoc = QJsonDocument::fromJson(packConfigFile.readAll());
    packConfigFile.close();

    if (packConfigJsonDoc.isObject()) {
        this->javaResourcePackConfig = packConfigJsonDoc.object();
    }

    // TODO: this->resourcePackConfigFormat = this->resourcePackConfig["pack"].toObject()["pack_format"].toInt(0);
    this->javaResourcePackDesc = this->javaResourcePackConfig["pack"].toObject()["description"].toString();

    // loading conversion pattern
    this->loadIdentityPatterns();
}
void Converter::loadIdentityPatterns() {
    QFile javaIdentityMapFile = QFile(":/identity_maps_java/assets/identity_maps/identity_maps_java/identity_map_java_" + QString::number(this->javaResourcePackConfigFormat) + ".json");
    QFile bedrockIdentityMapFile = QFile(":/identity_maps_bedrock/assets/identity_maps/identity_maps_bedrock/identity_map_bedrock_" + QString::number(this->javaResourcePackConfigFormat) + ".json");

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

    if (this->outputBedrockResourcePackType == 0) {
        qDebug() << "[DEBUG] Zipping the output resource pack into .mcpack zip archive.";
        this->zipDir(this->bedrockResourcePackTempPath, this->outputBedrockResourcePackDirPath + "/" + this->javaResourcePackName + " (converted).mcpack");
    } else {
        qDebug() << "[DEBUG] Copying the output resource pack to output directory.";
        this->copyDir(this->bedrockResourcePackTempPath, this->outputBedrockResourcePackDirPath + "/" + this->javaResourcePackName + " (converted)");
    }

    qDebug() << "[DEBUG] Conversion done.";
}

void Converter::convert() {
    QDir javaResourcePackTempDir = QDir(this->javaResourcePackTempPath);

    foreach(const QFileInfo info, javaResourcePackTempDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, this->javaIdentityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, this->javaIdentityMap[info.fileName()]);
    }
}
void Converter::convertFile(QFileInfo fileInfo, QJsonValueRef identityRef) {
    if (identityRef.isNull() || identityRef.isUndefined()) {
        return;
    }
    QString identity = identityRef.toString();

    if (this->bedrockIdentityMap[identity].isNull() || this->bedrockIdentityMap[identity].isUndefined()) {
        return;
    }
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

            bedrockManifestModule.insert("description", this->javaResourcePackDesc);
            bedrockManifestModule.insert("type", "resources");
            bedrockManifestModule.insert("uuid", QString(gen_uuid().c_str()));
            bedrockManifestModule.insert("version", bedrockManifestResourceVersion);

            bedrockManifestModules.append(bedrockManifestModule);

            bedrockManifestHeader.insert("description", this->javaResourcePackDesc);
            bedrockManifestHeader.insert("name", this->javaResourcePackName);
            if (bedrockResourcePackMCMetaUUIDType == 0) bedrockManifestHeader.insert("uuid", QString(gen_uuid().c_str()));
            else bedrockManifestHeader.insert("uuid", QString(gen_uuid().c_str()));
            bedrockManifestHeader.insert("version", bedrockManifestResourceVersion);
            bedrockManifestHeader.insert("min_engine_version", bedrockManifestMinEngineVersion);

            bedrockManifest.insert("format_version", 2);
            bedrockManifest.insert("header", bedrockManifestHeader);
            bedrockManifest.insert("modules", bedrockManifestModules);

            QFile bedrockManifestFile = QFile(this->bedrockResourcePackTempPath + "/manifest.json");
            bedrockManifestFile.open(QFile::WriteOnly);

            bedrockManifestFile.write(QJsonDocument(bedrockManifest).toJson());

            bedrockManifestFile.close();
        }
        else {
            if (Converter::conversionFunctions.contains(bedrockEquivelent)) {
                (this->*conversionFunctions.value(bedrockEquivelent))(fileInfo.canonicalPath(), bedrockResourcePackTempPath);
            } else {
                std::cout << "[ERROR] Function not found: \"" << bedrockEquivelent.toStdString().c_str() << "\"" << std::endl;
                return;
            }
        }
    } else {
        QStringList newPaths = bedrockEquivelent.split("%and%");

        cv::Mat image = cv::imread(fileInfo.canonicalFilePath().toStdString(), cv::IMREAD_UNCHANGED);

        if (image.empty()) {
            qDebug() << "Can not open image:" << fileInfo.canonicalFilePath();
            return;
        }

        foreach(QString newPath, newPaths) {
            QDir().mkpath(this->bedrockResourcePackTempPath + "/" + newPath.split('/').first(newPath.split('/').length() - 1).join('/'));
            std::string imageFileFormat = newPath.split('.').last().toUpper().toStdString();

            if (imageFileFormat == "TGA") {
                int CHANNELS = image.channels();

                stbi_write_tga((this->bedrockResourcePackTempPath + "/" + newPath).toStdString().c_str(), image.size().width, image.size().height, CHANNELS, image.data);
            } else {
                cv::imwrite((this->bedrockResourcePackTempPath + "/" + newPath).toStdString(), image);
            }
        }
    }
}
void Converter::convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef) {
    if (identityMapRef.isNull() || identityMapRef.isUndefined()) return;
    QJsonObject identityMap = identityMapRef.toObject();

    foreach(const QFileInfo info, QDir(dirInfo.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, identityMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, identityMap[info.fileName()]);
    }
}

void Converter::convert_item_clock(QString inputDir, QString outputDir) {
    QString outputPath = outputDir + "/textures/items/clock_item.png";
    QString outputPathAtlas = outputDir + "/textures/items/watch_atlas.png";

    cv::Mat clock = cv::imread((inputDir + "/clock_00.png").toStdString(), cv::IMREAD_UNCHANGED);
    int clockWidth = clock.size().width;
    int clockHeight = clock.size().height;

    cv::imwrite(outputPath.toStdString(), clock);

    cv::Mat atlas = cv::Mat::zeros(cv::Size(clockWidth, clockHeight * 64), CV_8UC4);

    for (int i = 0; i <= 63; i++) {
        std::string clockFileName;

        if (i < 10) clockFileName = (inputDir + "/clock_0" + QString::number(i) + ".png").toStdString();
        else clockFileName = (inputDir + "/clock_" + QString::number(i) + ".png").toStdString();

        clock = cv::imread(clockFileName, cv::IMREAD_UNCHANGED);

        clock.copyTo(atlas(cv::Rect(0, clockHeight * i, clockWidth, clockHeight)));
    }


    cv::imwrite(outputPathAtlas.toStdString(), atlas);
}
void Converter::convert_item_compass(QString inputDir, QString outputDir) {
    QString outputPath = outputDir + "/textures/items/compass_item.png";
    QString outputPathAtlas = outputDir + "/textures/items/compass_atlas.png";

    cv::Mat compass = cv::imread((inputDir + "/compass_00.png").toStdString(), cv::IMREAD_UNCHANGED);
    int compassWidth = compass.size().width;
    int compassHeight = compass.size().height;

    cv::imwrite(outputPath.toStdString(), compass);

    cv::Mat atlas = cv::Mat::zeros(cv::Size(compassWidth, compassHeight * 32), CV_8UC4);

    for (int i = 0; i <= 31; i++) {
        std::string compassFileName;

        if (i < 10) compassFileName = (inputDir + "/compass_0" + QString::number(i) + ".png").toStdString();
        else compassFileName = (inputDir + "/compass_" + QString::number(i) + ".png").toStdString();

        compass = cv::imread(compassFileName, cv::IMREAD_UNCHANGED);

        compass.copyTo(atlas(cv::Rect(0, compassHeight * i, compassWidth, compassHeight)));
    }


    cv::imwrite(outputPathAtlas.toStdString(), atlas);
}
void Converter::convert_item_recovery_compass(QString inputDir, QString outputDir) {
    QString outputPath = outputDir + "/textures/items/recovery_compass_item.png";
    QString outputPathAtlas = outputDir + "/textures/items/recovery_compass_atlas.png";

    cv::Mat recoveryCompass = cv::imread((inputDir + "/recovery_compass_00.png").toStdString(), cv::IMREAD_UNCHANGED);
    int recoveryCompassWidth = recoveryCompass.size().width;
    int recoveryCompassHeight = recoveryCompass.size().height;

    cv::imwrite(outputPath.toStdString(), recoveryCompass);

    cv::Mat atlas = cv::Mat::zeros(cv::Size(recoveryCompassWidth, recoveryCompassHeight * 32), CV_8UC4);

    for (int i = 0; i <= 31; i++) {
        std::string recoveryCompassFileName;

        if (i < 10) recoveryCompassFileName = (inputDir + "/recovery_compass_0" + QString::number(i) + ".png").toStdString();
        else recoveryCompassFileName = (inputDir + "/recovery_compass_" + QString::number(i) + ".png").toStdString();

        recoveryCompass = cv::imread(recoveryCompassFileName, cv::IMREAD_UNCHANGED);

        recoveryCompass.copyTo(atlas(cv::Rect(0, recoveryCompassHeight * i, recoveryCompassWidth, recoveryCompassHeight)));
    }


    cv::imwrite(outputPathAtlas.toStdString(), atlas);
}
