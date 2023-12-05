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
    QString inputJavaResPackPath_,
    QString outputBedResPackDirPath_,

    int inputJavaResPackType_,
    int outputBedResPackType_,
    int bedResPackMCMetaUUIDType_,

    QString bedResPackMCMetaUUID_
) : inputJavaResPackPath(inputJavaResPackPath_),
    outputBedResPackDirPath(outputBedResPackDirPath_),
    inputJavaResPackType(inputJavaResPackType_),
    outputBedResPackType(outputBedResPackType_),
    bedResPackMCMetaUUIDType(bedResPackMCMetaUUIDType_),
    bedResPackMCMetaUUID(bedResPackMCMetaUUID_)
{
    conversionFunctions.insert("convert_item_clock", &Converter::convert_item_clock);
    conversionFunctions.insert("convert_item_compass", &Converter::convert_item_compass);
    conversionFunctions.insert("convert_item_recovery_compass", &Converter::convert_item_recovery_compass);
    conversionFunctions.insert("convert_item_leather_boots", &Converter::convert_item_leather_boots);
    conversionFunctions.insert("convert_item_leather_helmet", &Converter::convert_item_leather_helmet);
    conversionFunctions.insert("convert_item_leather_leggings", &Converter::convert_item_leather_leggings);
    conversionFunctions.insert("convert_grass", &Converter::convert_grass);
    conversionFunctions.insert("convert_grass_block_top", &Converter::convert_grass_block_top);
    conversionFunctions.insert("convert_tall_grass_top", &Converter::convert_tall_grass_top);

    if (inputJavaResPackPath.isEmpty() or inputJavaResPackPath.isNull()) throw std::invalid_argument("Input file path was not given.");
    if (outputBedResPackDirPath.isEmpty() or outputBedResPackDirPath.isNull()) throw std::invalid_argument("Output directory was not given.");

    loadData();
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
            addDirToZip(zipArchive, zipSource, fileInfo, srcDir);
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
            addDirToZip(zipArchive, zipSource, fileInfo, rootSrcDir);
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
    javaResPackTempPath = tmpDir.path() + "/java-resource-pack";

    if (inputJavaResPackType == 0) {
        qDebug() << "[DEBUG] Input resource pack type is zip file.";

        QStringList _ = inputJavaResPackPath.split('/').last().split('.');
        _.removeLast();
        javaResPackName = _.join('.');
        javaResPackName = gen_uuid().c_str();

        qDebug() << "[DEBUG] Unzipping input java resource pack into a temperary directory.";
        unzipFile(inputJavaResPackPath, javaResPackTempPath);
    } else {
        qDebug() << "[DEBUG] Input resource pack type is folder.";

        javaResPackName = inputJavaResPackPath.split('/').last();

        qDebug() << "[DEBUG] Coppying input resource pack to temperary directory.";
        copyDir(inputJavaResPackPath, javaResPackTempPath);
    }

    // temperary output directory
    bedResPackTempPath = tmpDir.path() + "/bedreock-resource-pack";
    QDir().mkdir(bedResPackTempPath);

    // resource pack configuration
    QFile packConfigFile = QFile(javaResPackTempPath + "/pack.mcmeta");
    packConfigFile.open(QFile::ReadOnly);
    QJsonDocument packConfigJsonDoc = QJsonDocument::fromJson(packConfigFile.readAll());
    packConfigFile.close();

    if (packConfigJsonDoc.isObject()) {
        javaResPackConfig = packConfigJsonDoc.object();
    }

    javaResPackConfigFormat = javaResPackConfig["pack"].toObject()["pack_format"].toInt(0);
    javaResPackDesc = javaResPackConfig["pack"].toObject()["description"].toString();

    // id maps
    loadIdMaps();
}
void Converter::loadIdMaps() {
    QFile javaIdMapFile = QFile(QCoreApplication::applicationDirPath() + "/assets/id-maps/java/" + QString::number(javaResPackConfigFormat) + ".json");
    QFile bedIdMapFile = QFile(QCoreApplication::applicationDirPath() + "/assets/id-maps/bedrock/latest.json");

    javaIdMapFile.open(QIODevice::ReadOnly | QIODevice::Text);
    bedIdMapFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonDocument javaIdentityMapJsonDoc = QJsonDocument::fromJson(javaIdMapFile.readAll());
    QJsonDocument bedIdentityMapJsonDoc = QJsonDocument::fromJson(bedIdMapFile.readAll());

    javaIdMapFile.close();
    bedIdMapFile.close();

    if (javaIdentityMapJsonDoc.isObject()) {
        javaIdMap = javaIdentityMapJsonDoc.object();
    }
    if (bedIdentityMapJsonDoc.isObject()) {
        bedIdMap = bedIdentityMapJsonDoc.object();
    }
}

