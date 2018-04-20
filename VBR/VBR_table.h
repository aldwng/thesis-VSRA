#pragma once
#ifndef VBR_TABLE_H
#define VBR_TABLE_H

#include "VBR_basic.h"

class Table
{
private:
	int voter_num;
	int candidate_num;
	bool faked = false;
	string tactic;

public:
	int fake_num;
	vector<vector<int> > raw_table;
	vector<vector<int> > fake_table;

	bool revised;
	vector<vector<int> > revise_table;

	Table(int N, int M)
	{
		voter_num = N;
		candidate_num = M;
		srand((unsigned)time(NULL));
		raw_table.resize(N);
		fake_table.resize(N);
		for (int i = 0; i < N; i++)
		{
			raw_table[i].resize(M);
			fake_table[i].resize(M);
			int base = rand() % (N * 20); // 20 is a factor number that expands the range of the number 'base'
			base = max(base, 5); // in order to avoid the situation, where 'base' is 0
			for (int j = 0; j < M; j++)
			{
				int score = rand() % base;
				raw_table[i][j] = score;
				fake_table[i][j] = score;
			}
		}
		revised = false;
	}

	~Table() { ; }

	void set_tactic(string &CHOICE)
	{
		tactic = CHOICE;
		return;
	}

	void falsify_table(int NUM)
	{
		if (faked)
		{
			cout << "VBR fake_table has been falsified" << endl;
			return;
		}
		fake_num = min(NUM, voter_num);
		faked = true;
		if (tactic == "")
		{
			cout << "VBR how to falsify the table has not been decided, use setTactic() to decide" << endl;
		}
		else
		{
			if (tactic == "irrational")
			{
				tactic_irrational();
			}
			else if (tactic == "mischief")
			{
				tactic_mischief();
			}
			else if (tactic == "compromising")
			{
				tactic_compromising();
			}
			else if (tactic == "burying")
			{
				tactic_burying();
			}
			else if (tactic == "group")
			{
				tactic_group();
			}
			else if (tactic == "groups")
			{
				tactic_groups();
			}
			else
			{
				cout << "VBR ERROR Table.falsify_table(): no such operation" << endl;
			}
		}
		return;
	}

	double evaluate_winner(int WINNER)
	{
		vector<int> max_ballot(voter_num, -1);
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				max_ballot[i] = max(max_ballot[i], raw_table[i][j]);
			}
		}
		double satisfaction = 0.0;
		for (int i = 0; i < voter_num; i++)
		{
			satisfaction += (double)raw_table[i][WINNER] / (double)max_ballot[i];
		}
		return (satisfaction / voter_num);
	}

	void recover_table()
	{
		faked = false;
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				fake_table[i][j] = raw_table[i][j];
			}
		}
	}

	void modify_table()
	{
		revise_table.resize(voter_num, vector<int>(candidate_num, 0));
		revised = true;
		vector<int> avrg(candidate_num, 0);
		vector<int> max_score(voter_num, INT_MIN);
		vector<int> min_score(voter_num, INT_MAX);
		int max_points = INT_MIN;
		int min_points = INT_MAX;
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				max_score[i] = max(max_score[i], fake_table[i][j]);
				min_score[i] = min(min_score[i], fake_table[i][j]);
				max_points = max(max_points, fake_table[i][j]);
				min_points = min(min_points, fake_table[i][j]);
				avrg[j] += fake_table[i][j];
			}
		}
		for (int i = 0; i < candidate_num; i++)
		{
			avrg[i] = avrg[i] / voter_num;
		}
		int range_base = max_points - min_points;
		for (int i = 0; i < voter_num; i++)
		{
			double depth = ((double)(max_score[i] - min_score[i])) / ((double)MODIFY_FACTOR * range_base);
			for (int j = 0; j < candidate_num; j++)
			{
				revise_table[i][j] = fake_table[i][j] - (int)(depth * (double)(fake_table[i][j] - avrg[j]));
			}
		}
	}

