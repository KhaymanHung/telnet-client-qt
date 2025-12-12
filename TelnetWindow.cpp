#include "TelnetWindow.h"
#include <QApplication>
#include <QWidget>
#include <QFont>
#include <QKeyEvent>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

TelnetWindow::TelnetWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_socket(INVALID_SOCKET)
    , m_running(false)
    , m_receiveThread(nullptr)
{
    setupUI();
    
    // 連接信號與槽（使用 Qt::QueuedConnection 確保執行緒安全）
    connect(this, &TelnetWindow::outputReceived, this, &TelnetWindow::onOutputReceived, Qt::QueuedConnection);
}

TelnetWindow::~TelnetWindow() {
    m_running = false;
    
    if (m_receiveThread && m_receiveThread->joinable()) {
        m_receiveThread->join();
        delete m_receiveThread;
    }
    
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
    }
    
    WSACleanup();
}

void TelnetWindow::setupUI() {
    setWindowTitle("東方故事Ⅱ - 天朝遊俠錄");
    resize(800, 600);
    
    // 初始化預設顏色和格式
    m_defaultFgColor = QColor(Qt::white);
    m_defaultBgColor = QColor(Qt::black);
    m_currentFormat.setForeground(m_defaultFgColor);
    m_currentFormat.setBackground(m_defaultBgColor);
    
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    // 輸出顯示區域
    m_outputDisplay = new QTextEdit(this);
    m_outputDisplay->setReadOnly(true);
    m_outputDisplay->setFont(QFont("Consolas", 10));
    m_outputDisplay->setStyleSheet("QTextEdit { background-color: black; color: white; }");
    layout->addWidget(m_outputDisplay);
    
    // 輸入欄位
    m_inputField = new QLineEdit(this);
    m_inputField->setFont(QFont("Consolas", 10));
    m_inputField->setPlaceholderText("輸入命令後按 Enter 送出...");
    m_inputField->installEventFilter(this);
    connect(m_inputField, &QLineEdit::returnPressed, this, &TelnetWindow::onReturnPressed);
    layout->addWidget(m_inputField);
    
    setCentralWidget(centralWidget);
}

void TelnetWindow::startConnection(const std::string& host, int port) {
    m_outputDisplay->append("正在連線到 " + QString::fromStdString(host) + ":" + QString::number(port) + "...\n");
    
    if (!initializeWinsock()) {
        m_outputDisplay->append("初始化 Winsock 失敗！\n");
        return;
    }
    
    m_socket = connectToTelnet(host, port);
    if (m_socket == INVALID_SOCKET) {
        m_outputDisplay->append("連線失敗！\n");
        return;
    }
    
    // 設定 socket 為非阻塞模式
    u_long mode = 1;
    ioctlsocket(m_socket, FIONBIO, &mode);
    
    m_outputDisplay->append("已連線。\n");
    m_outputDisplay->append("================================================================\n");
    
    // 啟動接收執行緒
    m_running = true;
    m_receiveThread = new std::thread(&TelnetWindow::receiveThreadFunction, this);
}

void TelnetWindow::appendOutput(const QString& text) {
    m_outputDisplay->moveCursor(QTextCursor::End);
    m_outputDisplay->insertPlainText(text);
    m_outputDisplay->moveCursor(QTextCursor::End);
}

void TelnetWindow::onReturnPressed() {
    QString input = m_inputField->text();
    
    if (input == "quit" || input == "exit") {
        m_running = false;
        close();
        return;
    }
    
    // 在輸出區域顯示使用者輸入的命令（如果不是空的）
    if (!input.isEmpty()) {
        m_outputDisplay->append("> " + input + "\n");
    }
    
    // 轉換為 Big5 並發送（即使是空命令也發送）
    std::string utf8Input = input.toStdString();
    std::string big5Input = utf8ToBig5(utf8Input);
    big5Input += "\r\n";
    
    int bytesSent = send(m_socket, big5Input.c_str(), (int)big5Input.length(), 0);
    
    if (bytesSent == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            m_outputDisplay->append("\n傳送錯誤: " + QString::number(error) + "\n");
            m_running = false;
        }
    }
    
    m_inputField->clear();
}

void TelnetWindow::onOutputReceived(const QString& text) {
    appendOutputWithAnsi(text);
}

