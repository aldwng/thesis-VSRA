#pragma once

#ifndef VBR_METHOD_H
#define VBR_METHOD_H

#include "VBR_basic.h"

/*
bool compare_pair(pair<double, int> a, pair<double, int> b)
{
	if (a.first == b.first) return a.second < b.second;
	else return a.first > b.first;
}
*/

class Method
{
private:
	int voter_num;
	int candidate_num;
	vector<vector<double> > eval_table;
	string choice;

public:
	Method() {}

	Method(int N, int M, vector<vector<int> > &TABLE)
	{
		voter_num = N;
		candidate_num = M;
		eval_table.resize(N);
		for (int i = 0; i < N; i++)
		{
			eval_table[i].resize(M);
			for (int j = 0; j < M; j++)
			{
				eval_table[i][j] = (double)TABLE[i][j];
			}
		}
		weights.resize(candidate_num, 1.0);
	}

	Method(string CHOICE)
	{
		choice = CHOICE;
	}

	~Method() { ; }

	void set_method(string METHOD)
	{
		choice = METHOD;
		return;
	}

	string get_method()
	{
		return choice;
	}

	void set_table(int N, int M, vector<vector<int> > &TABLE)
	{
		voter_num = N;
		candidate_num = M;
		eval_table.clear();
		eval_table.resize(N, vector<double>(M, 0.0));
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < M; j++)
			{
				eval_table[i][j] = (double)TABLE[i][j];
			}
		}
		weights.resize(candidate_num, 1.0);
	}

	int get_winner()
	{
		int ans = -1;
		if (voter_num == 0 || candidate_num == 0) return ans;
		//weights.resize(candidate_num, 1.0);
		if (choice == "")
		{
			cout << "VBR ERROR Method.get_winner(): method choice has not been set" << endl;
			return ans;
		}
		else
		{
			if (choice == "plain")
			{
				ans = method_plain();
			}
			else if (choice == "range")
			{
				ans = method_range();
			}
			else if (choice == "borda")
			{
				ans = method_borda();
			}
			else if (choice == "cumulative")
			{
				ans = method_cumulative();
			}
			else if (choice == "condorcet")
			{
				ans = method_condorcet();
			}
			else if (choice == "copeland")
			{
				ans = method_copeland();
			}
			else if (choice == "schulze")
			{
				ans = method_schulze();
			}
			else
			{
				cout << "VBR ERROR Method.get_winner(): no such method" << endl;
			}
		}
		return ans;
	}

	int get_new_winner()
	{
		measure_similarity();
		int ans = get_winner();
		return ans;
	}

	void recover_weights()
	{
		weights.clear();
		weights.resize(candidate_num, 1.0);
	}

	int draft_winner(int ROUNDS)
	{
		int ans = -1;
		if (voter_num == 0 || candidate_num == 0) return ans;
		if (choice == "")
		{
			cout << "VBR ERROR Method.draft_winner(): method choice has not been set" << endl;
			return ans;
		}
		else
		{
			promote_candidates.clear();
			for (int i = 0; i < candidate_num; i++)
			{
				promote_candidates.push_back(i);
			}
			if (choice == "plain")
			{
				ans = method_plain();
			}
			else if (choice == "range")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_range(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else if (choice == "borda")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_borda(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else if (choice == "cumulative")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_cumulative(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else if (choice == "condorcet")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_condorcet(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else if (choice == "copeland")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_copeland(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else if (choice == "schulze")
			{
				for (int i = 0; i < PROMOTE_ROUNDS; i++)
				{
					draft_schulze(promote_candidates.size() * PROMOTE_FACTOR);
				}
				ans = promote_candidates[0];
			}
			else
			{
				cout << "VBR ERROR Method.draft_winner(): no such method" << endl;
			}
		}
		return ans;
	}

protected:
	vector<double> weights;

	int method_plain()
	{
		vector<double> score_sum(candidate_num, 0.0);
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				score_sum[j] += eval_table[i][j];
			}
		}
		int ans = 0;
		double max_score = 0.0;
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_sum[i] > max_score)
			{
				ans = i;
				max_score = score_sum[i];
			}
		}
		return ans;
	}

	int method_range()
	{
		cout << "VBR method: range" << endl;
		vector<double> score_sum(candidate_num, 0.0);
		double score_max = -1.0;
		int ans = -1;
		for (int i = 0; i < voter_num; i++)
		{
			double max_score = 0.0;
			for (int j = 0; j < candidate_num; j++)
			{
				max_score = max(max_score, eval_table[i][j]);
			}
			for (int j = 0; j < candidate_num; j++)
			{
				int temp = (int)((eval_table[i][j] / max_score) * RANGE);
				score_sum[j] += (double)temp * weights[i];
			}
		}
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_sum[i] > score_max)
			{
				score_max = score_sum[i];
				ans = i;
			}
		}
		return ans;
	}

	int method_borda()
	{
		cout << "VBR method: borda" << endl;
		vector<vector<int> > borda_table;
		vector<double> score_sum(candidate_num, 0.0);
		int ans = -1;
		double max_score = -1.0;
		borda_table.resize(voter_num);
		for (int i = 0; i < voter_num; i++)
		{
			borda_table[i].resize(candidate_num);
			vector<pair<double, int> > temp;
			for (int j = 0; j < candidate_num; j++)
			{
				pair<double, int> p(eval_table[i][j], j);
				temp.push_back(p);
			}
			sort(temp.begin(), temp.end(), [](pair<double, int> &a, pair<double, int> &b)
			{
				if (a.first == b.first) return a.second < b.second;
				else return a.first > b.first;
			});
			for (int j = 0; j < candidate_num; j++)
			{
				borda_table[i][temp[j].second] = candidate_num - j;
				score_sum[temp[j].second] += (double)(candidate_num - j) * weights[i];
			}
		}
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_sum[i] > max_score)
			{
				max_score = score_sum[i];
				ans = i;
			}
		}
		return ans;
	}

	int method_condorcet()
	{
		cout << "VBR method: condorcet" << endl;
		vector<double> wins_sum(candidate_num, 0.0);
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				int cnt = 0;
				for (int k = 0; k < candidate_num; k++)
				{
					if (eval_table[i][j] > eval_table[i][k]) cnt++;
				}
				wins_sum[j] += (double)cnt * weights[i];
			}
		}
		int ans = -1;
		double max_wins = -1.0;
		for (int i = 0; i < candidate_num; i++)
		{
			if (wins_sum[i] > max_wins)
			{
				max_wins = wins_sum[i];
				ans = i;
			}
		}
		return ans;
	}

	int method_cumulative()
	{
		cout << "VBR method: cumulative" << endl;
		vector<double> voter_sum(voter_num, 0.0);
		vector<double> score_sum(candidate_num, 0.0);
		for (int i = 0; i < voter_num; i++)
		{
			double temp = 0.0;
			for (int j = 0; j < candidate_num; j++)
			{
				temp += eval_table[i][j];
			}
			voter_sum[i] = temp;
		}
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				score_sum[j] += (eval_table[i][j] / voter_sum[i]) * weights[i];
			}
		}
		int ans = -1;
		double max_score = -1.0;
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_sum[i] > max_score)
			{
				max_score = score_sum[i];
				ans = i;
			}
		}
		return ans;
	}

	int method_copeland()
	{
		cout << "VBR method: copeland" << endl;
		vector<int> score_win(candidate_num, 0.0);
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				for (int k = 0; k < candidate_num; k++)
				{
					if (j != k)
					{
						score_win[j] += eval_table[i][j] >= eval_table[i][k] ? (int)weights[i] : -1 * (int)weights[i];
					}
				}
			}
		}
		int ans = -1;
		int max_score = INT_MIN;
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_win[i] > max_score)
			{
				max_score = score_win[i];
				ans = i;
			}
		}
		return ans;
	}

	int method_schulze()
	{
		cout << "VBR method: schulze" << endl;
		vector<int> score_win(candidate_num, 0);
		vector<vector<int> > schulze_table(candidate_num, vector<int>(candidate_num, 0));
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				for (int k = 0; k < candidate_num; k++)
				{
					if (j != k && eval_table[i][j] > eval_table[i][k])
					{
						schulze_table[j][k] += 1 * (int)weights[i];
					}
				}
			}
		}
		for (int k = 0; k < candidate_num; k++)
		{
			for (int i = 0; i < candidate_num; i++)
			{
				for (int j = 0; j < candidate_num; j++)
				{
					if (i != k && i != j && k != j && schulze_table[k][i] && schulze_table[i][k] && schulze_table[i][j])
					{
						schulze_table[i][j] = max(schulze_table[i][j], min(schulze_table[i][k], schulze_table[k][j]));
					}
				}
			}
		}
		for (int i = 0; i < candidate_num; i++)
		{
			for (int j = 0; j < candidate_num; j++)
			{
				if (schulze_table[i][j] > candidate_num / 2)
				{
					score_win[i] += 1;
				}
			}
		}
		int ans = -1;
		int max_score = INT_MIN;
		for (int i = 0; i < candidate_num; i++)
		{
			if (score_win[i] > max_score)
			{
				max_score = score_win[i];
				ans = i;
			}
		}
		return ans;
	}

	/*
	* ***********************
	* below is draft methods
	* ***********************
	*/
	vector<int> promote_candidates;

	int draft_range(int NUM)
	{
		cout << "VBR draft method: range" << endl;
		int tmp_num = promote_candidates.size();
		vector<pair<int, int> > score_sum(tmp_num, pair<int, int>(0, 0));
		int ans = -1;
		for (int i = 0; i < tmp_num; i++)
		{
			score_sum[i].second = promote_candidates[i];
		}
		for (int i = 0; i < voter_num; i++)
		{
			double max_score = 0.0;
			for (int j = 0; j < tmp_num; j++)
			{
				max_score = max(max_score, eval_table[i][promote_candidates[j]]);
			}
			for (int j = 0; j < tmp_num; j++)
			{
				int temp = (int)((eval_table[i][promote_candidates[j]] / max_score) * RANGE);
				score_sum[j].first += temp;
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<int, int> &pa, pair<int, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		promote_candidates.clear();
		NUM = max(1, NUM);
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	int draft_borda(int NUM)
	{
		cout << "VBR draft method: borda" << endl;
		int tmp_num = promote_candidates.size();
		vector<pair<int, int> > score_sum(tmp_num, pair<int, int>(0, 0));
		int ans = -1;
		for (int i = 0; i < tmp_num; i++)
		{
			score_sum[i].second = promote_candidates[i];
		}
		for (int i = 0; i < voter_num; i++)
		{
			vector<pair<double, int> > temp;
			for (int j = 0; j < tmp_num; j++)
			{
				pair<double, int> p(eval_table[i][promote_candidates[j]], j);
				temp.push_back(p);
			}
			sort(temp.begin(), temp.end(), [](pair<double, int> &a, pair<double, int> &b)
			{
				if (a.first == b.first) return a.second < b.second;
				else return a.first > b.first;
			});
			for (int j = 0; j < tmp_num; j++)
			{
				score_sum[temp[j].second].first += tmp_num - j;
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<int, int> &pa, pair<int, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		promote_candidates.clear();
		NUM = max(1, NUM);
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	int draft_condorcet(int NUM)
	{
		cout << "VBR draft method: condorcet" << endl;
		int tmp_num = promote_candidates.size();
		vector<pair<int, int> > score_sum(tmp_num, pair<int, int>(0, 0));
		int ans = -1;
		for (int i = 0; i < tmp_num; i++)
		{
			score_sum[i].second = promote_candidates[i];
		}
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < tmp_num; j++)
			{
				int cnt = 0;
				for (int k = 0; k < tmp_num; k++)
				{
					if (eval_table[i][promote_candidates[j]] > eval_table[i][promote_candidates[k]]) cnt++;
				}
				score_sum[j].first += cnt;
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<int, int> &pa, pair<int, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		promote_candidates.clear();
		NUM = max(1, NUM);
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	int draft_cumulative(int NUM)
	{
		cout << "VBR draft method: cumulative" << endl;
		int ans = -1;
		int tmp_num = promote_candidates.size();
		vector<double> voter_sum(voter_num, 0.0);
		vector<pair<double, int> > score_sum(tmp_num, pair<double, int>(0.0, 0));
		for (int i = 0; i < tmp_num; i++)
		{
			score_sum[i].second = promote_candidates[i];
		}
		for (int i = 0; i < voter_num; i++)
		{
			double temp = 0.0;
			for (int j = 0; j < tmp_num; j++)
			{
				temp += eval_table[i][promote_candidates[j]];
			}
			voter_sum[i] = temp;
		}
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < tmp_num; j++)
			{
				score_sum[j].first += eval_table[i][promote_candidates[j]] / voter_sum[i];
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<double, int> &pa, pair<double, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		NUM = max(1, NUM);
		promote_candidates.clear();
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	int draft_copeland(int NUM)
	{
		cout << "VBR draft method: copeland" << endl;
		int tmp_num = promote_candidates.size();
		vector<pair<int, int> > score_sum(tmp_num, pair<int, int>(0, 0));
		int ans = -1;
		for (int i = 0; i < tmp_num; i++)
		{
			score_sum[i].second = promote_candidates[i];
		}
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < tmp_num; j++)
			{
				for (int k = 0; k < tmp_num; k++)
				{
					if (j != k)
					{
						score_sum[j].first += eval_table[i][promote_candidates[j]] >= eval_table[i][promote_candidates[k]] ? 1 : -1;
					}
				}
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<int, int> &pa, pair<int, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		NUM = max(1, NUM);
		promote_candidates.clear();
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	int draft_schulze(int NUM)
	{
		cout << "VBR draft method: schulze" << endl;
		int tmp_num = promote_candidates.size();
		vector<pair<int, int> > score_sum(tmp_num, pair<int, int>(0, 0));
		int ans = -1;
		for (int i = 0; i < tmp_num; i++) {
			score_sum[i].second = promote_candidates[i];
		}
		vector<vector<int> > schulze_table(tmp_num, vector<int>(tmp_num, 0));
		for (int i = 0; i < voter_num; i++)
		{
			for (int j = 0; j < tmp_num; j++)
			{
				for (int k = 0; k < tmp_num; k++)
				{
					if (j != k && eval_table[i][promote_candidates[j]] > eval_table[i][promote_candidates[k]])
					{
						schulze_table[j][k] += 1;
					}
				}
			}
		}
		for (int k = 0; k < tmp_num; k++)
		{
			for (int i = 0; i < tmp_num; i++)
			{
				for (int j = 0; j < tmp_num; j++)
				{
					if (i != k && i != j && k != j && schulze_table[k][i] && schulze_table[i][k] && schulze_table[i][j])
					{
						schulze_table[i][j] = max(schulze_table[i][j], min(schulze_table[i][k], schulze_table[k][j]));
					}
				}
			}
		}
		for (int i = 0; i < tmp_num; i++)
		{
			for (int j = 0; j < tmp_num; j++)
			{
				if (schulze_table[i][j] > tmp_num / 2)
				{
					score_sum[i].first += 1;
				}
			}
		}
		sort(score_sum.begin(), score_sum.end(), [](pair<int, int> &pa, pair<int, int> &pb)
		{
			if (pa.first == pb.first) return pa.second > pb.second;
			else return pa.first > pb.first;
		});
		NUM = max(1, NUM);
		promote_candidates.clear();
		for (int i = 0; i < NUM; i++)
		{
			promote_candidates.push_back(score_sum[i].second);
		}
		ans = promote_candidates[0];
		return ans;
	}

	// use the order of preferences to calculate the similarity between two voters' ballots
	// chose not to use the certain value of voters' ballots because of difficulty, though which is a better way

	void measure_similarity()
	{
		vector<vector<int> > preference(voter_num, vector<int>(candidate_num, 0));
		for (int i = 0; i < voter_num; i++)
		{
			vector<pair<double, int> > tmp_rec;
			for (int j = 0; j < candidate_num; j++)
			{
				pair<double, int> tmp_pair(eval_table[i][j], j);
				tmp_rec.push_back(tmp_pair);
			}
			sort(tmp_rec.begin(), tmp_rec.end(), [](pair<double, int> &a, pair<double, int> &b)
			{
				if (a.first == b.first) return a.second > b.second;
				else return a.first > b.first;
			});
			for (int j = 0; j < candidate_num; j++)
			{
				preference[i][j] = tmp_rec[j].second;
			}
		}
		for (int i = 0; i < voter_num; i++)
		{
			double tmp_similarity = 1.0;
			for (int j = 0; j < voter_num && j != i; j++)
			{
				int cnt = 0;
				for (int k = 0; k < candidate_num; k++)
				{
					if (preference[i][k] == preference[j][k]) cnt++;
				}
				if (cnt == candidate_num) tmp_similarity += 1.0;
				//else if (cnt >= candidate_num / 2) tmp_similarity += 0.5;
			}
			weights[i] = ((double)candidate_num / tmp_similarity);
		}
	}
};

#endif
