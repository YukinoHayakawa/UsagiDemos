#pragma once

#include <bitset>
#include <cassert>
#include <fstream>
#include <vector>
#include <algorithm>
#include <fmt/ostream.h>

inline bool gDebug = false;

struct Position
{
    int row, col;

    Position(int row, int col)
        : row(row)
        , col(col)
    {
    }

    Position block_pos() const
    {
        return { row / 3, col / 3 };
    }

    void print() const
    {
        fmt::print("cell {}-{}", row, col);
    }
};

using Queue = std::vector<Position>;

struct SudokuInstance
{
    // SudokuBlock blocks[3][3];
    int cells[9][9] { };
    bool locked[9][9] { };
    int steps = 0;

    void fill(const Position &p, int num)
    {
        cells[p.row][p.col] = num;
    }

    void set_locked(const Position &p, bool locked_)
    {
        locked[p.row][p.col] = locked_;
    }

    bool is_locked(const Position &p)
    {
        return locked[p.row][p.col];
    }

    void toggle_locked(const Position & p)
    {
        locked[p.row][p.col] = !locked[p.row][p.col];
    }

    bool filled(const Position &p) const
    {
        return cells[p.row][p.col] != 0;
    }

    std::bitset<11> candidates(const Position &pos) const
    {
        // the numbers are all available initially
        std::bitset<11> ret = 0b111'111'111'0;

        // check within the block
        const auto block_pos = pos.block_pos();

        const auto ii = block_pos.row * 3;
        for(int i = ii; i < ii + 3; ++i)
        {
            const auto jj = block_pos.col * 3;
            for(int j = jj; j < jj + 3; ++j)
                ret[cells[i][j]] = false;
        }

        // check the same row
        for(int i = 0; i < 9; ++i)
            ret[cells[pos.row][i]] = false;

        // check the same column
        for(int i = 0; i < 9; ++i)
            ret[cells[i][pos.col]] = false;

        return ret;
    }

    /*
    bool lookahead(const Position &pos) const
    {
        // check within the block
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
            {
                auto p = pos;
                p.row_c = i;
                p.col_c = j;
                if(!filled(p) && candidates(p) == 0) return false;
            }

        // check the same row
        for(int i = 0; i < 3; ++i) // col of blocks
            for(int j = 0; j < 3; ++j) // col inside block
            {
                auto p = pos;
                p.col_b = i;
                p.col_c = j;
                if(!filled(p) && candidates(p) == 0) return false;
            }

        // check the same column
        for(int i = 0; i < 3; ++i) // row of blocks
            for(int j = 0; j < 3; ++j) // row inside block
            {
                auto p = pos;
                p.row_b = i;
                p.row_c = j;
                if(!filled(p) && candidates(p) == 0) return false;
            }

        return true;
    }
    */

