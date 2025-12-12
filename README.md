# Telnet 客戶端 (Qt GUI 版本)

這是一個使用 C++、Qt 5.15.2 和 Winsock2 實作的 Telnet 客戶端程式，具有圖形化介面，可以透過 Visual Studio 2022 開啟和編譯。

## 功能特點

- ✅ Qt 5.15.2 圖形化使用者介面
- ✅ 支援 Big5 ↔ UTF-8 編碼自動轉換
- ✅ 完整的 ANSI escape codes 顏色支援
- ✅ 固定連線到 us.muds.net:4000（東方故事Ⅱ）
- ✅ 即時接收和顯示伺服器回應
- ✅ 支援互動式命令輸入
- ✅ 多執行緒處理資料接收和傳送
- ✅ 黑色背景終端機風格介面

## 系統需求

- Windows 作業系統
- Visual Studio 2022
- Qt 5.15.2（安裝於 D:\Qt\5.15.2）
- Windows SDK 10.0 或更高版本

## 編譯方式

### 使用 Visual Studio 2022

1. 開啟 `telnet.sln` 解決方案檔案
2. 選擇建置組態（Debug 或 Release）
3. 選擇平台（**推薦使用 x64**）
4. 按 `F7` 或選擇 **建置 → 建置方案**
5. 建置完成後會自動複製所需的 Qt DLL 和平台插件

### 使用命令列 (MSBuild)

```cmd
msbuild telnet.sln /p:Configuration=Release /p:Platform=x64
```

## 使用方式

執行編譯後的程式：

```cmd
telnet.exe
```

程式會自動連線到 **us.muds.net:4000**（東方故事Ⅱ - 天朝遊俠錄）

### 操作說明

- 程式啟動後自動連線
- 在下方輸入欄位輸入命令
- 按 **Enter** 發送命令（即使是空命令也會發送）
- 伺服器的回應會即時顯示在上方文字區域
- 支援 ANSI 顏色碼，會正確顯示彩色文字
- 輸入 `quit` 或 `exit` 可以離開程式

## 專案結構

```
telnet/
├── telnet.sln              # Visual Studio 解決方案檔案
├── telnet.vcxproj          # Visual Studio 專案檔案
├── telnet.vcxproj.filters  # Visual Studio 專案篩選器
├── main.cpp                # 主程式原始碼
├── TelnetWindow.h          # Qt 視窗類別標頭檔
├── TelnetWindow.cpp        # Qt 視窗類別實作
├── moc_TelnetWindow.cpp    # Qt MOC 自動生成檔案
├── .gitignore              # Git 忽略檔案設定
└── README.md               # 說明文件
```

## 技術說明

### 使用的技術和函式庫

- **Qt 5.15.2**：跨平台 GUI 框架
  - QMainWindow：主視窗
  - QTextEdit：文字輸出顯示（支援富文本格式）
  - QLineEdit：命令輸入欄位
  - Qt Signals & Slots：執行緒間通訊
- **Winsock2**：Windows 網路程式設計 API
- **ws2_32.lib**：Winsock 連結函式庫
- **STL**：標準模板函式庫（string, thread, atomic, vector）

### 主要功能模組

1. **TelnetWindow 類別**：主視窗和 UI 管理
2. **connectToTelnet()**：建立 TCP 連線到 Telnet 伺服器
3. **receiveThreadFunction()**：獨立執行緒處理資料接收
4. **big5ToUtf8() / utf8ToBig5()**：編碼轉換
5. **appendOutputWithAnsi()**：解析 ANSI escape codes
6. **applyAnsiCode()**：應用顏色和格式

### ANSI 支援功能

程式完整支援以下 ANSI escape codes：
- `[0m` - 重置所有格式
- `[1m` - 粗體/亮色
- `[2m` - 暗色
- `[4m` - 底線
- `[7m` - 反轉前景和背景色
- `[30m-[37m` - 標準前景色
- `[40m-[47m` - 標準背景色
- `[90m-[97m` - 亮前景色
- `[100m-[107m` - 亮背景色

### 編碼轉換

- **接收**：伺服器發送的 Big5 編碼自動轉換為 UTF-8 顯示
- **發送**：使用者輸入的 UTF-8 文字自動轉換為 Big5 發送

### 網路通訊細節

- 使用 TCP/IP 協定
- 支援 IPv4 和 IPv6
- 非阻塞式 Socket 操作
- Telnet 協定使用 CRLF（\r\n）作為行結束符

## Qt 部署

程式已配置自動建置後事件，會自動複製以下檔案到輸出目錄：

### Debug 模式
- Qt5Cored.dll
- Qt5Guid.dll
- Qt5Widgetsd.dll
- platforms/qwindowsd.dll

### Release 模式
- Qt5Core.dll
- Qt5Gui.dll
- Qt5Widgets.dll
- platforms/qwindows.dll

**注意**：確保 Qt 安裝路徑為 `D:\Qt\5.15.2`，否則需要修改專案檔案中的路徑。

## 疑難排解

### 編譯錯誤

- 確認已安裝 Visual Studio 2022 C++ 桌面開發工具
- 確認 Windows SDK 已正確安裝
- 確認 Qt 5.15.2 已安裝於 `D:\Qt\5.15.2`
- 如果 Qt 安裝在其他路徑，需修改 `telnet.vcxproj` 中的路徑

### 執行時錯誤：找不到平台插件

如果出現 "could not initialize platform plugin" 錯誤：
- 確認 `platforms` 資料夾存在於執行檔同目錄
- 確認 `qwindows.dll` 或 `qwindowsd.dll` 在 `platforms` 資料夾中
- 重新建置專案以觸發建置後事件

### 連線失敗

- 檢查網路連線
- 確認 us.muds.net 可以連線（使用 ping 測試）
- 檢查防火牆設定

### 中文顯示亂碼

- 程式已內建 Big5 ↔ UTF-8 轉換，應該不會有亂碼問題
- 如果仍有問題，檢查 Qt 是否正確編譯為 UTF-8 模式

## 授權

此專案僅供學習和測試用途。

## 參考資源

- [Qt 5.15 官方文件](https://doc.qt.io/qt-5.15/)
- [Winsock 官方文件](https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)
- [Telnet 協定 (RFC 854)](https://tools.ietf.org/html/rfc854)
- [ANSI Escape Codes](https://en.wikipedia.org/wiki/ANSI_escape_code)
