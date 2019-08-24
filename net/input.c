#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int r, i;
	static struct jif_pkt *ppkt;

	if ((r = sys_page_alloc(0, ppkt, PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	while (1)
	{
		while((r = sys_ns_recv(ppkt->jp_data, 2048)) < 0)
			sys_yield();

		ppkt->jp_len = r;

		while((r = sys_ipc_try_send(ns_envid, NSREQ_INPUT, ppkt, PTE_P | PTE_U | PTE_W)) == -E_IPC_NOT_RECV)
			sys_yield();

		if (r < 0)
			cprintf("NS Input: sys_ipc_try_send error %e\n", r);

		for (i = 0; i < 10; i++)
			sys_yield();
	}
}
