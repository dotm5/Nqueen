#pragma once
#include <QColor>
#include <QString>
#include <QMap>

namespace NQueens {
namespace Config {

    // --- 常量 ---
    const int DEFAULT_BOARD_SIZE = 8;
    const int INITIAL_CELL_SIZE = 80;
    const int SOLUTION_PAUSE_MS = 1000;

    // --- 配色方案 ---
    namespace Colors {
        const QColor Bg("#F8FAFC");
        const QColor BoardBg("#FFFFFF");
        const QColor LightSquare("#E2E8F0");
        const QColor DarkSquare("#F1F5F9");
        const QColor QueenSafe("#10B981");
        const QColor QueenTrial("#F97316");
        const QColor QueenConflict("#EF4444");
        const QColor TextPrimary("#0F172A");
        const QColor TextSecondary("#64748B");
        const QColor Border("#CBD5E1");
        const QColor ButtonBg("#0EA5E9");
        const QColor ButtonHover("#38BDF8");
        const QColor ButtonPause("#F59E0B");
    }

    // --- 速度配置 ---
    // 注意：在头文件中定义非 const 的复杂类型最好用 extern 或 inline (C++17)
    // 这里为了简单使用 inline
    inline const QMap<QString, int> SPEED_SETTINGS = {
        {"慢速", 500},
        {"正常速度", 100},
        {"2倍速", 50},
        {"4倍速", 25},
        {"最大速度", 1}
    };

    // --- 全局样式表 ---
    const QString STYLESHEET = R"(
        QMainWindow, QWidget {
            background-color: #F8FAFC;
            color: #0F172A;
            font-family: "Segoe UI", "Helvetica Neue", "Arial", sans-serif;
        }
        QPushButton {
            background-color: #0EA5E9;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #38BDF8;
        }
        QPushButton:disabled {
            background-color: #64748B;
            color: #CBD5E1;
        }
        QPushButton#pauseButton {
            background-color: #F59E0B;
        }
        QPushButton#pauseButton:hover {
            background-color: #FBBF24;
        }
        QLabel {
            font-size: 14px;
            color: #0F172A;
        }
        QComboBox, QSpinBox {
            background-color: #FFFFFF;
            color: #0F172A;
            border: 1px solid #CBD5E1;
            padding: 6px;
            border-radius: 4px;
            min-width: 80px;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )";

} // namespace Config
} // namespace NQueens