#include "log.h"


QHash<QString, int> Log::colors = {
    { "error", 31 },
    { "error_alt", 35 },
    { "warning", 33 },
    { "debug", 0 },
    { "verbose", 30 },
    { "info", 36 },
    { "success", 32 },
    { "success_alt", 34 },
};

void Log::log(QString msg, QString color) {
    std::cout << "\033[" << Log::colors[color] << "m"<< msg.toStdString() << "\033[m" << std::endl;
}

void Log::e(QString msg) {
    Log::log("[_ERROR_] " + msg, "error");
}
void Log::w(QString msg) {
    Log::log("[WARNING] " + msg, "warning");
}
void Log::d(QString msg) {
    Log::log("[_DEBUG_] " + msg, "debug");
}
void Log::v(QString msg) {
    Log::log("[VERBOSE] " + msg, "verbose");
}
void Log::i(QString msg) {
    Log::log("[___INFO] " + msg, "info");
}
void Log::s(QString msg) {
    Log::log("[SUCCESS] " + msg, "success");
}
