/* file : srv_test.cpp
 * test app of cpp
 *
 * 2009-4-30 23:55
 */

#include "ndapplib/applib.h"
#include "ndapplib/httpParser.h"
#include "httpInstant.h"

#include <vector>

HttpInstant g_instance_srv ;
//DataCache g_dcache ;

HttpInstant &get_instance()
{
	return g_instance_srv;
}


int main(int argc, const char *argv[])
{
	HttpInstant &inst = g_instance_srv;
	if (argc <= 1) {
		if (-1 == inst.CreateEx(5, argv[0], "-f", "../cfg/nfhttp_cfg.xml", "-c", "program_setting")) {
			ndfprintf(stderr, "create instant, config file error\n");
			exit(1);
		}
		if (inst.Open(sizeof(NDHttpSession))) {
			ndfprintf(stderr, "open listener error\n");
			exit(1);
		}
	}
	else {
		if (-1 == inst.Start(argc, argv)) {
			ndfprintf(stderr, "Start Function error!\n");
			exit(1);
		}
	}
	

	ND_TRACE_FUNC() ;
		
	inst.End(inst.WaitServer());

	return 0;
}

