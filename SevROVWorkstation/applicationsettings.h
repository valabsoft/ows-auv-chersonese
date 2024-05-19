#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QString>

class ApplicationSettings
{
private:
    const std::string APP_VERSION = "1.2.1";

public:
    ApplicationSettings();
    QString getAppVersion();
    void load();
};

#endif // APPLICATIONSETTINGS_H