void Converter::startConversion() {
    qDebug() << "[DEBUG] Starting conversion.";

    convert();

    if (outputBedResPackType == 0) {
        qDebug() << "[DEBUG] Zipping the output resource pack into .mcpack zip archive.";
        zipDir(bedResPackTempPath, outputBedResPackDirPath + "/" + javaResPackName + " (converted).mcpack");
    } else {
        qDebug() << "[DEBUG] Copying the output resource pack to output directory.";
        copyDir(bedResPackTempPath, outputBedResPackDirPath + "/" + javaResPackName + " (converted)");
    }

    qDebug() << "[DEBUG] Conversion done.";
}

void Converter::convert() {
    QDir javaResPackTempDir = QDir(javaResPackTempPath);

    foreach(const QFileInfo info, javaResPackTempDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, javaIdMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, javaIdMap[info.fileName()]);
    }
}
void Converter::convertFile(QFileInfo fileInfo, QJsonValueRef idRef) {
    if (idRef.isNull() || idRef.isUndefined()) {
        return;
    }
    QString id = idRef.toString();

    if (bedIdMap[id].isNull() || bedIdMap[id].isUndefined()) {
        return;
    }
    QString bedEquivelent = bedIdMap[id].toString();

    if (bedEquivelent.startsWith("$")) {
        bedEquivelent = bedEquivelent.remove("$");

        if (bedEquivelent == "convert_meta") {
            QJsonObject bedManifest = QJsonObject();
            QJsonObject bedManifestHeader = QJsonObject();
            QJsonArray bedManifestModules = QJsonArray();
            QJsonObject bedManifestModule = QJsonObject();

            QJsonArray bedManifestResVersion = QJsonArray();
            QJsonArray bedManifestMinEngineVersion = QJsonArray();

            bedManifestResVersion.append(1);
            bedManifestResVersion.append(0);
            bedManifestResVersion.append(0);

            bedManifestMinEngineVersion.append(1);
            bedManifestMinEngineVersion.append(19);
            bedManifestMinEngineVersion.append(40);

            bedManifestModule.insert("description", javaResPackDesc);
            bedManifestModule.insert("type", "resources");
            bedManifestModule.insert("uuid", QString(gen_uuid().c_str()));
            bedManifestModule.insert("version", bedManifestResVersion);

            bedManifestModules.append(bedManifestModule);

            bedManifestHeader.insert("description", javaResPackDesc);
            bedManifestHeader.insert("name", javaResPackName);
            if (bedResPackMCMetaUUIDType == 0) bedManifestHeader.insert("uuid", QString(gen_uuid().c_str()));
            else bedManifestHeader.insert("uuid", QString(gen_uuid().c_str()));
            bedManifestHeader.insert("version", bedManifestResVersion);
            bedManifestHeader.insert("min_engine_version", bedManifestMinEngineVersion);

            bedManifest.insert("format_version", 2);
            bedManifest.insert("header", bedManifestHeader);
            bedManifest.insert("modules", bedManifestModules);

            QFile bedManifestFile = QFile(bedResPackTempPath + "/manifest.json");
            bedManifestFile.open(QFile::WriteOnly);

            bedManifestFile.write(QJsonDocument(bedManifest).toJson());

            bedManifestFile.close();
        }
        else {
            if (Converter::conversionFunctions.contains(bedEquivelent)) {
                (this->*conversionFunctions.value(bedEquivelent))(fileInfo.canonicalPath(), bedResPackTempPath);
            } else {
                std::cout << "[ERROR] Function not found: \"" << bedEquivelent.toStdString().c_str() << "\"" << std::endl;
                return;
            }
        }
    } else {
        QStringList newPaths = bedEquivelent.split("%and%");

        cv::Mat resImg = cv::imread(fileInfo.canonicalFilePath().toStdString(), cv::IMREAD_UNCHANGED);
        if (resImg.empty()) {
            qDebug() << "Can not open image:" << fileInfo.canonicalFilePath();
            return;
        }

        foreach(QString newPath, newPaths) {
            QDir().mkpath(bedResPackTempPath + "/" + newPath.split('/').first(newPath.split('/').length() - 1).join('/'));
            std::string resImgFormat = newPath.split('.').last().toUpper().toStdString();

            if (resImgFormat == "TGA") {
                int CHANNELS = resImg.channels();
                stbi_write_tga((bedResPackTempPath + "/" + newPath).toStdString().c_str(), resImg.size().width, resImg.size().height, CHANNELS, resImg.data);
            } else {
                cv::imwrite((bedResPackTempPath + "/" + newPath).toStdString(), resImg);
            }
        }
    }
}
void Converter::convertDir(QFileInfo dirInfo, QJsonValueRef idMapRef) {
    if (idMapRef.isNull() || idMapRef.isUndefined()) return;
    QJsonObject idMap = idMapRef.toObject();

    foreach(const QFileInfo info, QDir(dirInfo.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isFile()) convertFile(info, idMap[info.fileName()]);
        else if (info.isDir()) convertDir(info, idMap[info.fileName()]);
    }
}

