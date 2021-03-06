#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#if defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    setenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins", 1);
#endif
    QApplication a(argc, argv);
    a.setApplicationName("sdplayer");

    MainWindow w;
    w.show();
    return a.exec();
}
