#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#endif  // SOL >= 6

#include<kern/pci.h>


#define N_TX_DESC      0x40
#define TX_DESC_SIZE   0x10
#define TX_RING_SIZE   N_TX_DESC * TX_DESC_SIZE
#define BUF_MAX_SIZE   PGSIZE - 8

#define E1000_VENDOR_ID         0x8086
#define E1000_DEV_ID_82540EM    0x100E
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */

/* Transmit Control */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x00000001 /* End of Packet */
#define E1000_TXD_CMD_RS     0x00000008 /* Report Status */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */

struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
} __attribute__ ((packed));

int e1000_init(struct pci_func *f);
int e1000_transmit(void *addr, size_t len);
