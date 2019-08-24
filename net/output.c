#include "ns.h"
#include "inc/syscall.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the dejvice driver
	uint32_t to, whom;

	while (1) {

		to = ipc_recv((int32_t *) &whom, &nsipcbuf, NULL);

		if (whom != ns_envid || to != NSREQ_OUTPUT) {
			cprintf("NS OUTPUT: output thread got IPC message %d from env %x \n", whom, to);
			continue;
		}

		while(sys_ns_send_packet(0, nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len) < 0)
			sys_yield();
	}
}
