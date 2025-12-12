# Qt DLL 文件

此目錄包含運行程式所需的 Qt 5.15.2 DLL 文件。

## 目錄結構

```
qt_dlls/
├── x64/
│   ├── Debug/
│   │   ├── Qt5Cored.dll
│   │   ├── Qt5Guid.dll
│   │   ├── Qt5Widgetsd.dll
│   │   └── platforms/
│   │       └── qwindowsd.dll
│   └── Release/
│       ├── Qt5Core.dll
│       ├── Qt5Gui.dll
│       ├── Qt5Widgets.dll
│       └── platforms/
│           └── qwindows.dll
```

## 使用方式

編譯後，建置後事件會自動從此目錄複製對應的 DLL 文件到輸出目錄。

如果建置後事件失敗，可以手動複製：

### Debug x64
```cmd
xcopy /Y /D "qt_dlls\x64\Debug\*.dll" "x64\Debug\"
xcopy /Y /D "qt_dlls\x64\Debug\platforms\*.dll" "x64\Debug\platforms\"
```

### Release x64
```cmd
xcopy /Y /D "qt_dlls\x64\Release\*.dll" "x64\Release\"
xcopy /Y /D "qt_dlls\x64\Release\platforms\*.dll" "x64\Release\platforms\"
```

## 版本資訊

- Qt 版本：5.15.2
- 編譯器：MSVC 2019 64-bit
- 平台：Windows x64
