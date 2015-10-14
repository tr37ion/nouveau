/*
 * Copyright 2015 Martin Peres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Martin Peres
 */
#include <subdev/bios.h>
#include <subdev/bios/bit.h>
#include <subdev/bios/iccsense.h>

static u16
nvbios_iccsense_table(struct nvkm_bios *bios, u8 *ver, u8 *hdr, u8 *cnt,
		      u8 *len)
{
	struct bit_entry bit_P;
	u16 iccsense = 0x0000;

	if (!bit_entry(bios, 'P', &bit_P)) {
		if (bit_P.version == 2 && bit_P.length >= 0x2c)
		iccsense = nvbios_rd16(bios, bit_P.offset + 0x28);

		if (iccsense) {
			*ver = nvbios_rd08(bios, iccsense + 0);
			switch (*ver) {
			case 0x10:
			case 0x20:
				*hdr = nvbios_rd08(bios, iccsense + 1);
				*len = nvbios_rd08(bios, iccsense + 2);
				*cnt = nvbios_rd08(bios, iccsense + 3);
				return iccsense;
			default:
				break;
			}
		}
	}

	return 0x0000;
}

int
nvbios_iccsense_parse(struct nvkm_bios *bios, struct nvbios_iccsense *iccsense)
{
	u8 ver, hdr, cnt, len, i;
	u16 table, entry;

	table = nvbios_iccsense_table(bios, &ver, &hdr, &cnt, &len);
	if (!table)
		return -EINVAL;

	if (cnt > 3) {
		nvkm_info(&bios->subdev,
			  "iccsense: Got more entries than expected (%d)\n",
			  cnt);
		cnt = 3;
	}

	iccsense->nr_entry = 0;
	i = 0;
	while (i < cnt) {
		entry = table + hdr + i * len;

		switch(ver) {
		case 0x10:
			iccsense->rail[iccsense->nr_entry].extdev_id =
						nvbios_rd08(bios, entry + 0x2);
			iccsense->rail[iccsense->nr_entry].resistor_mohm =
						nvbios_rd08(bios, entry + 0x3);
			break;
		case 0x20:
			iccsense->rail[iccsense->nr_entry].extdev_id =
						nvbios_rd08(bios, entry + 0x1);
			iccsense->rail[iccsense->nr_entry].resistor_mohm =
						nvbios_rd08(bios, entry + 0x5);
			break;
		};

		/* only store entries where the resistor is > 0 */
		if (iccsense->rail[iccsense->nr_entry].resistor_mohm > 0)
			iccsense->nr_entry++;

		i++;
	}

	return 0;
}
