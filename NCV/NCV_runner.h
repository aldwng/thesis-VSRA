#pragma once

#include "NCV_voter.h"

class Run
{
public:
	Run()
	{
		init_objects();
		entity_num = entities.size();
		cout << "NCV Run.Run(): constructing run object completed" << "\n";
	}

	void run_main()
	{
		srand((unsigned)time(NULL));
		ofstream log(log_file);
		deployed_num.resize(TEST_ROUND, 0);
		deployed_rate.resize(TEST_ROUND, 0.0);
		deployed_vmu.resize(TEST_ROUND, 0.0);
		deployed_lc.resize(TEST_ROUND, 0.0);
		deployed_num_two.resize(TEST_ROUND, vector<int>(MAX_TRAFFIC_NUM / UNIT_TRAFFIC_NUM, 0));
		deployed_rate_two.resize(TEST_ROUND, vector<double>(MAX_TRAFFIC_NUM / UNIT_TRAFFIC_NUM, 0.0));
		cout << "NCV Run.run_main(): core operations are starting" << "\n";
		for (int i = 0; i < TEST_ROUND; i++)
		{

			int sum_req = 0;
			double sum_rate = 0.0;
			double sum_vmu = 0.0; // virtual machine ultilization
			double sum_lc = 0.0; // link consumption

			for (int j = 0; j < MAX_TRAFFIC_NUM; j++)
			{
				Traffic *req = req_generate(j);
				cout << "NCV Run.run_main(): a new traffic request generated" << "\n";
				log_request(req, log);
				cout << "NCV Run.run_main(): the traffic request recorded in the log file" << "\n";
				run_propose(req);
				cout << "NCV Run.run_main():  collecting proposals finished" << "\n";

				// voting process
				//run_evaluate(req);
				//cout << "NCV Run.run_main(): collecting evaluations finished" << "\n";
				//Counter* counter = new Counter(eval_table);
				//counter->set_choice("range");
				//counter->set_range(METHOD_RANGE);
				//int final = counter->winner_out();
				// voting process complete

				//int final = 3; // winner will always be GVR.
				int final = 0; // winner will always be GSP.
								//int final = (j % 2) == 0 ? 0 : 3; // winner will selected by round-robin
				//cout << "NCV Run.run_main(): voting process finished" << "\n";
				cout << "NCV the winner is: " << final << "\n";

				// test the ability of continuously serving requests 
				/*
				if (schemes[final] == NULL)
				{
				cout << "Run:(run_main()) the number of requests deployed: " << j + 1 << "\n";
				break;
				}
				*/
				// test the ability of serving requests
				if (schemes[final] != NULL)
				{
					sum_req += 1;
					sum_rate += req->rate;
				}

				deployed_num_two[i][j / UNIT_TRAFFIC_NUM] = sum_req;
				deployed_rate_two[i][j / UNIT_TRAFFIC_NUM] = sum_rate;

				log_scheme(schemes[final], log);
				cout << "NCV Run.run_main(): scheme logged" << "\n";

				run_deploy(req, schemes[final]);
				cout << "NCV Run.run_main(): service chain deployed" << "\n";
				run_statistics(req, schemes[final], i);
				run_vmu(req, schemes[final], i);
				run_lc(req, schemes[final], i);
				cout << "NCV Run.run_main(): statistics recorded" << "\n";
				//delete counter;
				delete req;
			}
			delete graph;
			delete graph_algorithm;
			graph = new SimpleFatTree(SIMPLEFATTREE_LEVEL, HOST_VOL, LINK_BASE_CAPACITY, VM_RCS);
			graph_algorithm = new GraphAlgorithm(graph);
			/*
			for (int i = 0; i < entities.size(); i++)
			{
				entities[i]->graph = graph;
				entities[i]->graph_algorithm = graph_algorithm;
			}
			*/
			// when test JVP, shuold use below to renew the JVP object
			delete entities[0];
			entities[0] = new JVP(graph, graph_algorithm);
		}
		int deployed_num_sum = 0;
		double deployed_rate_sum = 0.0;
		double deployed_vmu_sum = 0.0;
		double deployed_lc_sum = 0.0;
		for (int i = 0; i < TEST_ROUND; i++)
		{
			cout << "round: " << i + 1 << " num: " << deployed_num[i] << " rate: " << deployed_rate[i] << "\n";
			deployed_num_sum += deployed_num[i];
			deployed_rate_sum += deployed_rate[i];
			deployed_vmu_sum += deployed_vmu[i] / (double)deployed_num[i];
			deployed_lc_sum += deployed_lc[i] / (double)deployed_num[i];
		}
		cout << "avg num: " << ((double)deployed_num_sum / (double)TEST_ROUND) << " avg rate: " << deployed_rate_sum / (double)TEST_ROUND << "\n";
		cout << "avg vmu: " << (deployed_vmu_sum / (double)TEST_ROUND) << " avg lc: " << deployed_lc_sum / (double)TEST_ROUND << "\n";

		for (int i = 0; i < (MAX_TRAFFIC_NUM / UNIT_TRAFFIC_NUM); i++)
		{
			int out_num = 0;
			double out_rate = 0.0;
			for (int j = 0; j < TEST_ROUND; j++)
			{
				out_num += deployed_num_two[j][i];
				out_rate += deployed_rate_two[j][i];
			}
			cout << i << " " << out_num / TEST_ROUND << " " << out_rate / TEST_ROUND << "\n";
		}

	}

