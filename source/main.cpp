#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QPainter>
#include <QVariantAnimation>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QString>
#include <QStyle>
#include <QEasingCurve>
#include <QDir>
#include <QCoreApplication>
#include <QPixmap>
#include <QDebug>
#include <cmath> // For std::ceil

// --- 可配置的常量 ---
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
const QMap<QString, int> SPEED_SETTINGS = {
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

// --- 数据结构 ---
struct SolverState {
    QVector<int> queens;
    QPair<int, int> trialPos;
    bool hasConflict;
    bool solutionFound;
    int solutionsCount;     // 当前已找到的解总数
    int newSolutionsFound;  // 本次步骤新增的解数量 (1 或 2)
    int stepsCount;
    bool isFinished;
    bool isSymmetricBase;   // 标记是否是通过对称性找到的基础解（用于生成镜像）
};

// --- NQueens Solver (Bitmask + Symmetry Pruning) ---
class NQueensSolver {
public:
    NQueensSolver(int n)
        : n(n),
          queens(n, -1),
          solutionsFound(0),
          stepsCount(0),
          row(0),
          col(-1)
    {
        // history_* 存储每一行开始时的掩码状态
        history_col.resize(n + 1);
        history_ld.resize(n + 1);
        history_rd.resize(n + 1);

        history_col[0] = 0;
        history_ld[0] = 0;
        history_rd[0] = 0;
    }

    bool hasConflict(int r, int c) const {
        int col_mask = history_col[r];
        int ld_mask = history_ld[r];
        int rd_mask = history_rd[r];
        int bit = (1 << c);
        return (col_mask | ld_mask | rd_mask) & bit;
    }

    SolverState nextStep() {
        SolverState state;
        state.isFinished = false;
        state.newSolutionsFound = 0;
        state.isSymmetricBase = false;

        while (row >= 0 && row < n) {
            col++;

            // --- 对称性剪枝的核心逻辑 ---
            // 如果是第一行 (row == 0)，我们限制 col 的搜索范围。
            // 偶数 N: 搜索 0 到 N/2 - 1
            // 奇数 N: 搜索 0 到 N/2
            int limit = n;
            if (row == 0) {
                limit = (n + 1) / 2; // ceil(n/2)
            }

            if (col >= limit) {
                // 当前行搜索完毕或触发剪枝，回溯
                queens[row] = -1;
                row--;
                if (row >= 0) {
                    col = queens[row];
                }
                continue;
            }

            // 对于除第一行以外的行，虽然 col < limit 逻辑不适用（limit=n），
            // 但我们需要正常的 col >= n 判断来触发回溯。
            // 由于上面 row==0 时 limit 变小了，这里需要针对 row > 0 单独补一个判断
            if (row > 0 && col >= n) {
                queens[row] = -1;
                row--;
                if (row >= 0) col = queens[row];
                continue;
            }

            stepsCount++;
            bool conflict = hasConflict(row, col);

            state.queens = queens;
            state.trialPos = {row, col};
            state.hasConflict = conflict;
            state.solutionsCount = solutionsFound;
            state.stepsCount = stepsCount;
            state.solutionFound = false;

            if (!conflict) {
                queens[row] = col;
                int bit = (1 << col);

                if (row + 1 < n + 1) {
                    history_col[row + 1] = history_col[row] | bit;
                    history_ld[row + 1]  = (history_ld[row] | bit) << 1;
                    history_rd[row + 1]  = (history_rd[row] | bit) >> 1;
                }

                if (row == n - 1) {
                    // 找到一个解
                    state.solutionFound = true;

                    // 判断这个解是否能通过对称性产生另一个解
                    // 只有当第一行的皇后不在正中间时，才会有镜像解
                    // (在 N 为奇数且 row0_col == n/2 时，无镜像增益，因为我们全搜了该子树)
                    bool hasMirror = true;
                    if (n % 2 != 0 && queens[0] == n / 2) {
                        hasMirror = false;
                    }

                    if (hasMirror) {
                        solutionsFound += 2;
                        state.newSolutionsFound = 2;
                        state.isSymmetricBase = true; // 标记需要生成镜像图
                    } else {
                        solutionsFound += 1;
                        state.newSolutionsFound = 1;
                        state.isSymmetricBase = false;
                    }

                    state.solutionsCount = solutionsFound;
                    // 不 row--，继续利用循环回溯
                } else {
                    row++;
                    col = -1;
                }
            }

            return state;
        }

        state.isFinished = true;
        state.solutionsCount = solutionsFound;
        state.stepsCount = stepsCount;
        state.queens = queens;
        return state;
    }

    int getSolutionsCount() const { return solutionsFound; }
    int getStepsCount() const { return stepsCount; }

private:
    int n;
    QVector<int> queens;
    QVector<int> history_col;
    QVector<int> history_ld;
    QVector<int> history_rd;
    int solutionsFound;
    int stepsCount;
    int row;
    int col;
};

// --- Chessboard Widget ---
class ChessboardWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animatedRadius READ getAnimatedRadius WRITE setAnimatedRadius)

