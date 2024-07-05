#include "main_window.h"
#include "src/controller.h"
#include "src/notification_delegate.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

#include <winsock2.h>
#include <windows.h>

QIcon LoadIconFromExecutable() {
    HICON hIcon = LoadIconA(GetModuleHandle(nullptr), "icon");
    if (hIcon) {
        return QIcon(QPixmap::fromImage(QImage::fromHICON(hIcon)));
    }
    return QIcon();
}

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    QApplication a(argc, argv);

    a.setWindowIcon(LoadIconFromExecutable());

    qRegisterMetaType<NotificationMessage>("NotificationMessage");
    auto delegate = NotificationDelegate::Get();
    auto ctrl = Controller::Get();
    QObject::connect(&a, &QApplication::aboutToQuit, &a, [=]() {
        ctrl->StopThread();
        WSACleanup();
    });

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SteelDiv2Rcon_" + QLocale(locale).name();
        if (translator.load(":/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