protected:
	void tactic_irrational()
	{
		for (int i = 0; i < fake_num; i++)
		{
			int max_tmp = -1;
			for (int j = 0; j < candidate_num; j++)
			{
				max_tmp = max(max_tmp, raw_table[i][j]);
			}
			for (int j = 0; j < candidate_num; j++)
			{
				if (raw_table[i][j] < max_tmp)
				{
					fake_table[i][j] = raw_table[i][j] * DECREASEFACTOR;
				}
				else
				{
					fake_table[i][j] = raw_table[i][j] * INCREASEFACTOR;
				}
			}
		}
	}

	void tactic_mischief() //rarely changes the raw_table, which means every candidate can satisfy the voter equally
	{
		for (int i = 0; i < fake_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				raw_table[i][j] = CONSTBALLOT;
			}
		}
		// in this scenario, mischief voting behavior is giving the fake information about voters' preference by casting various ballot for every candidate
		// actually, voter who will play mischief has the same value of satisfaction for every candidate
	}

	void tactic_compromising()
	{
		for (int i = 0; i < fake_num; i++)
		{
			vector<pair<int, int> > tmp_ballots;
			for (int j = 0; j < candidate_num; j++)
			{
				pair<int, int> tmp_pair(raw_table[i][j], j);
				tmp_ballots.push_back(tmp_pair);
			}
			sort(tmp_ballots.begin(), tmp_ballots.end(), [](pair<int, int> &a, pair<int, int> &b)
			{
				if (a.first == b.first) return a.second < b.second;
				else return a.first > b.first;
			});
			for (int j = 1; j < candidate_num && j <= COMPROMISING_NUM; j++)
			{
				// the number of candidates' ballot compromised should be probably further discussed
				fake_table[i][tmp_ballots[j].second] = raw_table[i][tmp_ballots[0].second];
				// cast ballots as many as favorite candidate's
			}
		}
	}

	void tactic_burying()
	{
		for (int i = 0; i < fake_num; i++)
		{
			vector<pair<int, int> > tmp_ballots;
			for (int j = 0; j < candidate_num; j++)
			{
				pair<int, int> tmp_pair(raw_table[i][j], j);
				tmp_ballots.push_back(tmp_pair);
			}
			sort(tmp_ballots.begin(), tmp_ballots.end(), [](pair<int, int> &a, pair<int, int> &b)
			{
				if (a.first == b.first) return a.second < b.second;
				else return a.first > b.first;
			});
			for (int j = 1; j < candidate_num && j <= BURYING_NUM; j++)
			{
				// the number of candidates' ballot buried should be probably further discussed
				fake_table[i][tmp_ballots[j].second] = raw_table[i][tmp_ballots[candidate_num - 1].second];
				// cast ballots as many as most-hated candidate's
			}
		}
	}

	void tactic_groups()
	{
		int group_size = voter_num / fake_num;
		int n = fake_num;
		int start = 0;
		while (n)
		{
			set_alliance(start, start + group_size);
			start += group_size;
			n--;
			if (n == 1)
			{
				set_alliance(start, voter_num);
				n--;
				break;
			}
		}
	}

	void tactic_group()
	{
		set_alliance(0, fake_num);
	}

	void set_alliance(int LEFT, int RIGHT)
	{
		if (LEFT >= RIGHT) return;
		int random_voter = LEFT + rand() % (RIGHT - LEFT);
		int tmp_candidate = -1;
		int tmp_score = INT_MIN;
		for (int i = 0; i < candidate_num; i++)
		{
			if (raw_table[random_voter][i] >= tmp_score)
			{
				tmp_candidate = i;
				tmp_score = raw_table[random_voter][i];
			}
		}
		vector<int> max_score((RIGHT - LEFT), INT_MIN);
		for (int i = LEFT; i < RIGHT; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				max_score[i - LEFT] = max(max_score[i - LEFT], raw_table[i][j]);
				fake_table[i][j] = raw_table[random_voter][j];
			}
			//fake_table[i][tmp_candidate] = max_score[i] + 1;
			raw_table[i][tmp_candidate] = max_score[i - LEFT];
		}
	}
};

#endif