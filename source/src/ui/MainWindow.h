#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QTimer>

#include "core/NQueensSolver.h"
#include "ui/ChessboardWidget.h"

namespace NQueens {
    namespace UI {

        class MainWindow : public QMainWindow {
            Q_OBJECT

        public:
            MainWindow();
            ~MainWindow();

        private slots:
            void changeBoardSize(int newSize);
            void updateSpeed(const QString &speedText);
            void toggleSearch();
            void togglePause();
            void nextStep();

        private:
            void setupUI();
            void startSearch();
            void resetSearch();
            void resetUIState(bool finished);

            // 截图辅助函数
            void handleSnapshot(const SolverState& state);
            void saveSnapshot(int solutionIndex, bool isMirror, QVector<int> queens);

            int boardSize;
            Core::NQueensSolver *solver;
            ChessboardWidget *chessboard;
            QTimer *timer;
            bool isPaused;

            QSpinBox *sizeSpin;
            QComboBox *speedCombo;
            QPushButton *startButton;
            QPushButton *pauseButton;
            QLabel *statusLabel;
            QLabel *statsLabel;
        };

    } // namespace UI
} // namespace NQueens