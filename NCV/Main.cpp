#include "NCV_runner.h"

void ncv_main()
{
	Run* runner = new Run();
	runner->run_main();
	cout << "NCR ncv_main(): algorithm test complete!" << endl;
}

int main()
{
	ncv_main();
	getchar();
	return 0;
}
