#ifndef PARALLELVLAN_CHROMOSOME_H
#define PARALLELVLAN_CHROMOSOME_H


#include <vector>

class Chromosome {
private:
    std::vector<std::vector<int>> genes;
    int row;
    int col;
    int nperm;
    int nforb;
    int nvlan;

public:
    Chromosome();

    Chromosome(int row, int col);

    // bool operator==(const Chromosome &rhs) const;

    const std::vector<std::vector<int>> &getGenes() const;

    void setGenes(const std::vector<std::vector<int>> &genes);

    void coinFlip();

    void voteTune(int** p);

    void delColum();

    void trimGene();

    void updateMatrics(int **p);

    int getRow() const;

    void setRow(int row);

    int getCol() const;

    void setCol(int col);

    int getNperm() const;

    void setNperm(int nperm);

    int getNforb() const;

    void setNforb(int nforb);

    int getNvlan() const;

    void setNvlan(int nvlan);
};


#endif //PARALLELVLAN_CHROMOSOME_H
