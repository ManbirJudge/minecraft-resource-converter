#ifndef CONVERTER_H
#define CONVERTER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

#include <iostream>
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

    void unzipFile(QString zipFilePath, QString unzippedDirectoryPath);
    void copyDir(QString directory, QString toDirectory);

    void startConversion();

private:
    QString inputFilePath;
    QString outputDirPath;

    QString inputFileName;

    QString resourcePackName;
    QJsonObject resourcePackConfig;
    int resourcePackConfigFormat;

    QString inputResourcePackPath;
    QString outputResourcePackPath;

    int inputResourcePackType;
    int outputResourcePackType;

    QJsonObject javaIdentityMap;
    QJsonObject bedrockIdentityMap;

    void loadData();
    void loadIdentityPatterns();
};

#endif
