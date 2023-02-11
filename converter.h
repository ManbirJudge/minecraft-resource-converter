#ifndef CONVERTER_H
#define CONVERTER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

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
        QString inputFilePath,
        QString outputDirPath,

        int inputResourcePackType,
        int outputResourcePackType
    );

    void unzipFile(QString srcFilePath, QString unzippedDirectoryPath);
    void zipDirectory(QString srcDirPath, QString zippedFilePath);
    void addDirectoryToZip(zip_t* zipArchive, zip_source_t* zipSource, QFileInfo dirInfo, QDir rootSrcDir);

    void copyDir(QString directory, QString toDirectory);

    void startConversion();

signals:
    void conversionProgress(int currentFile, int totalFiles, QString fileName);

private:
    QString inputFilePath;
    QString outputDirPath;

    QString inputFileName;

    QString resourcePackName;
    QString resourcePackDesc;
    QJsonObject resourcePackConfig;
    int resourcePackConfigFormat = 9;

    QString inputResourcePackTempPath;
    QString outputResourcePackTempPath;

    int inputResourcePackType;
    int outputResourcePackType;

    QJsonObject javaIdentityMap;
    QJsonObject bedrockIdentityMap;

    // ------------------
    int totalFilesToBeConverted;
    // ------------------

    void loadData();
    void loadIdentityPatterns();

    void convert();
    void convertFile(QFileInfo fileInfo, QJsonValueRef identityRef);
    void convertDir(QFileInfo dirInfo, QJsonValueRef identityMapRef);
};

#endif
