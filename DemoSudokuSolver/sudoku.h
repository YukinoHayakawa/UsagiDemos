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
    int row_b = 0;
    int col_b = 0;
    int row_c = 0;
    int col_c = 0;

    Position(int row, int col)
    {
        row_b = row / 3; // row_of_block
        row_c = row % 3; // row_in_block
        col_b = col / 3; // col_of_block
        col_c = col % 3; // col_in_block
    }

    void print() const
    {
        fmt::print("block {}-{}, cell {}-{}", row_b, col_b, row_c, col_c);
    }
};

using Queue = std::vector<Position>;

struct SudokuBlock
{
    // the numbers in the block by [row][column]. 0 means empty.
    int block[3][3] = { };

    bool not_filled(int row, int col) const
    {
        return block[row][col] == 0;
    }

    bool filled(int row, int col) const
    {
        return block[row][col] != 0;
    }

    void fill(int row, int col, int num)
    {
        block[row][col] = num;
    }
};

struct SudokuInstance
{
    SudokuBlock blocks[3][3];
    int steps = 0;

    void fill(const Position &p, int num)
    {
        blocks[p.row_b][p.col_b].fill(
            p.row_c,
            p.col_c,
            num
        );
    }

    bool filled(const Position &p) const
    {
        return blocks[p.row_b][p.col_b].filled(
            p.row_c,
            p.col_c
        );
    }

    std::bitset<11> candidates(const Position &pos) const
    {
        // the numbers are all available initially
        std::bitset<11> ret = 0b111'111'111'0;
        auto &block = blocks[pos.row_b][pos.col_b];

        // check within the block
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                ret[block.block[i][j]] = false;

        // check the same row
        for(int i = 0; i < 3; ++i) // col of blocks
            for(int j = 0; j < 3; ++j) // col inside block
                ret[blocks[pos.row_b][i].block[pos.row_c][j]] = false;

        // check the same column
        for(int i = 0; i < 3; ++i) // row of blocks
            for(int j = 0; j < 3; ++j) // row inside block
                ret[blocks[i][pos.col_b].block[j][pos.col_c]] = false;

        return ret;
    }

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

    bool solve(Queue &queue, std::size_t index = 0)
    {
        // base case: all the numbers are filled, a solution was found.
        if(index == queue.size()) return true;

        std::sort(queue.begin() + index, queue.end(), [this](auto &&lhs, auto &&rhs)
        {
            return candidates(lhs).count() < candidates(rhs).count();
        });

        const auto &pos = queue[index];
        auto &block = blocks[pos.row_b][pos.col_b];
        assert(block.not_filled(pos.row_c, pos.col_c));
        const auto numbers = candidates(pos);

        // base case: the reduced domain is empty, backtracks.
        if(numbers == 0) return false;

        // recursive case: try each of the available numbers
        for(int i = 0; i < numbers.size(); ++i)
        {
            if(numbers[i] == false) continue;
            // attempt a candidate
            block.fill(pos.row_c, pos.col_c, i);
            ++steps;

            if(!lookahead(pos)) continue;

            if(gDebug)
            {
                fmt::print("Attempting number {} at ", i);
                pos.print();
                putchar(':\n\n');

                print();
            }

            // so we are temporarily done with the current cell, descend one
            // level down the search tree by attempting the next empty cell
            const auto solved = solve(queue, index + 1);
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
        block.fill(pos.row_c, pos.col_c, 0);
        return false;
    }

    void print() const
    {
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                const Position pos(i, j);
                const auto num = blocks[pos.row_b][pos.col_b].block[pos.row_c][pos.col_c];
                fmt::print("{} ", num);
                if(j % 3 == 2) putchar(' ');
            }
            if(i % 3 == 2) putchar('\n');
            putchar('\n');
        }
    }

    bool validate() const
    {
        bool v = true;
        for(int i = 0; i < 9; ++i)
        {
            for(int j = 0; j < 9; ++j)
            {
                const Position pos(i, j);
                if(candidates(pos) != 0)
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

    Queue unfilled;

    void read_from_file(const std::string &filename)
    {
        std::ifstream input { filename };

        int num, row = 0, col = 0;
        while(input >> num)
        {
            Position p(row, col);
            if(num == 0)
            {
                unfilled.push_back(p);
            }
            fill(p, num);
            ++col;
            if(col == 9)
            {
                ++row;
                col = 0;
            }
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

