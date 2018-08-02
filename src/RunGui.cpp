#include "RunGui.hpp"

#include <QApplication>
#include <QFile>
#include <QPalette>
#include <QStyleFactory>

#include "Application.hpp"
#include "common/NetworkManager.hpp"
#include "singletons/Paths.hpp"
#include "singletons/Updates.hpp"
#include "widgets/dialogs/LastRunCrashDialog.hpp"

#ifdef C_USE_BREAKPAD
#include <QBreakpadHandler.h>
#endif

// void initQt();
// void installCustomPalette();
// void showLastCrashDialog();
// void createRunningFile(const QString &path);
// void removeRunningFile(const QString &path);

namespace chatterino {
namespace {
void installCustomPalette()
{
    // borrowed from
    // https://stackoverflow.com/questions/15035767/is-the-qt-5-dark-fusion-theme-available-for-windows
    QPalette darkPalette = qApp->palette();

    darkPalette.setColor(QPalette::Window, QColor(22, 22, 22));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor("#333"));
    darkPalette.setColor(QPalette::AlternateBase, QColor("#444"));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(70, 70, 70));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    qApp->setPalette(darkPalette);
}

void initQt()
{
    // set up the QApplication flags
    QApplication::setAttribute(Qt::AA_Use96Dpi, true);
#ifdef Q_OS_WIN32
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    installCustomPalette();
}

void showLastCrashDialog()
{
#ifndef C_DISABLE_CRASH_DIALOG
    LastRunCrashDialog dialog;

    switch (dialog.exec()) {
        case QDialog::Accepted: {
        }; break;
        default: {
            _exit(0);
        }
    }
#endif
}

void createRunningFile(const QString &path)
{
    QFile runningFile(path);

    runningFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    runningFile.flush();
    runningFile.close();
}

void removeRunningFile(const QString &path)
{
    QFile::remove(path);
}
}  // namespace

void runGui(QApplication &a, Paths &paths, Settings &settings)
{
    chatterino::NetworkManager::init();
    chatterino::Updates::getInstance().checkForUpdates();

#ifdef C_USE_BREAKPAD
    QBreakpadInstance.setDumpPath(app->paths->settingsFolderPath + "/Crashes");
#endif

    // Running file
    auto runningPath = paths.miscDirectory + "/running_" + paths.applicationFilePathHash;

    if (QFile::exists(runningPath)) {
        showLastCrashDialog();
    } else {
        createRunningFile(runningPath);
    }

    Application app(settings, paths);
    app.initialize(settings, paths);
    app.run(a);
    app.save();

    removeRunningFile(runningPath);

    pajlada::Settings::SettingManager::gSave();

    chatterino::NetworkManager::deinit();

    _exit(0);
}
}  // namespace chatterino
