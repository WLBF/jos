#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile uint8_t* reg;

int e1000_init(struct pci_func *f) {
	pci_func_enable(f);

	reg = mmio_map_region(f->reg_base[0] & ~0xf, f->reg_size[0]);

	cprintf("%08x\n", *(uint32_t *)(reg + E1000_STATUS));

	return 0;
}
