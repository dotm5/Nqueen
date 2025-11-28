#include "ChessboardWidget.h"
#include <QPainter>
#include <QEasingCurve>
#include <cmath>

namespace NQueens {
namespace UI {

using namespace NQueens::Config;

ChessboardWidget::ChessboardWidget(int size, QWidget *parent)
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

void ChessboardWidget::setBoardSize(int size) {
    boardSize = size;
    boardState.queens.fill(-1, size);
    boardState.trialPos = {-1, -1};
    resizeEvent(nullptr);
    update();
}

void ChessboardWidget::setQueensManually(const QVector<int>& queens) {
    boardState.queens = queens;
    boardState.trialPos = {-1, -1};
    boardState.hasConflict = false;
    update();
}

void ChessboardWidget::setAnimationSpeed(int durationMs) {
    animation->setDuration(std::max(1, durationMs - 5));
}

void ChessboardWidget::setState(const SolverState &state) {
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

void ChessboardWidget::setAnimatedRadius(qreal r) {
    animatedRadius = r;
    update();
}

void ChessboardWidget::resizeEvent(QResizeEvent *event) {
    int side = std::min(width(), height());
    cellSize = (double)side / boardSize;
    boardOffsetX = (width() - side) / 2.0;
    boardOffsetY = (height() - side) / 2.0;
    if (event) QWidget::resizeEvent(event);
}

void ChessboardWidget::paintEvent(QPaintEvent *event) {
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

void ChessboardWidget::drawQueens(QPainter &painter) {
    int fontSize = std::max(10, int(cellSize / 4));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(fontSize);
    painter.setFont(font);

    // 绘制已放置
    for (int r = 0; r < boardState.queens.size(); ++r) {
        int c = boardState.queens[r];
        if (c != -1) {
            qreal radius = cellSize / 2.2;
            qreal cx = boardOffsetX + c * cellSize + cellSize / 2;
            qreal cy = boardOffsetY + r * cellSize + cellSize / 2;
            drawSingleQueen(painter, cx, cy, radius, Colors::QueenSafe, "Q");
        }
    }

    // 绘制尝试中
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

void ChessboardWidget::drawSingleQueen(QPainter &painter, qreal cx, qreal cy, qreal radius, const QColor &color, const QString &text) {
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

} // namespace UI
} // namespace NQueens