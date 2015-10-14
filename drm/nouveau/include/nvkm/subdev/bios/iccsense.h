#ifndef __NVBIOS_ICCSENSE_H__
#define __NVBIOS_ICCSENSE_H__
struct pwr_rail_t {
	int extdev_id;
	int resistor_mohm;
};

struct nvbios_iccsense {
	int nr_entry;
	struct pwr_rail_t rail[32];
};
#endif
