#ifndef LOG_H
#define LOG_H

#include <iostream>

#include <QString>
#include <QHash>

class Log
{
public:
    static QHash<QString, int> colors;

    static void log(QString msg, QString color);

    static void e(QString msg);
    static void w(QString msg);
    static void d(QString msg);
    static void v(QString msg);
    static void i(QString msg);
    static void s(QString msg);
};

#endif // LOG_H