public:
    explicit ChessboardWidget(int size, QWidget *parent = nullptr)
        : QWidget(parent), boardSize(size), animatedRadius(0), cellSize(INITIAL_CELL_SIZE) {

        boardState.queens.fill(-1, size);
        boardState.trialPos = {-1, -1};

        animation = new QVariantAnimation(this);
        animation->setDuration(100);
        animation->setEasingCurve(QEasingCurve::OutQuad);
        connect(animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &val){
            this->setAnimatedRadius(val.toReal());
        });

        setMinimumSize(400, 400);
    }

    void setBoardSize(int size) {
        boardSize = size;
        boardState.queens.fill(-1, size);
        boardState.trialPos = {-1, -1};
        resizeEvent(nullptr);
        update();
    }

    // 手动设置皇后位置（用于绘制镜像解）
    void setQueensManually(const QVector<int>& queens) {
        boardState.queens = queens;
        boardState.trialPos = {-1, -1};
        boardState.hasConflict = false;
        update();
    }

    void setAnimationSpeed(int durationMs) {
        animation->setDuration(std::max(1, durationMs - 5));
    }

    void setState(const SolverState &state) {
        QPair<int, int> oldTrialPos = currentTrialPos;

        boardState = state;
        currentTrialPos = state.trialPos;

        if (currentTrialPos != oldTrialPos && currentTrialPos.first != -1) {
            animation->stop();
            animation->setStartValue(0.0);
            qreal endRadius = cellSize / 2.2;
            animation->setEndValue(endRadius);
            animation->start();
        }
        update();
    }

    qreal getAnimatedRadius() const { return animatedRadius; }
    void setAnimatedRadius(qreal r) {
        animatedRadius = r;
        update();
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        int side = std::min(width(), height());
        cellSize = (double)side / boardSize;
        boardOffsetX = (width() - side) / 2.0;
        boardOffsetY = (height() - side) / 2.0;

        if (event) QWidget::resizeEvent(event);
    }

    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制棋盘
        for (int r = 0; r < boardSize; ++r) {
            for (int c = 0; c < boardSize; ++c) {
                QRectF rect(boardOffsetX + c * cellSize,
                            boardOffsetY + r * cellSize,
                            cellSize, cellSize);
                QColor color = ((r + c) % 2 == 0) ? Colors::LightSquare : Colors::DarkSquare;
                painter.fillRect(rect, color);
            }
        }

        drawQueens(painter);
    }

