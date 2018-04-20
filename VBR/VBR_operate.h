/*
* VBR: voting behavior research
* created in 30/10/2017
*/
#pragma once

#include "VBR_table.h"
#include "VBR_method.h"

//static string METHOD = "cumulative";
static string TACTIC = "groups";
static string OUTPUT = "vbr_out.txt";
static string VSR_OUTPUT = "vsr_out.txt";

class VBR
{
private:
	Method* vbr_method_origin;
	Method* vbr_method_fake;
	Method* vbr_method_revise; // used for revise method, which will decrease the influence of bad voting behaviors
	Table* vbr_table;
	int vbr_max_voter;
	int vbr_max_candidate;
	int vbr_fake_voter;
	int vbr_iteration;
	int vbr_winner_origin;
	int vbr_winner_fake;
	int vbr_winner_revise;

public:
	VBR(int MV, int MC, int ITER)
	{
		vbr_max_voter = MV;
		vbr_max_candidate = MC;
		vbr_iteration = ITER;
		vbr_method_origin = NULL;
		vbr_method_fake = NULL;
		vbr_method_revise = NULL;
		vbr_table = NULL;
		types.resize(METHODS);
		init_method();
	}

	~VBR()
	{
		if (vbr_method_origin != NULL)
		{
			delete vbr_method_origin;
			vbr_method_origin = NULL;
		}
		if (vbr_method_fake != NULL)
		{
			delete vbr_method_fake;
			vbr_method_fake = NULL;
		}
		
		if (vbr_method_revise != NULL)
		{
			delete vbr_method_revise;
			vbr_method_revise = NULL;
		}
		
		if (vbr_table != NULL)
		{
			delete vbr_table;
			vbr_table = NULL;
		}
	}

	void vbr_routine_fixed()
	{
		srand((unsigned)time(NULL));
		ofstream output(OUTPUT);
		int tmp_voter = rand() % vbr_max_voter;
		int tmp_candidate = rand() % vbr_max_candidate;
		tmp_voter = tmp_voter < 5 ? 5 : tmp_voter;
		tmp_candidate = tmp_candidate < 5 ? 5 : tmp_candidate;

		tmp_voter = vbr_max_voter;
		tmp_candidate = vbr_max_candidate;

		output << "the number of voters: " << tmp_voter << "\n";
		output << "the number of candidates: " << tmp_candidate << "\n";
		vbr_fake_voter = tmp_voter * FAKEVOTER;
		vbr_satisfaction_origin.resize(METHODS, vector<double>(vbr_fake_voter, 0.0));
		vbr_satisfaction_fake.resize(METHODS, vector<double>(vbr_fake_voter, 0.0));
		vbr_satisfaction_revise.resize(METHODS, vector<double>(vbr_fake_voter, 0.0));
		for (int i = 0; i < vbr_iteration; i++)
		{
			vbr_fake_voter = tmp_voter * FAKEVOTER;
			for (int j = 0; j < vbr_fake_voter; j++)
			{
				//cout << "voter number: " << tmp_voter << endl;
				vbr_table = new Table(tmp_voter, tmp_candidate);
				vbr_table->set_tactic(TACTIC);
				vbr_table->falsify_table(j + 1);
				vbr_table->modify_table();
				vbr_method_origin = new Method(tmp_voter, tmp_candidate, vbr_table->raw_table);
				vbr_method_fake = new Method(tmp_voter, tmp_candidate, vbr_table->fake_table);
				//if (vbr_table->revised) vbr_method_revise = new Method(tmp_voter, tmp_candidate, vbr_table->revise_table);
				for (int k = 0; k < METHODS; k++)
				{
					vbr_method_origin->set_method(types[k]);
					vbr_method_fake->set_method(types[k]);
					//vbr_method_revise->set_method(types[k]);
					vbr_winner_origin = vbr_method_origin->get_winner();
					vbr_winner_fake = vbr_method_fake->get_winner();
					//vbr_winner_revise = vbr_method_revise->get_winner();
					vbr_winner_revise = vbr_method_fake->draft_winner(PROMOTE_ROUNDS);
					//vbr_winner_revise = vbr_method_fake->get_new_winner();
					vbr_method_fake->recover_weights();
					vbr_measure(tmp_voter, tmp_candidate, j, k);
				}
				delete vbr_method_origin;
				delete vbr_method_fake;
				//delete vbr_method_revise;
				delete vbr_table;
				vbr_method_origin = NULL;
				vbr_method_fake = NULL;
				//vbr_method_revise = NULL;
				vbr_table = NULL;
			}
		}
		vbr_print_measure(output);
		return;
	}

