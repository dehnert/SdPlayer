#include <QApplication>
#include "mainwindow.h"

#if defined(_WIN64)
#include <WinDef.h>
#include <Winbase.h>
#endif

int main(int argc, char *argv[])
{
#if defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    setenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins", 1);
#endif
#if defined(_WIN64)
    //SetEnvironmentVariable(TEXT("VLC_PLUGIN_PATH"), TEXT("C:/Program Files/VideoLAN/VLC"));
    SetEnvironmentVariable(TEXT("VLC_PLUGIN_PATH"), TEXT("C:/Program Files/SdPlayer"));
#endif
    QApplication a(argc, argv);
    a.setApplicationName("sdplayer");

    MainWindow w;
    w.show();
    return a.exec();
}
