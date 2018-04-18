/*
* VRR: voting algorithm on routing resources
*/

#include "VRR_operate.h"

int main()
{
	srand((unsigned)time(NULL));

	string graph_address = "ATT.txt"; // experiment topology
	string req_address = "requests.txt"; // flow reguests
	string result_address = "results.txt"; // statistics

	vector<vector<double> > datL;
	vector<double> datL_avg;
	vector<vector<double> > haL;
	vector<double> haL_avg;
	vector<vector<double> > latL;
	vector<double> latL_avg;
	vector<vector<double> > teL;
	vector<double> teL_avg;

	ofstream result_out(result_address.c_str());

	for (int votes = 1; votes <= 1; votes++)
	{
		datL.clear();
		datL_avg.clear();
		haL.clear();
		haL_avg.clear();
		latL.clear();
		latL_avg.clear();
		teL.clear();
		teL_avg.clear();

		datL.resize(TESTNUM);
		datL_avg.resize(MAXREQ);
		haL.resize(TESTNUM);
		haL_avg.resize(MAXREQ);
		latL.resize(TESTNUM);
		latL_avg.resize(MAXREQ);
		teL.resize(TESTNUM);
		teL_avg.resize(MAXREQ);
		
		for (int i = 0; i < TESTNUM; i++)
		{
			cout << "VRR Main() test round: " << i << "\n";
			proceed_voting(graph_address, req_address, datL[i], haL[i], latL[i], teL[i]);
			for (int j = 0; j < MAXREQ; j++)
			{
				datL_avg[j] += datL[i][j] / TESTNUM;
				haL_avg[j] += haL[i][j] / TESTNUM;
				latL_avg[j] += latL[i][j] / TESTNUM;
				teL_avg[j] += teL[i][j] / TESTNUM;
			}
		}

		double ha_sum = 0.0;
		double dat_sum = 0.0;
		double lat_sum = 0.0;
		double te_sum = 0.0;
		for (int i = 0; i < MAXREQ; i++)
		{
			ha_sum += haL_avg[i];
			dat_sum += datL_avg[i];
			lat_sum += latL_avg[i];
			te_sum += teL_avg[i];
			result_out << haL_avg[i] << " " << datL_avg[i] << " " << latL_avg[i] << " " << teL_avg[i] << "\n";
		}
		result_out<< "Happiness: " << ha_sum / MAXREQ << " Maxbandwidth: " << dat_sum / MAXREQ << " Latency: " << lat_sum / MAXREQ << " Mlu: " << te_sum / MAXREQ << endl;
	}
	cout << "VRR Main(): test finished." << endl;
	getchar();
	return 0;
}
