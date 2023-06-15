#ifndef CONVERTER_H
#define CONVERTER_H

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
        QString inputJavaResourcePackPath,
        QString outputBedrockResourcePackDirPath,

        int inputJavaResourcePackType,
        int outputBedrockResourcePackType,

        int bedrockResourcePackMCMetaUUIDType,
        QString bedrockResourcePackMCMetaUUID
    );

    void startConversion();

signals:
    void conversionProgress(int currentFile, int totalFiles, QString fileName);

private:
    // utility functions
    void unzipFile(QString srcFilePath, QString unzippedDirectoryPath);
    void zipDir(QString srcDirPath, QString zippedFilePath);
    void addDirToZip(zip_t* zipArchive, zip_source_t* zipSource, QFileInfo dirInfo, QDir rootSrcDir);

    void copyDir(QString directory, QString toDirectory);

    // variables
    QString inputJavaResourcePackPath;
    QString outputBedrockResourcePackDirPath;

    QString inputJavaResourcePackFileName;

    QString javaResourcePackName;
    QString javaResourcePackDesc;
    QJsonObject javaResourcePackConfig;
    int javaResourcePackConfigFormat = 9;

    QString javaResourcePackTempPath;
    QString bedrockResourcePackTempPath;

    int inputJavaResourcePackType;
    int outputBedrockResourcePackType;
    int bedrockResourcePackMCMetaUUIDType;

    QString bedrockResourcePackMCMetaUUID;

    QJsonObject javaIdentityMap;
    QJsonObject bedrockIdentityMap;

    // sPeCiAl VaRiAbLe
    int totalFilesToBeConverted;

    // conversion functions
    void loadData();
    void loadIdentityPatterns();

    void convert();
    void convertFile(QFileInfo fileInfo, QJsonValueRef identityRef);
    void convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef);


    void convert_item_clock(QString inputDir, QString outputDir);
    void convert_item_compass(QString inputDir, QString outputDir);
    void convert_item_recovery_compass(QString inputDir, QString outputDir);

    QHash<QString, void (Converter::*)(QString, QString)> conversionFunctions;
};

#endif
