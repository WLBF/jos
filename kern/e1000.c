#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/error.h>
#include <inc/string.h>
// LAB 6: Your driver code here

volatile uint32_t* reg;

static struct tx_desc tx_ring[N_TX_DESC] __attribute__((aligned(16)));
static char tx_buf[N_TX_DESC][TX_BUFSIZE] __attribute__((aligned(4096)));

static struct rx_desc rx_ring[N_RX_DESC] __attribute__((aligned(16)));
static char rx_buf[N_RX_DESC][RX_BUFSIZE] __attribute__((aligned(4096)));

int e1000_init(struct pci_func *f) {
	int i;
	pci_func_enable(f);

	reg = mmio_map_region(f->reg_base[0] & ~0xf, f->reg_size[0]);

	cprintf("%08x\n", reg[E1000_STATUS / 4]);

	assert(((uint32_t)tx_ring & 0x0000000f) == 0);
	assert(((uint32_t)rx_ring & 0x0000000f) == 0);

	for (i = 0; i < N_TX_DESC; i++) {
		tx_ring[i].addr = PADDR(&tx_buf[i]);
		tx_ring[i].cmd = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
		tx_ring[i].status |= E1000_TXD_STAT_DD;
	}

	for (i = 0; i < N_RX_DESC; i++) {
		rx_ring[i].addr = PADDR(&rx_buf[i]);
	}

	reg[E1000_TDBAL / 4] = PADDR(tx_ring);
	reg[E1000_TDBAH / 4] = 0;
	reg[E1000_TDLEN / 4] = TX_RING_SIZE;
	reg[E1000_TDH / 4] = 0;
	reg[E1000_TDT / 4] = 0;
	reg[E1000_TCTL / 4] = E1000_TCTL_EN | E1000_TCTL_PSP | 0x10 << 4 | 0x40 << 12;
	reg[E1000_TIPG / 4] = 0xa | 0x4 << 10 | 0x6 << 20;


	reg[E1000_RAL / 4] = 0x12005452;
	reg[E1000_RAH / 4] = 0x5634 | E1000_RAH_AV;
	memset((void *)&reg[E1000_MTA / 4], 0, 4 * 128);
	reg[E1000_RDBAL / 4] = PADDR(rx_ring);
	reg[E1000_RDBAH / 4] = 0;
	reg[E1000_RDLEN / 4] = RX_RING_SIZE;
	reg[E1000_RDH / 4] = 0;
	reg[E1000_RDT / 4] = N_RX_DESC - 1;
	reg[E1000_RCTL / 4] = E1000_RCTL_EN | E1000_RCTL_LBM_NO | E1000_RCTL_SZ_2048 | E1000_RCTL_SECRC | E1000_RCTL_BAM;
	return 0;
}

int e1000_transmit(void *va, size_t len) {
	int t;
	t = reg[E1000_TDT / 4];

	if (len > TX_BUFSIZE)
		len = TX_BUFSIZE;

	if (!(tx_ring[t].status & E1000_TXD_STAT_DD))
		return -E_NO_MEM;

	memcpy(&tx_buf[t], va, len);
	tx_ring[t].length = len;
	tx_ring[t].status &= ~E1000_TXD_STAT_DD;

	reg[E1000_TDT / 4] = (t + 1) % N_TX_DESC;

	return 0;
}

int e1000_receive(void *va, size_t len) {
	int t;
	t = (reg[E1000_RDT / 4] + 1) % N_RX_DESC;

	if (!(rx_ring[t].status & E1000_RXD_STAT_DD))
		return -E_NO_MEM;

	if (rx_ring[t].length > len)
		return -E_INVAL;

	memcpy(va, &rx_buf[t], rx_ring[t].length);
	rx_ring[t].status &= ~E1000_RXD_STAT_DD;

	reg[E1000_RDT / 4] = t;

	return (int)rx_ring[t].length;
}