void TelnetWindow::receiveThreadFunction() {
    char buffer[4096];
    int bytesReceived;
    
    while (m_running) {
        bytesReceived = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            // 將 Big5 編碼轉換為 UTF-8
            std::string utf8Text = big5ToUtf8(buffer, bytesReceived);
            emit outputReceived(QString::fromStdString(utf8Text));
        }
        else if (bytesReceived == 0) {
            emit outputReceived("\n連線已被遠端主機關閉。\n");
            m_running = false;
            break;
        }
        else {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                emit outputReceived("\n接收錯誤: " + QString::number(error) + "\n");
                m_running = false;
                break;
            }
        }
        
        // 短暫延遲避免 CPU 占用過高
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool TelnetWindow::initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return (result == 0);
}

SOCKET TelnetWindow::connectToTelnet(const std::string& host, int port) {
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo* result = nullptr;
    struct addrinfo hints;
    
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    std::string portStr = std::to_string(port);
    int iResult = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (iResult != 0) {
        return INVALID_SOCKET;
    }
    
    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }
        
        iResult = ::connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }
    
    freeaddrinfo(result);
    return sock;
}

std::string TelnetWindow::big5ToUtf8(const char* big5Str, int length) {
    if (length <= 0) return "";
    
    int wideSize = MultiByteToWideChar(950, MB_PRECOMPOSED, big5Str, length, nullptr, 0);
    if (wideSize <= 0) {
        return std::string(big5Str, length);
    }
    
    std::vector<wchar_t> wideStr(wideSize);
    MultiByteToWideChar(950, MB_PRECOMPOSED, big5Str, length, wideStr.data(), wideSize);
    
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), wideSize, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 0) {
        return std::string(big5Str, length);
    }
    
    std::vector<char> utf8Str(utf8Size);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), wideSize, utf8Str.data(), utf8Size, nullptr, nullptr);
    
    return std::string(utf8Str.data(), utf8Size);
}

std::string TelnetWindow::utf8ToBig5(const std::string& utf8Str) {
    if (utf8Str.empty()) return "";
    
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.length(), nullptr, 0);
    if (wideSize <= 0) {
        return utf8Str;
    }
    
    std::vector<wchar_t> wideStr(wideSize);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.length(), wideStr.data(), wideSize);
    
    int big5Size = WideCharToMultiByte(950, 0, wideStr.data(), wideSize, nullptr, 0, nullptr, nullptr);
    if (big5Size <= 0) {
        return utf8Str;
    }
    
    std::vector<char> big5Str(big5Size);
    WideCharToMultiByte(950, 0, wideStr.data(), wideSize, big5Str.data(), big5Size, nullptr, nullptr);
    
    return std::string(big5Str.data(), big5Size);
}

// 解析並應用 ANSI escape codes
void TelnetWindow::appendOutputWithAnsi(const QString& text) {
    QTextCursor cursor = m_outputDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QString remaining = text;
    int pos = 0;
    
    while (pos < remaining.length()) {
        // 查找 ESC 字元 (ASCII 27 或 \x1B)
        int escPos = remaining.indexOf(QChar(27), pos);
        
        if (escPos == -1) {
            // 沒有更多的 ANSI 碼，輸出剩餘文字
            cursor.insertText(remaining.mid(pos), m_currentFormat);
            break;
        }
        
        // 輸出 ANSI 碼之前的文字
        if (escPos > pos) {
            cursor.insertText(remaining.mid(pos, escPos - pos), m_currentFormat);
        }
        
        // 檢查是否為 CSI 序列 (ESC[)
        if (escPos + 1 < remaining.length() && remaining[escPos + 1] == '[') {
            // 查找 ANSI 碼的結束位置
            int endPos = escPos + 2;
            while (endPos < remaining.length()) {
                QChar ch = remaining[endPos];
                if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                    // 找到結束字元
                    QString ansiCode = remaining.mid(escPos + 2, endPos - escPos - 2);
                    applyAnsiCode(ansiCode);
                    pos = endPos + 1;
                    break;
                }
                endPos++;
            }
            if (endPos >= remaining.length()) {
                // 未找到結束字元，跳過這個 ESC
                pos = escPos + 1;
            }
        } else {
            // 不是 CSI 序列，跳過這個 ESC
            pos = escPos + 1;
        }
    }
    
    m_outputDisplay->setTextCursor(cursor);
}

