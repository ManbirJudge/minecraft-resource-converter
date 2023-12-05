#ifndef CONVERTER_H
#define CONVERTER_H

#include <QCoreApplication>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <functional>
#include <random>

#include <zip.h>

class Converter
{
public:
    Converter(
        QString inputJavaResPackPath,
        QString outputBedResPackDirPath,

        int inputJavaResPackType,
        int outputBedResPackType,

        int bedResPackMCMetaUUIDType,
        QString bedResPackMCMetaUUID
    );

    void startConversion();

    const cv::Vec3b grassTint {147, 223, 183};

signals:
    void conversionProgress(int currentFile, int totalFiles, QString fileName);

private:
    // utility functions
    void unzipFile(QString srcFilePath, QString unzippedDirectoryPath);
    void zipDir(QString srcDirPath, QString zippedFilePath);
    void addDirToZip(zip_t* zipArchive, zip_source_t* zipSource, QFileInfo dirInfo, QDir rootSrcDir);

    void copyDir(QString directory, QString toDirectory);

    // variables
    QString inputJavaResPackPath;
    QString outputBedResPackDirPath;

    int inputJavaResPackType;
    int outputBedResPackType;
    int bedResPackMCMetaUUIDType;
    QString bedResPackMCMetaUUID;

    QString inputJavaResPackFileName;

    QString javaResPackName;
    QString javaResPackDesc;
    QJsonObject javaResPackConfig;
    int javaResPackConfigFormat = 9;

    QString javaResPackTempPath;
    QString bedResPackTempPath;

    QJsonObject javaIdMap;
    QJsonObject bedIdMap;

    // sPeCiAl VaRiAbLe
    int totalFilesToBeConverted;

    // conversion functions
    void loadData();
    void loadIdMaps();

    void convert();
    void convertFile(QFileInfo fileInfo, QJsonValueRef identityRef);
    void convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef);

    cv::Mat applyTint(const cv::Mat& grayImg, const cv::Vec3b& color);

    // on demand speical conversion functions (ODSCFs)
    void convert_item_clock(QString inputDir, QString outputDir);
    void convert_item_compass(QString inputDir, QString outputDir);
    void convert_item_recovery_compass(QString inputDir, QString outputDir);
    void convert_item_leather_boots(QString inputDir, QString outputDir);
    void convert_item_leather_helmet(QString inputDir, QString outputDir);
    void convert_item_leather_leggings(QString inputDir, QString outputDir);
    void convert_grass(QString inputDir, QString outputDir);
    void convert_grass_block_top(QString inputDir, QString outputDir);
    void convert_tall_grass_top(QString inputDir, QString outputDir);

    QHash<QString, void(Converter::*)(const QString, const QString)> conversionFunctions;
};

#endif