	void vbr_routine_flexible()
	{
		srand((unsigned)time(NULL));
		ofstream output(OUTPUT);
		for (int i = 0; i < vbr_iteration; i++)
		{
			int tmp_voter = rand() % vbr_max_voter;
			int tmp_candidate = rand() % vbr_max_candidate;
			tmp_voter = tmp_voter < 5 ? 5 : tmp_voter;
			tmp_candidate = tmp_candidate < 5 ? 5 : tmp_candidate;
			vbr_fake_voter = tmp_voter * FAKEVOTER;
			cout << "VBR voter number: " << tmp_voter << " candidate number: " << tmp_candidate << endl;
			for (int j = 0; j < vbr_fake_voter; j++)
			{
				//cout << "voter number: " << tmp_voter << endl;
				vbr_table = new Table(tmp_voter, tmp_candidate);
				vbr_table->set_tactic(TACTIC);
				vbr_table->falsify_table(j + 1);
				vbr_method_origin = new Method(tmp_voter, tmp_candidate, vbr_table->raw_table);
				vbr_method_fake = new Method(tmp_voter, tmp_candidate, vbr_table->fake_table);
				//vbr_method_origin->set_method(METHOD);
				//vbr_method_fake->set_method(METHOD);
				vbr_winner_origin = vbr_method_origin->get_winner();
				vbr_winner_fake = vbr_method_fake->get_winner();
				//vbr_measure(tmp_voter, tmp_candidate, j, );
				delete vbr_method_origin;
				delete vbr_method_fake;
				delete vbr_table;
				vbr_method_origin = NULL;
				vbr_method_fake = NULL;
				vbr_table = NULL;
			}
		}
		vbr_print_measure(output);
		return;
	}

protected:
	vector<vector<double> > vbr_satisfaction_origin; // the length of this vector relies on the number of negative-effect voters
	vector<vector<double> > vbr_satisfaction_fake;
	vector<vector<double> > vbr_satisfaction_revise;
	vector<string> types;

	void init_method()
	{
		types[0] = "borda";
		types[1] = "condorcet";
		types[2] = "cumulative";
		types[3] = "copeland";
		types[4] = "schulze";
		types[5] = "range";
	}

	void vbr_measure(int NUMVOTER, int NUMCANDI, int INDEX, int TYPE)
	{
		cout << "VBR origin winner: " << vbr_winner_origin << endl;
		cout << "VBR fake winner: " << vbr_winner_fake << endl;
		cout << "VBR revise winner: " << vbr_winner_revise << endl;
		double meter_origin = 0.0;
		double meter_fake = 0.0;
		double meter_revise = 0.0;
		vector<int> max_ballot(NUMVOTER, -1);
		for (int i = 0; i < NUMVOTER; i++)
		{
			for (int j = 0; j < NUMCANDI; j++)
			{
				max_ballot[i] = max(max_ballot[i], vbr_table->raw_table[i][j]);
			}
		}
		for (int i = 0; i < NUMVOTER; i++)
		{
			meter_origin += (double)vbr_table->raw_table[i][vbr_winner_origin] / (double)max_ballot[i];
			meter_fake += (double)vbr_table->raw_table[i][vbr_winner_fake] / (double)max_ballot[i];
			meter_revise += (double)vbr_table->raw_table[i][vbr_winner_revise] / (double)max_ballot[i];
		}
		vbr_satisfaction_origin[TYPE][INDEX] += meter_origin / NUMVOTER;
		vbr_satisfaction_fake[TYPE][INDEX] += meter_fake / NUMVOTER;
		vbr_satisfaction_revise[TYPE][INDEX] += meter_revise / NUMVOTER;
		return;
	}

