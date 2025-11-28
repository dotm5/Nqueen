#include "NQueensSolver.h"
#include <cmath>

namespace NQueens {
namespace Core {

NQueensSolver::NQueensSolver(int n)
    : n(n), queens(n, -1), solutionsFound(0), stepsCount(0), row(0), col(-1)
{
    history_col.resize(n + 1, 0);
    history_ld.resize(n + 1, 0);
    history_rd.resize(n + 1, 0);
}

bool NQueensSolver::hasConflict(int r, int c) const {
    int col_mask = history_col[r];
    int ld_mask = history_ld[r];
    int rd_mask = history_rd[r];
    int bit = (1 << c);
    return (col_mask | ld_mask | rd_mask) & bit;
}

SolverState NQueensSolver::nextStep() {
    SolverState state;
    state.isFinished = false;
    state.newSolutionsFound = 0;
    state.isSymmetricBase = false;

    while (row >= 0 && row < n) {
        col++;
        int limit = n;
        if (row == 0) {
            limit = (n + 1) / 2;
        }

        if (col >= limit) {
            queens[row] = -1;
            row--;
            if (row >= 0) col = queens[row];
            continue;
        }

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
                state.solutionFound = true;
                bool hasMirror = true;
                if (n % 2 != 0 && queens[0] == n / 2) {
                    hasMirror = false;
                }

                if (hasMirror) {
                    solutionsFound += 2;
                    state.newSolutionsFound = 2;
                    state.isSymmetricBase = true;
                } else {
                    solutionsFound += 1;
                    state.newSolutionsFound = 1;
                    state.isSymmetricBase = false;
                }

                state.solutionsCount = solutionsFound;
                // 不 row--，等待下次循环继续回溯
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

int NQueensSolver::getSolutionsCount() const { return solutionsFound; }
int NQueensSolver::getStepsCount() const { return stepsCount; }

} // namespace Core
} // namespace NQueens