    void reset()
    {
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                if(!locked[i][j])
                    cells[i][j] = 0;
            }
        }
    }

    bool solve__fewest_candidate_first()
    {
        steps = 0;
        Queue unfilled;

        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                if(cells[i][j] == 0)
                    unfilled.push_back({ i, j });
            }
        }

        const auto ret = solve__fewest_candidate_first__helper(unfilled);
        (void)validate();
        return ret;
    }

    bool solve__fewest_candidate_first__helper(Queue &unfilled, std::size_t index = 0)
    {
        // base case: all the numbers are filled, a solution was found.
        if(index == unfilled.size()) return true;

        std::sort(unfilled.begin() + index, unfilled.end(), [this](auto &&lhs, auto &&rhs)
        {
            return candidates(lhs).count() < candidates(rhs).count();
        });

        const auto &pos = unfilled[index];
        assert(!filled(pos));
        const auto numbers = candidates(pos);

        // base case: the reduced domain is empty, backtracks.
        if(numbers == 0) return false;

        // recursive case: try each of the available numbers
        for(int i = 0; i < numbers.size(); ++i)
        {
            if(numbers[i] == false) continue;
            // attempt a candidate
            fill(pos, i);
            ++steps;

            // if(!lookahead(pos)) continue;

            if(gDebug)
            {
                fmt::print("Attempting number {} at ", i);
                pos.print();
                putchar(':\n\n');

                print();
            }

            // so we are temporarily done with the current cell, descend one
            // level down the search tree by attempting the next empty cell
            const auto solved = solve__fewest_candidate_first__helper(unfilled, index + 1);
            // the subtree is dealt with
            if(solved) return true;
            // otherwise we try the next number
        }

        if(gDebug)
        {
            fmt::print("Failed. Resetting ");
            pos.print();
            fmt::print(" to 0.\n");
        }

        // run out of the candidate numbers!
        // revert the effects and report this solution as a failure.
        fill(pos, 0);
        return false;
    }

    void print() const
    {
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                fmt::print("{} ", cells[i][j]);
                if(j % 3 == 2) putchar(' ');
            }
            if(i % 3 == 2) putchar('\n');
            putchar('\n');
        }
    }

    void save(const std::string &filename) const
    {
        std::ofstream out { filename };

        // write numbers
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                fmt::print(out, "{} ", cells[i][j]);
                if(j % 3 == 2) out.put(' ');
            }
            if(i % 3 == 2) out.put('\n');
            out.put('\n');
        }
        out.put('\n');
        // write locks
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                fmt::print(out, "{} ", (int)locked[i][j]);
                if(j % 3 == 2) out.put(' ');
            }
            if(i % 3 == 2) out.put('\n');
            out.put('\n');
        }
    }

    bool validate(const Position &pos) const
    {
        // the numbers are all available initially
        std::bitset<11> ret = 0b111'111'111'0;

        // check within the block
        const auto block_pos = pos.block_pos();

        const auto ii = block_pos.row * 3;
        for(int i = ii; i < ii + 3; ++i)
        {
            const auto jj = block_pos.col * 3;
            for(int j = jj; j < jj + 3; ++j)
                ret[cells[i][j]] = false;
        }
        if(ret != 0) return false;

        ret = 0b111'111'111'0;
        // check the same row
        for(int i = 0; i < 9; ++i)
            ret[cells[pos.row][i]] = false;
        if(ret != 0) return false;

        ret = 0b111'111'111'0;
        // check the same column
        for(int i = 0; i < 9; ++i)
            ret[cells[i][pos.col]] = false;
        if(ret != 0) return false;

        return true;
    }

    bool validate() const
    {
        bool v = true;
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                const Position pos(i, j);
                if(!validate(pos))
                {
                    fmt::print("Constraint was not satisfied at: ");
                    pos.print();
                    putchar('\n');
                    v = false;
                }
            }
        }
        return v;
    }

    void read_from_file(const std::string &filename)
    {
        std::ifstream input { filename };

        int num, row = 0, col = 0;
        // read numbers
        while(input >> num)
        {
            Position p(row, col);
            fill(p, num);
            ++col;
            if(col == 9)
            {
                ++row;
                col = 0;
            }
            if(row == 9) break;
        }
        row = 0, col = 0;
        // read locks
        while(input >> num)
        {
            Position p(row, col);
            if(num != 0) set_locked({ row, col }, true);
            ++col;
            if(col == 9)
            {
                ++row;
                col = 0;
            }
            if(row == 9) break;
        }
    }
};


/*
int main(int argc, char *argv[])
{
    if(argc == 3)
        gDebug = true;

    SudokuInstance instance;


    fmt::print("Trying to solve the puzzle:\n\n");

    instance.print();

    fmt::print("\n");

    const auto solved = instance.solve(unfilled);
    if(!solved)
    {
        fmt::print("No solution could be found.\n");
        return 1;
    }

    fmt::print("Got a solution in {} steps:\n\n", instance.steps);
    instance.print();
    (void)instance.validate();


    // todo: probably all the candidate of all cells should be considered
    // together and sorted in the order of max size of subtree (most trimmed
    // domain)
    return 0;
}
*/