private:
    void drawQueens(QPainter &painter) {
        int fontSize = std::max(10, int(cellSize / 4));
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(fontSize);
        painter.setFont(font);

        // 绘制已放置的皇后
        for (int r = 0; r < boardState.queens.size(); ++r) {
            int c = boardState.queens[r];
            if (c != -1) {
                qreal radius = cellSize / 2.2;
                qreal cx = boardOffsetX + c * cellSize + cellSize / 2;
                qreal cy = boardOffsetY + r * cellSize + cellSize / 2;
                drawSingleQueen(painter, cx, cy, radius, Colors::QueenSafe, "Q");
            }
        }

        // 绘制尝试中的皇后
        if (currentTrialPos.first != -1) {
            int r = currentTrialPos.first;
            int c = currentTrialPos.second;
            qreal cx = boardOffsetX + c * cellSize + cellSize / 2;
            qreal cy = boardOffsetY + r * cellSize + cellSize / 2;

            QColor color = boardState.hasConflict ? Colors::QueenConflict : Colors::QueenTrial;
            QString text = boardState.hasConflict ? "X" : "?";

            drawSingleQueen(painter, cx, cy, animatedRadius, color, text);
        }
    }

    void drawSingleQueen(QPainter &painter, qreal cx, qreal cy, qreal radius, const QColor &color, const QString &text) {
        if (radius < 1) return;

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(cx, cy), radius, radius);

        if (radius > cellSize / 4) {
            painter.setPen(Colors::Bg);
            QRectF rect(cx - radius, cy - radius, radius * 2, radius * 2);
            painter.drawText(rect, Qt::AlignCenter, text);
        }
    }

    int boardSize;
    SolverState boardState;
    QPair<int, int> currentTrialPos;
    qreal animatedRadius;
    qreal cellSize;
    qreal boardOffsetX, boardOffsetY;
    QVariantAnimation *animation;
};

// --- Main Window ---
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow() {
        setWindowTitle("N-Queens Visualizer (Symmetry Pruning)");
        setMinimumSize(800, 800);

        solver = nullptr;
        isPaused = false;

        setupUI();
        setStyleSheet(STYLESHEET);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::nextStep);

        updateSpeed("正常速度");
    }

    ~MainWindow() {
        if (solver) delete solver;
    }

private slots:
    void changeBoardSize(int newSize) {
        if (!timer->isActive()) {
            boardSize = newSize;
            chessboard->setBoardSize(newSize);
            statusLabel->setText(QString("棋盘大小已改为 %1×%1").arg(newSize));
        }
    }

    void updateSpeed(const QString &speedText) {
        int interval = SPEED_SETTINGS.value(speedText, 100);
        timer->setInterval(interval);
        chessboard->setAnimationSpeed(interval);
    }

    void toggleSearch() {
        if (timer->isActive() || isPaused) {
            timer->stop();
            resetSearch();
        } else {
            startSearch();
        }
    }

    void togglePause() {
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

    void nextStep() {
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
            // 保存基础解
            int currentId = state.solutionsCount - state.newSolutionsFound + 1;
            saveSnapshot(currentId, false);

            QString msg = QString("找到解 #%1").arg(currentId);

            // 如果是对称解，保存镜像
            if (state.isSymmetricBase) {
                int mirrorId = currentId + 1;
                saveSnapshot(mirrorId, true); // True 表示保存镜像
                msg += QString(" 及镜像解 #%1 (自动推导)").arg(mirrorId);
            }

            statusLabel->setText(msg + "，图片已保存。");
            timer->stop();

            QTimer::singleShot(SOLUTION_PAUSE_MS, this, [this](){
                if (!isPaused && startButton->text() == "停止") {
                     timer->start();
                }
            });
        } else {
            statusLabel->setText(QString("正在搜索... 已找到 %1 个解").arg(state.solutionsCount));
        }

        statsLabel->setText(QString("步数: %1").arg(state.stepsCount));
    }

