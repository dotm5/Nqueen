#pragma once
#include <QWidget>
#include <QVariantAnimation>
#include "common/Types.h"
#include "common/Config.h"

namespace NQueens {
	namespace UI {

		class ChessboardWidget : public QWidget {
			Q_OBJECT
			Q_PROPERTY(qreal animatedRadius READ getAnimatedRadius WRITE setAnimatedRadius)

		public:
			explicit ChessboardWidget(int size, QWidget *parent = nullptr);

			void setBoardSize(int size);
			void setQueensManually(const QVector<int>& queens);
			void setAnimationSpeed(int durationMs);
			void setState(const SolverState &state);

			qreal getAnimatedRadius() const { return animatedRadius; }
			void setAnimatedRadius(qreal r);

		protected:
			void resizeEvent(QResizeEvent *event) override;
			void paintEvent(QPaintEvent *event) override;

		private:
			void drawQueens(QPainter &painter);
			void drawSingleQueen(QPainter &painter, qreal cx, qreal cy, qreal radius, const QColor &color, const QString &text);

			int boardSize;
			SolverState boardState;
			QPair<int, int> currentTrialPos;
			qreal animatedRadius;
			qreal cellSize;
			qreal boardOffsetX, boardOffsetY;
			QVariantAnimation *animation;
		};

	} // namespace UI
} // namespace NQueens