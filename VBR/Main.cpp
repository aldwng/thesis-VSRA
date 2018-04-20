#include "VBR_operate.h"


void vbr_main()
{
	cout << "VBR the process is starting" << "\n";
	VBR vbr(MAXVOTER, MAXCANDIDATE, ITERATION);
	vbr.vbr_routine_fixed();
	cout << "VBR the process is finished!" << endl;
}

void vsr_main()
{
	VSR vsr(MAXVOTER, MAXCANDIDATE, ITERATION, METHODS);
	vsr.vsr_routine();
	cout << "VBR vsr_main() ends!" << endl;
}

int main() {
	vbr_main();
	getchar();
	return 0;
}