// 根據 ANSI 代碼應用格式
void TelnetWindow::applyAnsiCode(const QString& code) {
    if (code.isEmpty() || code == "0") {
        // 重置所有格式
        m_currentFormat = QTextCharFormat();
        m_currentFormat.setForeground(m_defaultFgColor);
        m_currentFormat.setBackground(m_defaultBgColor);
        return;
    }
    
    // 分割多個代碼 (例如: "2;37;0")
    QStringList codes = code.split(';');
    
    for (const QString& c : codes) {
        bool ok;
        int codeNum = c.toInt(&ok);
        if (!ok) continue;
        
        if (codeNum == 0) {
            // 重置
            m_currentFormat = QTextCharFormat();
            m_currentFormat.setForeground(m_defaultFgColor);
            m_currentFormat.setBackground(m_defaultBgColor);
        }
        else if (codeNum == 1) {
            // 粗體/亮色
            m_currentFormat.setFontWeight(QFont::Bold);
        }
        else if (codeNum == 2) {
            // 暗色
            m_currentFormat.setFontWeight(QFont::Light);
        }
        else if (codeNum == 4) {
            // 底線
            m_currentFormat.setFontUnderline(true);
        }
        else if (codeNum == 7) {
            // 反轉顏色
            QColor fg = m_currentFormat.foreground().color();
            QColor bg = m_currentFormat.background().color();
            m_currentFormat.setForeground(bg);
            m_currentFormat.setBackground(fg);
        }
        else if (codeNum >= 30 && codeNum <= 37) {
            // 前景色
            m_currentFormat.setForeground(getAnsiColor(codeNum - 30, false));
        }
        else if (codeNum >= 40 && codeNum <= 47) {
            // 背景色
            m_currentFormat.setBackground(getAnsiColor(codeNum - 40, false));
        }
        else if (codeNum >= 90 && codeNum <= 97) {
            // 亮前景色
            m_currentFormat.setForeground(getAnsiColor(codeNum - 90, true));
        }
        else if (codeNum >= 100 && codeNum <= 107) {
            // 亮背景色
            m_currentFormat.setBackground(getAnsiColor(codeNum - 100, true));
        }
    }
}

// 取得 ANSI 顏色
QColor TelnetWindow::getAnsiColor(int colorCode, bool isBright) {
    static const QColor darkColors[] = {
        QColor(0, 0, 0),       // 0: Black
        QColor(170, 0, 0),     // 1: Red
        QColor(0, 170, 0),     // 2: Green
        QColor(170, 85, 0),    // 3: Yellow/Brown
        QColor(0, 0, 170),     // 4: Blue
        QColor(170, 0, 170),   // 5: Magenta
        QColor(0, 170, 170),   // 6: Cyan
        QColor(170, 170, 170)  // 7: White/Gray
    };
    
    static const QColor brightColors[] = {
        QColor(85, 85, 85),    // 0: Bright Black (Gray)
        QColor(255, 85, 85),   // 1: Bright Red
        QColor(85, 255, 85),   // 2: Bright Green
        QColor(255, 255, 85),  // 3: Bright Yellow
        QColor(85, 85, 255),   // 4: Bright Blue
        QColor(255, 85, 255),  // 5: Bright Magenta
        QColor(85, 255, 255),  // 6: Bright Cyan
        QColor(255, 255, 255)  // 7: Bright White
    };
    
    if (colorCode < 0 || colorCode > 7) {
        return isBright ? brightColors[7] : darkColors[7];
    }
    
    return isBright ? brightColors[colorCode] : darkColors[colorCode];
}

// 事件過濾器，處理特殊按鍵
bool TelnetWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_inputField && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        
        // Ctrl+Enter 可以插入換行符號到輸入欄位（雖然 QLineEdit 不支援多行，但這裡保留邏輯）
        // 一般 Enter 就直接發送命令
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                // Ctrl+Enter: 不做處理，讓預設行為發生
                return false;
            }
            // 普通 Enter: 觸發發送（已經由 returnPressed 信號處理）
            return false;
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}
