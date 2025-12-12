#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include <string>
#include <atomic>
#include <thread>
#include <winsock2.h>

class TelnetWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TelnetWindow(QWidget* parent = nullptr);
    ~TelnetWindow();

    void startConnection(const std::string& host, int port);
    void appendOutput(const QString& text);

signals:
    void outputReceived(const QString& text);

private slots:
    void onReturnPressed();
    void onOutputReceived(const QString& text);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QTextEdit* m_outputDisplay;
    QLineEdit* m_inputField;
    
    SOCKET m_socket;
    std::atomic<bool> m_running;
    std::thread* m_receiveThread;

    void setupUI();
    void receiveThreadFunction();
    bool initializeWinsock();
    SOCKET connectToTelnet(const std::string& host, int port);
    
    // 編碼轉換函數
    static std::string big5ToUtf8(const char* big5Str, int length);
    static std::string utf8ToBig5(const std::string& utf8Str);
    
    // ANSI 解析相關
    void appendOutputWithAnsi(const QString& text);
    QColor getAnsiColor(int colorCode, bool isBright);
    void applyAnsiCode(const QString& code);
    
    QTextCharFormat m_currentFormat;
    QColor m_defaultFgColor;
    QColor m_defaultBgColor;
};

#endif // TELNETWINDOW_H
