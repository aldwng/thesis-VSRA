#include "VRR_qos.h"
#include "VRR_data.h"
#include "VRR_mlu.h"

void generate_traffic(string ADDRESS, Graph* GRAPH) // generate request .txt file content
{
	ofstream out_file(ADDRESS.c_str());
	out_file << MAXREQ << endl;
	for (int i = 0; i < MAXREQ; i++)
	{
		int app_id = rand() % (APPNUM);
		int from = rand() % 5;
		int to = rand() % 5 + 15;
		while (from == to)
		{
			to = rand() % GRAPH->n;
		}
		double rate = 1.0 + MINFLOW + rand() % MAXFLOW; // (MINFLOW, MAXFLOW]
		out_file << i << " " << app_id << " " << from << " " << to << " " << rate << endl;
	}
}

void proceed_voting(string GRAPH_ADDRESS, string REQ_ADDRESS, 
	vector<double>& DAT_LIST, vector<double>& HA_LIST, vector<double>& LAT_LIST, vector<double>& TE_LIST)
{   // voting process
	double happiness_sum_all = 0.0; // happiness sum, in order to calculate the average happiness of each round
	//double s2_sum_all = 0.00; // variance sum

	srand((unsigned)time(NULL));

	Graph* graph = new Graph(GRAPH_ADDRESS); // input the topo graph

	generate_traffic(REQ_ADDRESS, graph);

	//************************  initialization  ************************

	vector<vector<double> > eval_table;
	vector<int> weights(DECNUM, 1); // the voting weight of each voter
	eval_table.resize(DECNUM); 
	for (int i = 0; i < DECNUM; i++)
	{
		eval_table[i].resize(DECNUM);
	}

	for (int i = APPNUM; i < DECNUM; i++)
	{
		weights[i] = MLUVOTES;
	}

	vector<Voter*> voters; // store all voters
	for (int i = 0; i < QOSNUM; i++)
	{
		Voter* voter = new Qos(i, graph);
		voters.push_back(voter);
	}
	for (int i = QOSNUM; i < QOSNUM + DATANUM; i++)
	{
		Voter* voter = new Data(i, graph);
		voters.push_back(voter);
		weights[i] = DATAVOTES;
	}
	for (int i = APPNUM; i < DECNUM; i++)
	{
		Voter* voter = new Mlu(i, graph);
		voters.push_back(voter);
		weights[i] = MLUVOTES;
	}

	for (int i = 0; i < DECNUM; i++)
	{
		voters[i]->init();
	}
	cout << "VRR proceed_voting(): initialization complete" << "\n";

	// initialize background flow in the network
	for (int i = 0; i < APPNUM; i++)
	{
		for (int j = 0; j < graph->m; j++)
		{
			double flow_rate = 1.0 + (rand() % 2) * (rand() % BGFLOW); // random size of flows
			int from = graph->incL[j]->src;
			int to = graph->incL[j]->dst;
			voters[i]->adj_load[from][to] += flow_rate;
			graph->load[from][to] += flow_rate;
			if (graph->load[from][to] >= graph->adjM[from][to]->cap)
			{
				graph->load[from][to] = graph->adjM[from][to]->cap - 1.0;
			}
			if (voters[i]->adj_load[from][to] >= graph->adjM[from][to]->cap)
			{
				voters[i]->adj_load[from][to] = graph->adjM[from][to]->cap - 1.0;
			}
		}
	}

	ifstream req_file(REQ_ADDRESS.c_str()); // input request file
	int req_count = 0;
	req_file >> req_count;
	vector<Request*> requests; // store all requests
	requests.clear();
	for(int i = 0; i < req_count; i++)
	{
		int id = 0, app_id = 0, req_src = 0, req_dst = 0;
		double req_rate = 0.00;
		req_file >> id >> app_id >> req_src >> req_dst >> req_rate;
		Request* req = new Request(id, app_id, req_src, req_dst, req_rate);
		requests.push_back(req);
	}
	cout << "VRR proceed_voting(): input requests complete" << "\n";

	// for each request, proceed voting algorithm
	ofstream log_out("logs.txt");
	for (int i = 0; i < req_count; i++)
	{
		for (int j = 0; j < DECNUM; j++)
		{
			voters[j]->init();
		}
		log_out << "type: " << requests[i]->type << " src: " << requests[i]->src << " dst: " << requests[i]->dst << endl;
		
		// network flows disapper
		/*
		int dsp_id = rand() % APPNUM;
		int dsp_rate = rand() % LOSTFLOW;
		for (int k = 0; k < M; p++)
		{
			int from = graph->incL[k]->src;
			int to = graph->incL[k]->dst;
			if (voters[dsp_id]->adj_load[from][to] > dsp_rate)
			{
				voters[dsp_id]->adj_load[from][to] -= (dsp_rate - 1.00);
				graph->load[from][to] -= dsp_rate;
			}
		}
		*/

		// voting algorithm
		// each voter generates proposal for a flow request
		for (int j = 0; j < DECNUM; j++)
		{
			voters[j]->propose(requests[i]);
			for (int k = 0; k < voters[j]->path_record.size(); k++)
			{
				log_out << voters[j]->path_record[k] << " ";
			}
			log_out << endl;
		}

		// each voter evaluates candidates
		for (int j = 0; j < DECNUM; j++)
		{
			voters[j]->evaluate(requests[i], voters);
		}

		// voting method
		for (int j = 0; j < DECNUM; j++)
		{
			for (int k = 0; k < DECNUM; k++)
			{
				//voter j candidate k
				if (voters[j]->evaluate_list[k] == 0)
				{
					eval_table[j][k] = RINF;
				}
				else
				{
					eval_table[j][k] = voters[j]->evaluate_list[k];
				}
			}
		}
		int winner = 0;

		string voting_method = "RangeVoting";

		Voting* vote = new Voting(eval_table, weights, DECNUM, DECNUM);
		winner = vote->voting(voting_method);
        cout << "VRR proceed_voting(): voting process complete and winner selected" << "\n";
		delete vote;
		
		//winner = requests[i]->type; // the winner set as the flow's owner
		//winner = QOSNUM + DATANUM; // the winner set as ISP

		// used for get winner of QoS applications
		/*
		vector<vector<double> > qos_table (QOSNUM, vector<double>(QOSNUM, 0.0));
		for (int j = 0; j < QOSNUM; j++)
		{
			for (int k = 0; k < QOSNUM; k++)
			{
				qos_table[j][k] = eval_table[j][k];
			}
		}
		Voting* vote_qos = new Voting(qos_table, weights, QOSNUM, QOSNUM);
		if (requests[i]->type < QOSNUM)
		{
			winner = reqL[i]->TYPE;
		}
		else
		{
			winner = vote_qos->voting(voting_method);
		}
		delete vote_qos;
		*/

		// used for get winner of Data applications
		/*
		vector<vector<double> > data_table (DATANUM, vector<double>(DATANUM, 0.0));
		for (int j = QOSNUM; j < QOSNUM + DATANUM; j++)
		{
			for (int k = QOSNUM; k < QOSNUM + DATANUM; k++)
			{
				data_table[j - QOSNUM][k - QOSNUM] = eval_table[j][k];
			}
		}
		Voting* vote_data = new Voting(data_table, weights, DATANUM, DATANUM);
		if (requests[i]->type < QOSNUM + DATANUM && requests[i]->type >= QOSNUM)
		{
			winner = requests[i]->type;
		}
		else
		{
			winner = vote_data->voting(voting_method) + QOSNUM;
		}
		delete vote_data;
		*/

		cout << "VRR proceed_voting(): the winner is: " << winner << "\n";
		cout << "VRR process_voting(): request type: " << requests[i]->type << "\n";

		// reinforcement mechanism
		// adjustment step is 1
		
		// RM1, first simple way to reinforcement learning.
		for (int j = 0; j < DECNUM; j++) 
		{
			if (j == winner) 
			{
				voters[j]->alpha += 2;
				voters[j]->accepted += 1;
				if (voters[j]->alpha > RMFACTOR) voters[j]->alpha = RMFACTOR;
			}
			else 
			{
				voters[j]->alpha -= 2;
				voters[j]->aborted += 1;
				if (voters[j]->alpha < RMFACTOR / 10) voters[j]->alpha = RMFACTOR / 2;
			}
		}
		
		// RM2, second way to reinforcement learning based on the history.
		/*
		for (int j = 0; j < DECNUM; j++)
		{
			if (j == winner)
			{
				voters[j]->accepted += 1;
				voters[j]->alpha += voters[j]->accepted;
				voters[j]->aborted = 0;
				if (voters[j]->alpha > RMFACTOR) voters[j]->alpha = RMFACTOR;
			}
			else
			{
				voters[j]->aborted += 1;
				voters[j]->alpha -= decL[j]->aborted;
				voters[j]->accepted = 0;
				if (voters[j]->alpha < RMFACTOR / 100) voters[j]->alpha = RMFACTOR / 2;
			}
		}
		*/

		// reinforcement mechanism of adjusting weight
		// RMW1
		/*
		if (winner != DECNUM - 1)
		{
			weights[DECNUM - 1] += 1;
		}
		else
		{
			weights[DECNUM - 1] -= 2;
			if (weights[DECNUM - 1] < 1) weights[DECNUM - 1] = 1;
		}
		*/

		//RMW2
		/*
		if (winner != DECNUM - 1)
		{
			voters[DECNUM - 1]->accepted = 0;
			voters[DECNUM - 1]->aborted += 1;
			if (voters[DECNUM - 1]->aborted > (DECNUM - 1) / 2)
			{
				weights[DECNUM - 1] += 1;
				voters[DECNUM - 1]->aborted = 0;
			}
		}
		else
		{
			voters[DECNUM - 1]->accepted += 1;
			voters[DECNUM - 1]->aborted = 0;
			if (voters[DECNUM - 1]->accepted = 1) weights[DECNUM - 1] -= 1;
			if (voters[DECNUM - 1]->accepted > 1) weights[DECNUM - 1] -= voters[DECNUM - 1]->accepted;
		}
		if (weights[DECNUM - 1] < 1) weights[DECNUM - 1] = 1;
		*/

		// calculate happinuess
		double happiness_sum = 0.0;
		double happiness_avg = 0.0;
		vector<double> min_value(DECNUM, 9999.9);
		for (int j = 0; j < DECNUM; j++)
		{
			for (int k = 0; k < DECNUM; k++)
			{
				min_value[j] = min(min_value[j], eval_table[j][k]);
			}
		}
		for (int j = 0; j < DECNUM; j++)
		{
			happiness_sum += min_value[j] / eval_table[j][winner]; // the smaller value is better
		}
        happiness_avg = happiness_sum / DECNUM;
		happiness_sum_all += happiness_sum;

		HA_LIST.push_back(happiness_avg);

		// calculate happiness variance
		/*
		double s2_sum = 0.0;
		double s2_avg = 0.0;
		for (int j = 0; j < APPNUM; j++)
		{
			s2_sum += (happiness_avg - min_value[j] / eval_table[j][winner]) * (happiness_avg - min_value[j] / eval_table[j][winner]);
		}
		s2_avg = s2_sum / APPNUM;
		s2_sum_all += s2_sum;
		*/

		double dat = INF;
		double lat = 0.0;

		// calculate latency and path bandwidth
		for (int j = 0; j < voters[winner]->path_record.size() - 1; j++)
		{
			int tail = voters[winner]->path_record[j];
			int head = voters[winner]->path_record[j+ 1];
			if (graph->adjM[tail][head]->cap <= graph->load[tail][head]) dat = 1.0;
			else 
			{
				dat = (graph->adjM[tail][head]->cap - graph->load[tail][head]) > dat ? dat : (graph->adjM[tail][head]->cap - graph->load[tail][head]);
			}

			if (graph->load[tail][head] + requests[i]->rate < graph->adjM[tail][head]->cap)
			{
				graph->load[tail][head] += requests[i]->rate;
				voters[requests[i]->type]->adj_load[tail][head] += requests[i]->rate;
				lat += (requests[i]->rate / (graph->adjM[tail][head]->cap - graph->load[tail][head] - requests[i]->rate)) > 1 ? 1 : 
					(requests[i]->rate / (graph->adjM[tail][head]->cap - graph->load[tail][head] - requests[i]->rate));
			}
			else 
			{
				voters[requests[i]->type]->adj_load[tail][head] += graph->adjM[tail][head]->cap - graph->load[tail][head] - 1.0;
				graph->load[tail][head] = graph->adjM[tail][head]->cap - 1.0;
				if (voters[requests[i]->type]->adj_load[tail][head] + 1.0 >= graph->adjM[tail][head]->cap)
				{
					voters[requests[i]->type]->adj_load[tail][head] = graph->adjM[tail][head]->cap - 1.0;
				}
				lat += 1;
			}
		}
		if (lat > 1.7) lat = 1.7;
		if (lat <= 0) lat = 1.7;
		DAT_LIST.push_back(dat);
		LAT_LIST.push_back(lat);

		// calculate mlu
		double te = 0.0;
		for (int j = 0; j < graph->m; j++)
		{
			int from = graph->incL[j]->src;
			int to = graph->incL[j]->dst;
			double capacity = graph->incL[j]->cap;
			te = (graph->load[from][to] / capacity) > te ? (graph->load[from][to] / capacity) : te;
			if (te > 1.0) te = 1.0;
		}
		TE_LIST.push_back(te);
	}

	cout << "VRR procee_voting(): happiness = " << happiness_sum_all / MAXREQ / DECNUM << "\n";
	delete graph;
	for (int i = 0; i < DECNUM; i++)
	{
		delete voters[i];
	}
	for (int i = 0; i < MAXREQ; i++)
	{
		delete requests[i];
	}
	cout << "VRR process_voting(): voting process complete" << "\n";
}

