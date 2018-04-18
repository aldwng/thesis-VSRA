#include "VRR_basic.h"
#include "VRR_graph.h"

#ifndef VRR_VOTING_H
#define VRR_VOTING_H

class Voter
{
public:
    int id;
    vector<vector<double> > adj_load;
    vector<int> path_record;
    vector<double> evaluate_list;

	int alpha = RMFACTOR;
	int accepted = 0;
	int aborted = 0;
	// used for reinforcement mechanism

	Voter(){ ; }

    Voter(int ID, Graph* GRAPH)
	{
        id = ID;
        adj_load.resize(GRAPH->n);
        for (int i = 0; i < GRAPH->n; i++)
		{
            adj_load[i].resize(GRAPH->n);
        }
    }

    void init()
	{
        path_record.clear();
        evaluate_list.clear();
        evaluate_list.resize(DECNUM);
    }

    virtual void propose(Request* REQ) = 0;

    virtual void evaluate(Request* REQ, vector<Voter*> &VOTERS) = 0;

    ~Voter(){ ; }

};

// notice: table[i][j] means that voter i evaluate candidate j
// notice: int Voting class, table[i][j] means that voter j evaluate candidate i
class Voting
{
public:
    vector<vector<double> > table;
    vector<int> weight;

    int candidate_num, voter_num;

	// notice: the smallest value means highest happiness
    Voting(vector<vector<double> > &TABLE, vector<int> &WEIGHTS, int CANDIDATES, int VOTERS)
	{

        candidate_num = CANDIDATES;
        voter_num = VOTERS;
        table.resize(candidate_num, vector<double>(voter_num, 0.0));

        for (int i = 0; i < candidate_num; i++)
		{
            for (int j = 0; j < voter_num; j++)
			{
                table[i][j] = TABLE[j][i];
            }
        }

        weight.resize(voter_num, 0);
        for (int i = 0; i < voter_num; i++)
		{
            weight[i] = WEIGHTS[i];
        }

    }

    int voting(string method)
	{
		if (method == "RangeVoting") return range_voting(RANGE);
		else return 0;
	}

    int range_voting(int R) // R = range
	{
		vector<vector<int> > modified_table(candidate_num, vector<int>(voter_num, 0));
        int winner;
        vector<double> min_score(voter_num, INF);
        vector<int> sum_score(candidate_num, 0);
        int max_value = 0;
        for (int i = 0; i < voter_num; i++)
		{
            for (int j = 0; j < candidate_num; j++)
			{
                if (min_score[i] > table[j][i])
				{
                    min_score[i] = table[j][i];
                }
            }
        }
        for (int i = 0; i < voter_num; i++)
		{
            for (int j = 0; j < candidate_num; j++)
			{
                modified_table[j][i] = (int)(RANGE * (min_score[i] / table[j][i]));
				sum_score[j] += modified_table[j][i] * weight[i];
			}
        }

        for (int i = 0; i < candidate_num; i++)
		{
            if (max_value < sum_score[i])
			{
                max_value = sum_score[i];
                winner = i;
            }
        }
        return winner;
    }

    ~Voting(){;}
};

#endif
