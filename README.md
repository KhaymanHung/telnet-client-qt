# Telnet 客戶端專案

這是一個使用 C++ 和 Winsock2 實作的 Telnet 客戶端程式，可以透過 Visual Studio 2022 開啟和編譯。

## 功能特點

- 支援連線到任何 Telnet 伺服器
- 支援自訂 IP 位址和埠號
- 即時接收和顯示伺服器回應
- 支援互動式命令輸入
- 使用多執行緒處理資料接收和傳送

## 系統需求

- Windows 作業系統
- Visual Studio 2022
- Windows SDK 10.0 或更高版本

## 編譯方式

### 使用 Visual Studio 2022

1. 開啟 `telnet.sln` 解決方案檔案
2. 選擇建置組態（Debug 或 Release）
3. 選擇平台（x86 或 x64）
4. 按 `Ctrl+Shift+B` 或選擇 **建置 → 建置方案**

### 使用命令列 (MSBuild)

```cmd
msbuild telnet.sln /p:Configuration=Release /p:Platform=x64
```

## 使用方式

### 方法 1：互動式執行

直接執行編譯後的程式：

```cmd
telnet.exe
```

程式會提示您輸入：
1. 目標 IP 或主機名稱
2. 埠號（預設為 23）

### 方法 2：命令列參數

```cmd
telnet.exe <主機名稱或IP> [埠號]
```

範例：
```cmd
# 連線到本機的 Telnet 伺服器（預設埠號 23）
telnet.exe localhost

# 連線到指定 IP 和埠號
telnet.exe 192.168.1.100 23

# 連線到遠端主機
telnet.exe towel.blinkenlights.nl
```

### 操作說明

- 連線成功後，可以直接輸入命令並按 Enter 傳送
- 伺服器的回應會即時顯示在螢幕上
- 輸入 `quit` 或 `exit` 可以離開程式

## 專案結構

```
telnet/
├── telnet.sln              # Visual Studio 解決方案檔案
├── telnet.vcxproj          # Visual Studio 專案檔案
├── telnet.vcxproj.filters  # Visual Studio 專案篩選器
├── main.cpp                # 主程式原始碼
└── README.md               # 說明文件
```

## 技術說明

### 使用的 API 和函式庫

- **Winsock2**：Windows 網路程式設計 API
- **ws2_32.lib**：Winsock 連結函式庫
- **STL**：標準模板函式庫（string, thread, atomic）

### 主要功能模組

1. **InitializeWinsock()**：初始化 Winsock 環境
2. **ConnectToTelnet()**：建立 TCP 連線到 Telnet 伺服器
3. **ReceiveThread()**：獨立執行緒處理資料接收
4. **main()**：主程式，處理使用者輸入和資料傳送

### 網路通訊細節

- 使用 TCP/IP 協定
- 支援 IPv4 和 IPv6
- 非阻塞式 Socket 操作
- Telnet 協定使用 CRLF（\r\n）作為行結束符

## 注意事項

1. 執行前請確保目標伺服器的 Telnet 服務已啟動
2. Windows 10/11 預設未啟用 Telnet 伺服器，需要手動安裝
3. 某些防火牆可能會阻擋 Telnet 連線（埠 23）
4. 本程式僅實作基本的 Telnet 客戶端功能，不支援完整的 Telnet 協定選項協商

## 疑難排解

### 編譯錯誤

- 確認已安裝 Visual Studio 2022 C++ 桌面開發工具
- 確認 Windows SDK 已正確安裝

### 連線失敗

- 檢查目標主機是否可達（使用 ping 測試）
- 確認目標埠號正確
- 檢查防火牆設定
- 確認 Telnet 服務已在目標主機上啟動

### 編碼問題

程式已設定使用 UTF-8 編碼輸出，如果遇到亂碼問題：
- 確認控制台的字型支援您需要的字元
- 可以嘗試修改 `SetConsoleOutputCP()` 的參數

## 授權

此專案僅供學習和測試用途。

## 參考資源

- [Winsock 官方文件](https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)
- [Telnet 協定 (RFC 854)](https://tools.ietf.org/html/rfc854)