	~Run()
	{
		if (graph != NULL)
		{
			delete graph;
			graph = NULL;
		}
		for (int i = 0; i < entities.size(); i++)
		{
			if (entities[i] != NULL)
			{
				delete entities[i];
				entities[i] = NULL;
			}
		}
	}

private:
	Topo* graph;
	GraphAlgorithm* graph_algorithm;
	vector<Entity*> entities;

	vector<Scheme*> schemes;
	int entity_num;
	vector<vector<double> > eval_table;

	// for the purpose of getting statistics
	vector<int> deployed_num;
	vector<double> deployed_rate;
	vector<double> deployed_vmu;
	vector<double> deployed_lc;

	vector<vector<int> > deployed_num_two;
	vector<vector<double> > deployed_rate_two;

	void init_objects()
	{
		graph = new SimpleFatTree(SIMPLEFATTREE_LEVEL, HOST_VOL, LINK_BASE_CAPACITY, VM_RCS);
		graph_algorithm = new GraphAlgorithm(graph);
		Entity* tmp_ent = new JVP(graph, graph_algorithm);
		entities.push_back(tmp_ent);

		// init voters in voting algorithm
		//tmp_ent = new CSP(graph, 2, graph_algorithm);
		//entities.push_back(tmp_ent);
		//tmp_ent = new CSP(graph, 1, graph_algorithm);
		//entities.push_back(tmp_ent);
		//tmp_ent = new CSP(graph, 3, graph_algorithm);
		//entities.push_back(tmp_ent);
		//tmp_ent = new HLB(graph, graph_algorithm);
		//entities.push_back(tmp_ent);
		//tmp_ent = new GVR(graph, graph_algorithm);
		//entities.push_back(tmp_ent);
	}

	Traffic* req_generate(int ID)
	{
		Traffic* req_ans = NULL;
		double rate_tmp = (double)(rand() % (MAX_TRAFFIC_RATE - MIN_TRAFFIC_RATE)) + (double)MIN_TRAFFIC_RATE;
		int src_tmp = rand() % graph->host_num;
		src_tmp = graph->host_list[src_tmp]->id;
		int dst_tmp = src_tmp;
		while (dst_tmp == src_tmp)
		{
			dst_tmp = rand() % graph->host_num;
			dst_tmp = graph->host_list[dst_tmp]->id;
		}
		int length_tmp = rand() % (MAX_CHAIN_LENGTH - MIN_CHAIN_LENGTH) + MIN_CHAIN_LENGTH;
		req_ans = new Traffic(ID, src_tmp, dst_tmp, rate_tmp, SERVICE_TYPE, length_tmp);
		return req_ans;
	}

	void run_propose(Traffic* REQ)
	{
		schemes.clear();
		cout << "NCV Run.run_propose(): collecting proposals is starting" << "\n";
		for (int i = 0; i < entity_num; i++)
		{
			cout << "NCV Run.run_propose(): entity " << i + 1 << " starts to propose" << "\n";
			Scheme* scheme_tmp = entities[i]->proposal_generate(REQ);
			schemes.push_back(scheme_tmp);
			cout << "NCV Run.run_propose(): entity " << i + 1 << " finishes proposing" << "\n";
		}
	}

	void run_evaluate(Traffic* REQ)
	{
		cout << "NCV Run.run_evaluate(): collecting evaluations is starting" << "\n";
		for (int i = 0; i < entity_num; i++)
		{
			cout << "NCV Run.run_evaluate(): entity " << i + 1 << " starts to evaluate" << "\n";
			for (int j = 0; j < schemes.size(); j++)
			{
				cout << i << " " << j << "\n";
				eval_table[i][j] = entities[i]->evaluate(REQ, schemes[j]);
			}
			cout << "NCV Run.run_evaluate(): entity " << i + 1 << " finishes evaluating" << "\n";
		}
	}