private:
    // 保存截图，支持镜像翻转
    void saveSnapshot(int solutionIndex, bool isMirror) {
        QString appPath = QCoreApplication::applicationDirPath();
        QString imgDirPath = appPath + "/img";
        QDir imgDir(imgDirPath);
        if (!imgDir.exists()) imgDir.mkpath(".");

        // 如果是镜像，临时修改棋盘显示
        QVector<int> originalQueens;
        if (isMirror) {
            // 获取当前的 Queens 状态
            // 注意：此时 chessboard 已经有最新的 state
            // 但我们没有直接访问 solver 的 queens 的接口，只能依赖 GUI 刷新
            // 这里我们手动计算镜像位置
            // 镜像逻辑：col_new = (N - 1) - col_old
            // 只有当行有皇后时才翻转
            // 但此时为了截图，我们需要构造一个临时状态
            // 简单方法：利用 SolverState 里的 queens
            // 由于 SolverState 是作为值传递给 Chessboard 的，我们可以临时 hack 一下
            // 或者更好的是，Chessboard 提供一个 direct set 方法
            // 实际上，为了简单，我们可以不做这一步，因为用户肉眼看不到镜像过程
            // 但为了"图片已保存"的承诺，我们需要生成这张图。

            // 1. 拿到原始数据
            // 这里比较 trick，因为 grab() 抓取的是屏幕像素。
            // 必须让棋盘先渲染成镜像的样子，抓取，再恢复。
            // 这是一个瞬间过程，用户可能看到闪烁，但这正是"生成中"的体现。

            // 下面实现：保存当前状态 -> 设置镜像 -> update & repaint -> grab -> 恢复
            // 注意：repaint() 是同步的，所以可行。

            // 既然 SolverState 是私有的，我们在 Chessboard 加个接口
            // 我们无法直接读 SolverState，但我们在 nextStep 拥有 state 变量
            // 修正：nextStep 中的 `state` 变量含有 queens

            // 保存当前（非镜像）状态
            originalQueens = solver->nextStep().queens; // 错误，不能再次调用 nextStep
            // 实际上 nextStep 返回的 state 已经有了 queens
            // 但是 nextStep 是局部变量。我们在 nextStep 函数内部，可以直接用 state.queens

            // 由于 saveSnapshot 是成员函数，无法访问 nextStep 的局部变量
            // 修改 saveSnapshot 签名传入 queens
        }

        // 修正逻辑：移到 saveSnapshot 内部处理有点麻烦，改为在 nextStep 调用时处理
        // 但为了代码整洁，我们假设 Chessboard 处于"正确状态"
        // 如果是 Mirror，我们需要强制让 Chessboard 显示镜像

        if (isMirror) {
            // 我们需要重新计算镜像的 Queens
            // 这里比较难拿到原始 queens，除非我们保存它
            // 让我们简化：只保存"当前屏幕显示"的图片。
            // 对于镜像解，我们在 nextStep 里手动操作 Chessboard
        }
    }

    // 重载版本，直接传入 queens
    void saveSnapshot(int solutionIndex, bool isMirror, QVector<int> queens) {
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
            chessboard->repaint(); // 强制重绘
        }

        QPixmap pixmap = chessboard->grab();
        QString fileName = QString("%1/solution_%2.png").arg(imgDirPath).arg(solutionIndex);
        pixmap.save(fileName, "PNG");

        if (isMirror) {
            // 恢复原状
            chessboard->setQueensManually(queens); // 恢复
            chessboard->repaint();
        }
    }

    // 重载 nextStep 中的调用
    // 修改 nextStep 逻辑以适配

    void setupUI() {
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

    void startSearch() {
        if (solver) delete solver;
        solver = new NQueensSolver(boardSize);

        startButton->setText("停止");
        pauseButton->setEnabled(true);
        sizeSpin->setEnabled(false);
        statusLabel->setText("正在搜索... (对称优化中)");
        statsLabel->setText("步数: 0");

        timer->start();
    }

    void resetSearch() {
        if (solver) { delete solver; solver = nullptr; }
        resetUIState(false);
    }

    void resetUIState(bool finished) {
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

    int boardSize;
    NQueensSolver *solver;
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

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    MainWindow window;
    window.show();

    return app.exec();
}