#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/error.h>
#include <inc/string.h>
// LAB 6: Your driver code here

volatile uint32_t* reg;

static struct tx_desc tx_ring[N_TX_DESC] __attribute__((aligned(16)));
static char tx_buf[N_TX_DESC][4096] __attribute__((aligned(4096)));

int e1000_init(struct pci_func *f) {
	int i;
	pci_func_enable(f);

	reg = mmio_map_region(f->reg_base[0] & ~0xf, f->reg_size[0]);

	cprintf("%08x\n", reg[E1000_STATUS / 4]);

	assert(((uint32_t)tx_ring & 0x0000000f) == 0);

	reg[E1000_TDBAL / 4] = PADDR(tx_ring);
	reg[E1000_TDBAH / 4] = 0;
	reg[E1000_TDLEN / 4] = TX_RING_SIZE;
	reg[E1000_TDH / 4] = 0;
	reg[E1000_TDT / 4] = 0;
	reg[E1000_TCTL / 4] = E1000_TCTL_EN | E1000_TCTL_PSP | 0x10 << 4 | 0x40 << 12;
	// reg[E1000_TIPG / 4] = 0xa | 0x8 << 10 | 0xc << 20;
	reg[E1000_TIPG / 4] = 0xa | 0x4 << 10 | 0x6 << 20;

	for (i = 0; i < N_TX_DESC; i++) {
		tx_ring[i].addr = PADDR(&tx_buf[i]);
		tx_ring[i].cmd = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
		tx_ring[i].status |= E1000_TXD_STAT_DD;
	}

	return 0;
}

int e1000_transmit(void *va, size_t len) {
	int t;

	if (len > BUF_MAX_SIZE)
		return -E_INVAL;

	t = reg[E1000_TDT / 4];

	if (!(tx_ring[t].status & E1000_TXD_STAT_DD)) {
		cprintf("e1000 transmit ring is full\n");
		return -E_NO_MEM;
	}

	memcpy(&tx_buf[t], va, len);
	tx_ring[t].length = len;
	tx_ring[t].status &= ~E1000_TXD_STAT_DD;

	reg[E1000_TDT / 4] = (t + 1) % N_TX_DESC;

	return 0;
}
