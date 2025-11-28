#pragma once
#include <QVector>
#include "common/Types.h"

namespace NQueens {
	namespace Core {

		class NQueensSolver {
		public:
			explicit NQueensSolver(int n);

			// 执行下一步搜索
			SolverState nextStep();

			int getSolutionsCount() const;
			int getStepsCount() const;

		private:
			bool hasConflict(int r, int c) const;

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

	} // namespace Core
} // namespace NQueens