cv::Mat Converter::applyTint(const cv::Mat& bgrImg, const cv::Vec3b& color) {
    qDebug() << bgrImg.channels();

    // gray to BGR
    cv::Mat grayImg;
    cv::cvtColor(bgrImg, grayImg, cv::COLOR_BGR2GRAY);

    // normalize
    // cv::Mat normalized;
    // grayImg.convertTo(normalized, CV_32F, 1.0 / 255.0);

    // scale the color
    cv::Mat tinted(grayImg.rows, grayImg.cols, bgrImg.type());

    if (bgrImg.channels() == 3) {
        for (int i = 0; i < grayImg.rows; i++) {
            for (int j = 0; j < grayImg.cols; j++) {
                double normPixVal =  (int) grayImg.at<uchar>(i, j) / 255.0;
                tinted.at<cv::Vec3b>(i, j) = cv::Vec3b(normPixVal * color[0], normPixVal * color[1], normPixVal * color[2]);
            }
        }
    } else if (bgrImg.channels() == 4) {
        for (int i = 0; i < grayImg.rows; i++) {
            for (int j = 0; j < grayImg.cols; j++) {
                double normPixVal =  (int) grayImg.at<uchar>(i, j) / 255.0;
                tinted.at<cv::Vec4b>(i, j) = cv::Vec4b(
                    normPixVal * color[0],
                    normPixVal * color[1],
                    normPixVal * color[2],
                    bgrImg.at<cv::Vec4b>(i, j)[3]
                );
            }
        }
    }

    // Blend the colored image with the original grayscale image
    cv::Mat resultImg;
    cv::addWeighted(bgrImg, 0.0, tinted, 1.0, 0.0, resultImg);

    return tinted;
}
void Converter::BGRA2GRAYAlpha(const cv::Mat& bgraImg, cv::Mat& outputImg) {
    // split BGRA image channels
    std::vector<cv::Mat> bgraChannels;
    cv::split(bgraImg, bgraChannels);

    // extract the alpha channel
    cv::Mat alphaChannel = bgraChannels.back();

    // convert the BGR channels to grayscale
    cv::Mat grayImg;
    cv::cvtColor(bgraImg, grayImg, cv::COLOR_BGRA2GRAY);

    // create a 4-channel output image
    cv::Mat grayAlphaImg;
    cv::cvtColor(grayImg, grayAlphaImg, cv::COLOR_GRAY2BGRA);

    std::vector<cv::Mat> grayAlphaChannels;
    cv::split(grayAlphaImg, grayAlphaChannels);

    // copy the alpha channel to the output
    cv::Mat outputChannels[4] = {grayAlphaChannels[0], grayAlphaChannels[1], grayAlphaChannels[2], alphaChannel};
    cv::merge(outputChannels, 4, outputImg);
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
void Converter::convert_item_leather_boots(QString inputDir, QString outputDir) {
    outputDir = outputDir + "/textures/items";

    cv::Mat itemImg = cv::imread((inputDir + "/leather_boots.png").toStdString(), cv::IMREAD_UNCHANGED);
    cv::Mat itemOverlayImg = cv::imread((inputDir + "/leather_boots_overlay.png").toStdString(), cv::IMREAD_UNCHANGED);

    cv::Mat itemOverlayImgGray;
    BGRA2GRAYAlpha(itemOverlayImg, itemOverlayImgGray);

    cv::Mat result;
    cv::addWeighted(itemImg, 1, itemOverlayImgGray, 1, 0, result);

    stbi_write_tga((outputDir + "/leather_boots.tga").toStdString().c_str(), result.size().width, result.size().height, result.channels(), result.data);
}
void Converter::convert_item_leather_helmet(QString inputDir, QString outputDir){
    outputDir = outputDir + "/textures/items";

    cv::Mat itemImg = cv::imread((inputDir + "/leather_helmet.png").toStdString(), cv::IMREAD_UNCHANGED);
    cv::Mat itemOverlayImg = cv::imread((inputDir + "/leather_helmet_overlay.png").toStdString(), cv::IMREAD_UNCHANGED);

    cv::Mat itemOverlayImgGray;
    BGRA2GRAYAlpha(itemOverlayImg, itemOverlayImgGray);

    cv::Mat result;
    cv::addWeighted(itemImg, 1, itemOverlayImgGray, 1, 0, result);

    stbi_write_tga((outputDir + "/leather_helmet.tga").toStdString().c_str(), result.size().width, result.size().height, result.channels(), result.data);
}
void Converter::convert_item_leather_leggings(QString inputDir, QString outputDir){
    outputDir = outputDir + "/textures/items";

    cv::Mat itemImg = cv::imread((inputDir + "/leather_leggings.png").toStdString(), cv::IMREAD_UNCHANGED);
    cv::Mat itemOverlayImg = cv::imread((inputDir + "/leather_leggings_overlay.png").toStdString(), cv::IMREAD_UNCHANGED);

    cv::Mat itemOverlayImgGray;
    BGRA2GRAYAlpha(itemOverlayImg, itemOverlayImgGray);

    cv::Mat result;
    cv::addWeighted(itemImg, 1, itemOverlayImgGray, 1, 0, result);

    stbi_write_tga((outputDir + "/leather_leggings.tga").toStdString().c_str(), result.size().width, result.size().height, result.channels(), result.data);
}
void Converter::convert_grass(QString inputDir, QString outputDir){
    outputDir = outputDir + "/textures/blocks";
    cv::Mat grassImg = cv::imread((inputDir + "/grass.png").toStdString(), cv::IMREAD_UNCHANGED);

    cv::Mat grassImgCarried = applyTint(grassImg, grassTint);

    cv::imwrite((outputDir + "/tallgrass.png").toStdString(), grassImg);
    stbi_write_tga((outputDir + "/tallgrass.tga").toStdString().c_str(), grassImg.size().width, grassImg.size().height, grassImg.channels(), grassImg.data);
    cv::imwrite((outputDir + "/tallgrass_carried.png").toStdString(), grassImgCarried);
}
void Converter::convert_grass_block_top(QString inputDir, QString outputDir){
    outputDir = outputDir + "/textures/blocks";
    cv::Mat img = cv::imread((inputDir + "/grass_block_top.png").toStdString(),  cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);

    cv::Mat carried = applyTint(img, grassTint);

    cv::imwrite((outputDir + "/grass_top.png").toStdString(), img);
    cv::imwrite((outputDir + "/grass_carried.png").toStdString(), carried);
}
void Converter::convert_tall_grass_top(QString inputDir, QString outputDir){
    outputDir = outputDir + "/textures/blocks";
    cv::Mat img = cv::imread((inputDir + "/tall_grass_top.png").toStdString(), cv::IMREAD_UNCHANGED);

    cv::Mat carried = applyTint(img, grassTint);

    cv::imwrite((outputDir + "/double_plant_grass_carried.png").toStdString(), carried);
    stbi_write_tga((outputDir + "/double_plant_grass_top.tga").toStdString().c_str(), img.size().width, img.size().height, img.channels(), img.data);
}
