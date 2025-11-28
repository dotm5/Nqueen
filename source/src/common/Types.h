#pragma once
#include <QVector>
#include <QPair>

namespace NQueens {

	struct SolverState {
		QVector<int> queens;
		QPair<int, int> trialPos;
		bool hasConflict;
		bool solutionFound;
		int solutionsCount;
		int newSolutionsFound;
		int stepsCount;
		bool isFinished;
		bool isSymmetricBase;
	};

} // namespace NQueens