	void vbr_print_measure(ofstream &OUT)
	{
		OUT << "tactic: " << TACTIC << "\n";
		OUT << "iteration-average satisfaction for each number of negative-effect voters:" << "\n";
		for (int i = 0; i < METHODS; i++)
		{
			OUT << types[i] << ":" << "\n";
			for (int j = 0; j < vbr_fake_voter; j++)
			{
				OUT << vbr_satisfaction_origin[i][j] / vbr_iteration << " " << vbr_satisfaction_fake[i][j] / vbr_iteration << " " << vbr_satisfaction_revise[i][j] / vbr_iteration;
				OUT << "\n";
			}
			OUT << "\n" << endl;
		}
		OUT << endl;
	}
};

// voting method simple research
class VSR
{
private:
	Method* method;
	Table* table;
	int iterations;
	int max_voter;
	int max_candidate;
	int method_types;
	vector<int> winners;
	vector<double> satisfaction;
	vector<string> types;

	void init_method()
	{
		types[0] = "borda";
		types[1] = "condorcet";
		types[2] = "cumulative";
		types[3] = "copeland";
		types[4] = "schulze";
		types[5] = "range";
	}

	void measure_winner(int VOTER, int CANDIDATE, int INDEX)
	{
		double meter = 0.0;
		vector<int> max_ballot(VOTER, -1);
		for (int i = 0; i < VOTER; i++)
		{
			for (int j = 0; j < CANDIDATE; j++)
			{
				max_ballot[i] = max(max_ballot[i], table->raw_table[i][j]);
			}
		}
		for (int i = 0; i < VOTER; i++)
		{
			meter += (double)table->raw_table[i][winners[INDEX]] / (double)max_ballot[i];
		}
		satisfaction[INDEX] += meter / (double)VOTER;
	}

public:
	VSR(int VOTER, int CANDIDATE, int ITERATION, int METHODS)
	{
		max_voter = VOTER;
		max_candidate = CANDIDATE;
		iterations = ITERATION;
		method = new Method("range");
		winners.resize(METHODS, -1);
		method_types = METHODS;
		types.resize(METHODS);
		satisfaction.resize(METHODS, 0.0);
	}

	~VSR()
	{
		if (table != NULL)
		{
			delete table;
			table = NULL;
		}
		if (method != NULL)
		{
			delete method;
			method = NULL;
		}
	}

	void vsr_routine()
	{
		init_method();
		ofstream output(VSR_OUTPUT);
		output << "iterations: " << iterations << endl;
		output << "max voters: " << max_voter << " max candidates: " << max_candidate << endl;
		for (int i = 0; i < iterations; i++)
		{
			for (int j = 2; j < max_voter; j++)
			{
				for (int k = 2; k < max_candidate; k++)
				{
					table = new Table(j + 1, k + 1);
					method->set_table(j + 1, k + 1, table->raw_table);
					for (int p = 0; p < method_types; p++)
					{
						method->set_method(types[p]);
						winners[p] = method->get_winner();
						if (winners[p] < 0) return;
						cout << "winner is: " << winners[p] << " ";
						measure_winner(j + 1, k + 1, p);
					}
					delete table;
					table = NULL;
				}
				cout << "\n";
			}
		}
		cout << "VBR VSR the routine is finished" << endl;
		for (int i = 0; i < method_types; i++)
		{
			output << types[i] << ": " << satisfaction[i] / (iterations * (max_voter - 2) * (max_candidate - 2)) << endl;
		}
	}
};
