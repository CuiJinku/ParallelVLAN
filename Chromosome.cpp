#include <cstdlib>
#include <random>
#include <algorithm>
#include <chrono>
#include "Chromosome.h"

Chromosome::Chromosome(int row, int col) : row(row), col(col) {

    srand(1);
    for (int i = 0; i < row; i++) {
        bool allZero = true;
        std::vector<int> tmp (col);
        for (int j = 0; j < col; j++) {
            tmp[j] = rand() % 2;
            if (1 == tmp[j]) {
                allZero == false;
            }
        }
        if (allZero) {
            int j = rand() % col;
            tmp[j] = 1;      // computer must be in a vlan
        }
        genes.push_back(tmp);
    }
}

Chromosome::Chromosome() {}

int Chromosome::getRow() const {
    return row;
}

void Chromosome::setRow(int row) {
    Chromosome::row = row;
}

int Chromosome::getCol() const {
    return col;
}

void Chromosome::setCol(int col) {
    Chromosome::col = col;
}

const std::vector<std::vector<int>> &Chromosome::getGenes() const {
    return genes;
}

int Chromosome::getNperm() const {
    return nperm;
}

void Chromosome::setNperm(int nperm) {
    Chromosome::nperm = nperm;
}

int Chromosome::getNforb() const {
    return nforb;
}

void Chromosome::setNforb(int nforb) {
    Chromosome::nforb = nforb;
}

int Chromosome::getNvlan() const {
    return nvlan;
}

void Chromosome::setNvlan(int nvlan) {
    Chromosome::nvlan = nvlan;
}

void Chromosome::setGenes(const std::vector<std::vector<int>> &genes) {
    Chromosome::genes = genes;
}

void Chromosome::coinFlip() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen (seed);
    std::uniform_real_distribution<double> rdis(0.0,1.0);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (rdis(gen) < 0.05) {
                genes[i][j] = 1 - genes[i][j];
            }
        }
    }
}

void Chromosome::voteTune(int** p) {

    int** ap = new int* [row];
    for (int i = 0; i < row; i++) {
        ap[i] = new int[row];
    }

    int** vzero = new int* [row];
    for (int i = 0; i < row; i++) {
        vzero[i] = new int [col];
    }

    int** vone = new int* [row];
    for (int i = 0; i < row; i++) {
        vone[i] = new int [col];
    }

    for(int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            for (int k = 0; k < col; k++) {
                ap[i][j] |= genes[i][k] & genes[j][k];
            }
        }
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen (seed);
    std::uniform_int_distribution<int> idis(0, col-1); // [0, col-1]
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            if (i == j ) {
                if (0 == p[i][j]) {
                    printf("Unrealistic policy\n");
                    exit(-2);
                }
                else {
                    if (0 == ap[i][j]) {
                        int k = idis(gen);
                        vone[i][k]++;
                        // printf("The %2d computer is not in any VLAN\n", i);
                        // exit(-3);
                    }
                }
            }
            else {
                if (1==ap[i][j] && 0==p[i][j]) {
                    for (int k = 0; k < col; k++) {
                        if (1==genes[i][k] && 1==genes[j][k]) {
                            int choice = rand()%2;
                            if (0 == choice) vzero[i][k]++;
                            else vzero[j][k]++;
                        }
                    }
                }
                if (0==ap[i][j] && 1==p[i][j]) {
                    int k = rand() % col;
                    // pairs could be (0, 0), (0, 1), (1, 0)
                    if (0==genes[i][k] && 0==genes[j][k]) {
                        vone[i][k]++;
                        vone[j][k]++;
                    }
                    else if (0==genes[i][k] && 1==genes[j][k]) {
                        vone[i][k]++;
                    }
                    else { //if (1==genes[i][k] && 0==genes[j][k])
                        vone[j][k]++;
                    }
                }
            }
        }
    }

    for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (vone[i][j] > vzero[i][j]) genes[i][j] = 1;
            else if (vone[i][j] < vzero[i][j]) genes[i][j] = 0;
            else {
                if (0 != vone[i][j]) genes[i][j] = rand() % 2;
            }
        }
    }

    for (int i = 0; i < row; i++) {
        delete[] vone[i];
    }
    delete[] vone;

    for (int i = 0; i < row; i++) {
        delete[] vzero[i];
    }
    delete[] vzero;

    for (int i = 0; i < row; i++) {
        delete[] ap[i];
    }
    delete[] ap;
}

void Chromosome::delColum() {
    /*
    std::default_random_engine gen;
    std::uniform_real_distribution<double> rdis(0.0,1.0);
    double prob = rdis(gen);
    if (prob < 0.5) {
     */
    int ncol = rand() % col;
    std::vector<std::vector<int>> tmp;
    for (int i = 0; i < row; i++) {
        std::vector<int> r;
        for(int j = 0; j < col; j++) {
            if (j != ncol) {
                r.push_back(genes[i][j]);
            }
        }
        tmp.push_back(r);
    }
    genes = tmp;
}

void Chromosome::trimGene() {

    // transpose
    std::vector<std::vector<int>> t;
    for (int j = 0; j < col; j++) {
        std::vector<int> tmp;
        for (int i = 0; i < row; i++) {
            tmp.push_back(genes[i][j]);
        }
        t.push_back(tmp);
    }

    // remove duplicates
    std::vector<std::vector<int>>::iterator it;
    it = std::unique (t.begin(), t.end());
    t.resize(std::distance(t.begin(),it));

    // remove zero rows in transpose
    std::vector<int> zero(col, 0);
    it = std::find(t.begin(), t.end(), zero);
    if (it != t.end()) {
        t.erase(it);
    }

    row = t[0].size();
    col = t.size();
    std::vector<std::vector<int>> g;
    for (int j = 0; j < row; j++) {
        std::vector<int> tmp;
        for (int i = 0; i < col; i++) {
            tmp.push_back(t[i][j]);
        }
        g.push_back(tmp);
    }
    genes = g;
}

void Chromosome::updateMatrics(int **p) {
    // approximate policy matrix
    int** ap = new int* [row];
    for (int i = 0; i < row; i++) {
        ap[i] = new int[row];
    }

    nperm = 0;
    nforb = 0;
    for(int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            for (int k = 0; k < col; k++) {
                ap[i][j] |= genes[i][k] & genes[j][k];
            }
            if (ap[i][j] == p[i][j]) {
                if (1 == ap[i][j]) {
                    nperm++;
                }
                else {
                    nforb++;
                }
            }
        }
    }


    for (int i = 0; i < row; i++) {
        delete[] ap[i];
    }
    delete[] ap;
}