#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <array>
#include <omp.h>
#include "Chromosome.h"

using namespace std;

bool comp(Chromosome &a, Chromosome &b)
{
    // return a.getNperm() > b.getNperm();
    return a.getNperm() + a.getNforb() + a.getRow() - a.getCol() > b.getNperm() + b.getNforb() + b.getRow() - b.getCol();
    // bool ans;
    // if (a.getNperm() > b.getNperm())
    // {
    //     ans = true;
    // }
    // else if (a.getNperm() < b.getNperm())
    // {
    //     ans = false;
    // }
    // else
    // {
    //     if (a.getNforb() > b.getNforb())
    //     {
    //         ans = true;
    //     }
    //     else if (a.getNforb() < b.getNforb())
    //     {
    //         ans = false;
    //     }
    //     else
    //     {
    //         if (a.getCol() < b.getCol())
    //         {
    //             ans = true;
    //         }
    //         else if (a.getCol() >= b.getCol())
    //         {
    //             ans = false;
    //         }
    //     }
    // }
    // return ans;
}

int main()
{

    const int ncomp = 20;
    // allocate space for policy matrix
    int **policy = new int *[ncomp];
    for (int i = 0; i < ncomp; i++)
    {
        policy[i] = new int[ncomp];
    }

    ifstream file;
    file.open("policy20x3.txt");
    if (!file.is_open())
    {
        cout << "Cannot open policy file" << endl;
        exit(-1);
    }
    else
    {
        for (int i = 0; i < ncomp; i++)
        {
            for (int j = 0; j < ncomp; j++)
            {
                file >> policy[i][j];
            }
        }
    }
    file.close();
    /*
    // generate policy matrix
    srand(1);
    for (int i = 0; i < ncomp; i++) {
        for (int j = 0; j < ncomp; j++) {
            if (i == j) {
                policy[i][j] = 1;
            }
            else {
                policy[i][j] = policy[j][i] = rand() % 2;
            }
        }
    }
     */

    // diagonal must be 1
    for (int i = 0; i < ncomp; i++)
    {
        assert(1 == policy[i][i]);
    }

#ifdef PRINT
    for (int i = 0; i < ncomp; i++)
    {
        for (int j = 0; j < ncomp; j++)
        {
            printf("%2d ", policy[i][j]);
        }
        printf("\n");
    }
#endif

    // generate population
    const int npopu = 400;
    // construct a trivial random generator engine from a time-based seed:
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    // default_random_engine generator (seed);
    default_random_engine gen(seed);
    double mu = ncomp / 2;
    double sigma = ncomp / 5;
    normal_distribution<> dis(mu, sigma);
    const int low = mu - 2 * sigma;
    const int high = mu + 2 * sigma;

    int *nvlans = new int[npopu];
    for (int i = 0; i < npopu; i++)
    {
        int nvlan = round(dis(gen));
        if (nvlan < low)
            nvlan = low;
        if (nvlan > high)
            nvlan = high;
        nvlans[i] = nvlan;
    }

    vector<Chromosome> population(npopu);
    for (int i = 0; i < npopu; i++)
    {
        population[i] = Chromosome(ncomp, nvlans[i]);
        population[i].updateMatrics(policy);
    }

// print the populations
#ifdef PRINT
    printf("print the populations\n");
    for (int i = 0; i < npopu; i++)
    {
        Chromosome temp = population[i];
        for (int r = 0; r < temp.getRow(); r++)
        {
            for (int c = 0; c < temp.getCol(); c++)
            {
                printf("%2d", temp.getGenes()[r][c]);
            }
            printf("\n");
        }
    }
#endif

    double start = omp_get_wtime();

    // number of generations or iterations
    array<Chromosome, 10> head;
    const int ngenn = 120;

    uniform_real_distribution<double> rdis(0.0, 1.0);
    for (int i = 0; i < ngenn; i++)
    {
        vector<Chromosome> paretoFrontier;
        vector<Chromosome> nonFrontier;
        while (!population.empty())
        {
            sort(population.begin(), population.end(), comp);
            Chromosome frontier = population.front();
            paretoFrontier.push_back(frontier);
            population.erase(population.begin());

            vector<Chromosome> rest;
            for (Chromosome chromosome : population)
            {
                if (!(frontier.getNperm() >= chromosome.getNperm() &&
                      frontier.getNforb() >= chromosome.getNforb() &&
                      frontier.getCol() <= chromosome.getCol()))
                {
                    rest.push_back(chromosome);
                }
                else
                {
                    nonFrontier.push_back(chromosome);
                }
            }
            population = rest;
        }

#ifdef PRINT
        int a = paretoFrontier.size();
        int c = nonFrontier.size();
        printf("%2d ", a);
        printf("%2d\n", c);
#endif

#ifdef PRINT
    printf("Frontier\n");
    for (Chromosome chromosome : paretoFrontier)
    {
        printf("%3d%3d%3d\n", chromosome.getNperm(), chromosome.getNforb(), chromosome.getCol());
    }

    printf("nonFrontier\n");
    for (Chromosome chromosome : nonFrontier)
    {
        printf("%3d%3d%3d\n", chromosome.getNperm(), chromosome.getNforb(), chromosome.getCol());
    }
#endif

#ifdef LOG
        char *buffer = new char[30];
        sprintf(buffer, "./frontier/frontier%03d.txt", i);
        string path = buffer;
        ofstream outFile(path);
        for (Chromosome chromosome : paretoFrontier) {
            outFile << chromosome.getNperm() << " " << chromosome.getNforb() << " " << chromosome.getCol() << endl;
        }
        outFile.flush();
        outFile.close();
        delete[] buffer;
#endif

        vector<Chromosome> nextPopulation = paretoFrontier;
        while (nextPopulation.size() < npopu)
        {
            // start selection 
            // if the size of pareto frontier is large enough, parents are chosen from the frontier
            // else if frontier is not emtpy, one from frontier, one from non-frontier
            // else, both from non frontier
            Chromosome chromosome1, chromosome2;
            double density = 0.40;
            if (paretoFrontier.size() > npopu * density)
            {
                chromosome1 = paretoFrontier.at(rand() % paretoFrontier.size());
                chromosome2 = paretoFrontier.at(rand() % paretoFrontier.size());
            }
            else if (paretoFrontier.size() > 0)
            {
                chromosome1 = paretoFrontier.at(rand() % paretoFrontier.size());
                chromosome2 = nonFrontier.at(rand() % nonFrontier.size());
            }
            else
            {
                chromosome1 = nonFrontier.at(rand() % nonFrontier.size());
                chromosome2 = nonFrontier.at(rand() % nonFrontier.size());
            }
            // end selection 


            // start crossover
            // double prob = rdis(gen);
            vector<vector<int>> genes1 = chromosome1.getGenes();
            vector<vector<int>> genes2 = chromosome2.getGenes();
            int colRange = min(chromosome1.getCol(), chromosome2.getCol());
            uniform_int_distribution<int> ndis(1, colRange);   // [1, colRange]
            int ntimes = ndis(gen);
            for (int j = 0; j < ntimes; j++) {
                int nCol = ndis(gen) - 1;
                for (int k = 0; k < ncomp; k++) {
                    swap(genes1[k][nCol], genes2[k][nCol]);
                }
            }
            chromosome1.setGenes(genes1);
            chromosome2.setGenes(genes2);
            // end crossover 


            // start mutation 
            double prob = rdis(gen);
            if (prob > 0.5) {
                chromosome1.coinFlip();
                chromosome2.coinFlip();
            }
            else if (prob > 0.3)
            {
                chromosome1.voteTune(policy);
                chromosome2.voteTune(policy);
            }
            // end mutation 


            chromosome1.trimGene();
            // printf("%2d %2d %2d\n", chromosome1.getNperm(), chromosome1.getNforb(), chromosome1.getCol());
            chromosome2.trimGene();
            chromosome1.updateMatrics(policy);
            chromosome2.updateMatrics(policy);
            

            if (chromosome1.getNperm() > chromosome2.getNperm() &&
                chromosome1.getNforb() > chromosome2.getNforb() &&
                chromosome1.getCol() < chromosome2.getCol())
            {
                nextPopulation.push_back(chromosome1);
            }
            else if (chromosome2.getNperm() > chromosome1.getNperm() &&
                     chromosome2.getNforb() > chromosome1.getNforb() &&
                     chromosome2.getCol() < chromosome1.getCol())
            {
                nextPopulation.push_back(chromosome2);
            }
            else
            {
                nextPopulation.push_back(chromosome1);
                nextPopulation.push_back(chromosome2);
            }
        }

        population = nextPopulation;

    }
    double elapsed = omp_get_wtime() - start;
    printf("time: %f\n", elapsed);

    delete[] nvlans;

    for (int i = 0; i < ncomp; i++)
    {
        delete[] policy[i];
    }
    delete[] policy;

    return 0;
}
