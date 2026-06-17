#include "ui/mainwindow.h"
#include "shared/systemcontext.h"
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <application/singleinstance.h>
#include <qstyle.h>
#include <shared/applicationcontext.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // saving snapshot because QApplication::setStyle loses those
    SystemContext::theme.styleName = a.style()->objectName();
    SystemContext::theme.palette = a.palette();

    QCoreApplication::setApplicationName(ApplicationContext::name);
    QCoreApplication::setApplicationVersion(ApplicationContext::version);

    // forbid running several copies
    SingleInstance instance;

    if (!instance.start("SecretsApp"))
    {
        return 0;
    }

    // managing autostart
    bool startMinimized = false;

    for (const QString &arg : QCoreApplication::arguments())
    {
        if (arg == "--minimized")
        {
            startMinimized = true;
        }
    }

    MainWindow w;

    if (startMinimized)
    {
        w.hide();
    }
    else
    {
        w.show();
    }

    // raise window of already running instance
    // if user tries to launch another one
    QObject::connect(
        &instance,
        &SingleInstance::showRequested,
        &w,
        [&w]()
        {
            w.showNormal();
            w.raise();
            w.activateWindow();
        });

    return a.exec();
}