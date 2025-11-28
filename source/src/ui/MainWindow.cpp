#include "MainWindow.h"
#include "common/Config.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QCoreApplication>

namespace NQueens {
namespace UI {

using namespace NQueens::Config;

MainWindow::MainWindow() : solver(nullptr), isPaused(false) {
    setWindowTitle("N-Queens Visualizer (Symmetry Pruning)");
    setMinimumSize(800, 800);

    setupUI();
    setStyleSheet(STYLESHEET);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::nextStep);

    updateSpeed("正常速度");
}

MainWindow::~MainWindow() {
    if (solver) delete solver;
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QGroupBox *controlGroup = new QGroupBox("控制设置 (对称性剪枝版)");
    QGridLayout *controlLayout = new QGridLayout(controlGroup);

    controlLayout->addWidget(new QLabel("棋盘大小:"), 0, 0);
    sizeSpin = new QSpinBox();
    sizeSpin->setRange(4, 14);
    sizeSpin->setValue(DEFAULT_BOARD_SIZE);
    connect(sizeSpin, &QSpinBox::valueChanged, this, &MainWindow::changeBoardSize);
    controlLayout->addWidget(sizeSpin, 0, 1);

    controlLayout->addWidget(new QLabel("速度:"), 0, 2);
    speedCombo = new QComboBox();
    for(auto key : SPEED_SETTINGS.keys()) speedCombo->addItem(key);
    speedCombo->setCurrentText("正常速度");
    connect(speedCombo, &QComboBox::currentTextChanged, this, &MainWindow::updateSpeed);
    controlLayout->addWidget(speedCombo, 0, 3);

    startButton = new QPushButton("开始演示");
    pauseButton = new QPushButton("暂停");
    pauseButton->setObjectName("pauseButton");
    pauseButton->setEnabled(false);

    connect(startButton, &QPushButton::clicked, this, &MainWindow::toggleSearch);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::togglePause);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(startButton);
    btnLayout->addWidget(pauseButton);
    controlLayout->addLayout(btnLayout, 0, 4, 1, 2);

    statusLabel = new QLabel("点击 '开始演示'。算法将利用对称性只搜索一半棋盘。");
    statsLabel = new QLabel("");
    controlLayout->addWidget(statusLabel, 1, 0, 1, 3);
    controlLayout->addWidget(statsLabel, 1, 3, 1, 3);

    mainLayout->addWidget(controlGroup);

    boardSize = DEFAULT_BOARD_SIZE;
    chessboard = new ChessboardWidget(boardSize);
    mainLayout->addWidget(chessboard, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::changeBoardSize(int newSize) {
    if (!timer->isActive()) {
        boardSize = newSize;
        chessboard->setBoardSize(newSize);
        statusLabel->setText(QString("棋盘大小已改为 %1×%1").arg(newSize));
    }
}

void MainWindow::updateSpeed(const QString &speedText) {
    int interval = SPEED_SETTINGS.value(speedText, 100);
    timer->setInterval(interval);
    chessboard->setAnimationSpeed(interval);
}

void MainWindow::toggleSearch() {
    if (timer->isActive() || isPaused) {
        timer->stop();
        resetSearch();
    } else {
        startSearch();
    }
}

void MainWindow::togglePause() {
    isPaused = !isPaused;
    if (isPaused) {
        timer->stop();
        pauseButton->setText("继续");
        statusLabel->setText("已暂停");
    } else {
        timer->start();
        pauseButton->setText("暂停");
        statusLabel->setText("正在搜索... (对称性剪枝开启)");
    }
}

void MainWindow::startSearch() {
    if (solver) delete solver;
    solver = new Core::NQueensSolver(boardSize);

    startButton->setText("停止");
    pauseButton->setEnabled(true);
    sizeSpin->setEnabled(false);
    statusLabel->setText("正在搜索... (对称优化中)");
    statsLabel->setText("步数: 0");

    timer->start();
}

void MainWindow::resetSearch() {
    if (solver) { delete solver; solver = nullptr; }
    resetUIState(false);
}

void MainWindow::resetUIState(bool finished) {
    startButton->setText(finished ? "重新开始" : "开始演示");
    pauseButton->setEnabled(false);
    pauseButton->setText("暂停");
    isPaused = false;
    sizeSpin->setEnabled(true);
    if (!finished) {
        SolverState emptyState;
        emptyState.queens.fill(-1, boardSize);
        emptyState.trialPos = {-1, -1};
        emptyState.hasConflict = false;
        chessboard->setState(emptyState);
        statusLabel->setText("点击 '开始演示' 启动。");
        statsLabel->setText("");
    }
}

void MainWindow::nextStep() {
    if (!solver) return;

    SolverState state = solver->nextStep();
    chessboard->setState(state);

    if (state.isFinished) {
        timer->stop();
        statusLabel->setText(QString("完成! 找到 %1 个解 (利用对称性减少了约50%计算)").arg(state.solutionsCount));
        statsLabel->setText(QString("计算步数: %1").arg(state.stepsCount));
        resetUIState(true);
        return;
    }

    if (state.solutionFound) {
        handleSnapshot(state);
        statsLabel->setText(QString("步数: %1").arg(state.stepsCount));
    } else {
        statusLabel->setText(QString("正在搜索... 已找到 %1 个解").arg(state.solutionsCount));
        statsLabel->setText(QString("步数: %1").arg(state.stepsCount));
    }
}

void MainWindow::handleSnapshot(const SolverState& state) {
    timer->stop(); // 暂停以保存

    int currentId = state.solutionsCount - state.newSolutionsFound + 1;
    saveSnapshot(currentId, false, state.queens); // 保存基础解

    QString msg = QString("找到解 #%1").arg(currentId);

    if (state.isSymmetricBase) {
        int mirrorId = currentId + 1;
        saveSnapshot(mirrorId, true, state.queens); // 保存镜像解
        msg += QString(" 及镜像解 #%1 (自动推导)").arg(mirrorId);
    }

    statusLabel->setText(msg + "，图片已保存。");

    // 延时后自动恢复搜索
    QTimer::singleShot(SOLUTION_PAUSE_MS, this, [this](){
        if (!isPaused && startButton->text() == "停止") {
             timer->start();
        }
    });
}

void MainWindow::saveSnapshot(int solutionIndex, bool isMirror, QVector<int> queens) {
    QString appPath = QCoreApplication::applicationDirPath();
    QString imgDirPath = appPath + "/img";
    QDir imgDir(imgDirPath);
    if (!imgDir.exists()) imgDir.mkpath(".");

    if (isMirror) {
        QVector<int> mirrorQueens = queens;
        for(int& col : mirrorQueens) {
            if (col != -1) col = (boardSize - 1) - col;
        }
        chessboard->setQueensManually(mirrorQueens);
        chessboard->repaint(); // 强制重绘以供 grab() 抓取
    }

    QPixmap pixmap = chessboard->grab();
    QString fileName = QString("%1/solution_%2.png").arg(imgDirPath).arg(solutionIndex);
    pixmap.save(fileName, "PNG");

    if (isMirror) {
        chessboard->setQueensManually(queens); // 恢复原状
        chessboard->repaint();
    }
}

} // namespace UI
} // namespace NQueens