	bool run_deploy(Traffic* REQ, Scheme* SCHEME)
	{
		if (SCHEME == NULL) return false;
		int scheme_size = SCHEME->service_chain.size();
		for (int i = 0; i < scheme_size; i++)
		{
			SCHEME->service_chain[i].first->host->vm_deploy(REQ->function_chain[i], REQ->rate,
				SCHEME->service_chain[i].second);
		}
		cout << "NCV Run.run_deploy(): vm deployed" << "\n";
		// here is to update graph information.
		int hosts_size = SCHEME->host_chain.size();
		for (int i = 0; i <= hosts_size; i++)
		{
			if (i == 0)
			{
				if (SCHEME->host_chain[i]->id != REQ->src)
				{
					Node* flag = SCHEME->host_chain[i];
					while (flag->id != REQ->src)
					{
						Node* tmp = graph_algorithm->parents[REQ->src][flag->id];
						graph->load_matrix[tmp->id][flag->id] += REQ->rate;
						graph->overall_bandwidth -= REQ->rate;
						flag = tmp;
					}
				}
			}
			else if (i == hosts_size)
			{
				if (SCHEME->host_chain[i - 1]->id != REQ->dst)
				{
					int src_tmp = SCHEME->host_chain[i - 1]->id;
					Node* flag = graph->node_map[REQ->dst];
					while (flag->id != src_tmp)
					{
						Node* tmp = graph_algorithm->parents[src_tmp][flag->id];
						graph->load_matrix[tmp->id][flag->id] += REQ->rate;
						graph->overall_bandwidth -= REQ->rate;
						flag = tmp;
					}
				}
			}
			else
			{
				Node* src_tmp = SCHEME->host_chain[i - 1];
				Node* flag = SCHEME->host_chain[i];
				while (flag != NULL && src_tmp != NULL && flag->id != src_tmp->id)
				{
					Node* tmp = graph_algorithm->parents[src_tmp->id][flag->id];
					graph->load_matrix[tmp->id][flag->id] += REQ->rate;
					graph->overall_bandwidth -= REQ->rate;
					flag = tmp;
				}
			}
		}
		cout << "NCV Run.run_deploy(): link deployed" << "\n";
		return true;
	}

	void run_statistics(Traffic* REQ, Scheme* SCHEME, int INDEX)
	{
		if (SCHEME == NULL) return;
		deployed_num[INDEX] += 1;
		deployed_rate[INDEX] += REQ->rate;
	}

	void run_vmu(Traffic* REQ, Scheme* SCHEME, int INDEX)
	{
		// virtual machine utilization statistics, average of each virtual machine
		if (SCHEME == NULL) return;
		double vmu = 0.0;
		int chain_len = SCHEME->service_chain.size();
		for (int i = 0; i < chain_len; i++)
		{
			vmu += (SCHEME->service_chain[i].second->rcs - SCHEME->service_chain[i].second->rcs_free) / SCHEME->service_chain[i].second->rcs;
		}
		vmu = vmu / (double)chain_len;
		deployed_vmu[INDEX] += vmu;
	}

	void run_lc(Traffic* REQ, Scheme* SCHEME, int INDEX)
	{
		// link consumption utilization statistics
		if (SCHEME == NULL) return;
		double lc = 0.0;
		int hosts_size = SCHEME->host_chain.size();
		for (int i = 0; i <= hosts_size; i++)
		{
			if (i == 0)
			{
				if (SCHEME->host_chain[i]->id != REQ->src)
				{
					Node* flag = SCHEME->host_chain[i];
					while (flag->id != REQ->src)
					{
						Node* tmp = graph_algorithm->parents[REQ->src][flag->id];
						lc += (graph->adj_matrix[tmp->id][flag->id]->cap / LINK_BASE_CAPACITY) * REQ->rate;
						flag = tmp;
					}
				}
			}
			else if (i == hosts_size)
			{
				if (SCHEME->host_chain[i - 1]->id != REQ->dst)
				{
					int src_tmp = SCHEME->host_chain[i - 1]->id;
					Node* flag = graph->node_map[REQ->dst];
					while (flag->id != src_tmp)
					{
						Node* tmp = graph_algorithm->parents[src_tmp][flag->id];
						lc += (graph->adj_matrix[tmp->id][flag->id]->cap / LINK_BASE_CAPACITY) * REQ->rate;
						flag = tmp;
					}
				}
			}
			else
			{
				Node* src_tmp = SCHEME->host_chain[i - 1];
				Node* flag = SCHEME->host_chain[i];
				while (flag != NULL && src_tmp != NULL && flag->id != src_tmp->id)
				{
					Node* tmp = graph_algorithm->parents[src_tmp->id][flag->id];
					lc += (graph->adj_matrix[tmp->id][flag->id]->cap / LINK_BASE_CAPACITY) * REQ->rate;
					flag = tmp;
				}
			}
		}
		deployed_lc[INDEX] += lc;
	}

	void log_request(Traffic* REQ, ofstream& LOG)
	{
		LOG << "ID: " << REQ->id << "\n" << "SRC: " << REQ->src << " DST: " << REQ->dst << " RATE: " << REQ->rate << "\n";
		for (auto function : REQ->function_chain)
		{
			LOG << function << " ";
		}
		LOG << endl;
	}

	void log_scheme(Scheme* SCHEME, ofstream& LOG)
	{
		if (SCHEME == NULL) return;
		LOG << "deploy scheme:" << endl;
		int function_size = SCHEME->service_chain.size();
		for (auto service : SCHEME->service_chain)
		{
			LOG << "Host:" << service.first->id << " VM:" << service.second->id << " ";
		}
		LOG << endl;
	}
};
