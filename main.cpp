#include <QApplication>
#include "TelnetWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // 設定應用程式編碼
    QApplication::setApplicationName("Telnet Client");
    QApplication::setOrganizationName("TelnetApp");
    
    // 創建主視窗
    TelnetWindow window;
    window.show();
    
    // 自動連線到固定伺服器
    window.startConnection("us.muds.net", 4000);
    
    return app.exec();
}
