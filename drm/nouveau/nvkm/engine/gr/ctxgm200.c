/*
 * Copyright 2015 Red Hat Inc.
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
 * Authors: Ben Skeggs <bskeggs@redhat.com>
 */
#include "ctxgf100.h"

/*******************************************************************************
 * PGRAPH context register lists
 ******************************************************************************/

static const struct gf100_gr_init
gm200_grctx_init_icmd_0[] = {
	{ 0x001000,   1, 0x01, 0x00000002 },
	{ 0x0006aa,   1, 0x01, 0x00000001 },
	{ 0x0006ad,   2, 0x01, 0x00000100 },
	{ 0x0006b1,   1, 0x01, 0x00000011 },
	{ 0x00078c,   1, 0x01, 0x00000008 },
	{ 0x000792,   1, 0x01, 0x00000001 },
	{ 0x000794,   3, 0x01, 0x00000001 },
	{ 0x000797,   1, 0x01, 0x000000cf },
	{ 0x00079a,   1, 0x01, 0x00000002 },
	{ 0x0007a1,   1, 0x01, 0x00000001 },
	{ 0x0007a3,   3, 0x01, 0x00000001 },
	{ 0x000831,   1, 0x01, 0x00000004 },
	{ 0x01e100,   1, 0x01, 0x00000001 },
	{ 0x001000,   1, 0x01, 0x00000008 },
	{ 0x000039,   3, 0x01, 0x00000000 },
	{ 0x000380,   1, 0x01, 0x00000001 },
	{ 0x000366,   2, 0x01, 0x00000000 },
	{ 0x000368,   1, 0x01, 0x00000fff },
	{ 0x000370,   2, 0x01, 0x00000000 },
	{ 0x000372,   1, 0x01, 0x000fffff },
	{ 0x000374,   1, 0x01, 0x00000100 },
	{ 0x000818,   8, 0x01, 0x00000000 },
	{ 0x000848,  16, 0x01, 0x00000000 },
	{ 0x000738,   1, 0x01, 0x00000000 },
	{ 0x000b07,   1, 0x01, 0x00000002 },
	{ 0x000b08,   2, 0x01, 0x00000100 },
	{ 0x000b0a,   1, 0x01, 0x00000001 },
	{ 0x000a04,   1, 0x01, 0x000000ff },
	{ 0x000a0b,   1, 0x01, 0x00000040 },
	{ 0x00097f,   1, 0x01, 0x00000100 },
	{ 0x000a02,   1, 0x01, 0x00000001 },
	{ 0x000809,   1, 0x01, 0x00000007 },
	{ 0x00c221,   1, 0x01, 0x00000040 },
	{ 0x00c401,   1, 0x01, 0x00000001 },
	{ 0x00c402,   1, 0x01, 0x00010001 },
	{ 0x00c403,   2, 0x01, 0x00000001 },
	{ 0x00c40e,   1, 0x01, 0x00000020 },
	{ 0x01e100,   1, 0x01, 0x00000001 },
	{ 0x001000,   1, 0x01, 0x00000001 },
	{ 0x000b07,   1, 0x01, 0x00000002 },
	{ 0x000b08,   2, 0x01, 0x00000100 },
	{ 0x000b0a,   1, 0x01, 0x00000001 },
	{ 0x01e100,   1, 0x01, 0x00000001 },
	{ 0x001000,   1, 0x01, 0x00000004 },
	{ 0x000039,   3, 0x01, 0x00000000 },
	{ 0x0000a9,   1, 0x01, 0x0000ffff },
	{ 0x000038,   1, 0x01, 0x0fac6881 },
	{ 0x00003d,   1, 0x01, 0x00000001 },
	{ 0x0000e8,   8, 0x01, 0x00000400 },
	{ 0x000078,   8, 0x01, 0x00000300 },
	{ 0x000050,   1, 0x01, 0x00000011 },
	{ 0x000058,   8, 0x01, 0x00000008 },
	{ 0x000208,   8, 0x01, 0x00000001 },
	{ 0x000081,   1, 0x01, 0x00000001 },
	{ 0x000085,   1, 0x01, 0x00000004 },
	{ 0x000088,   1, 0x01, 0x00000400 },
	{ 0x000090,   1, 0x01, 0x00000300 },
	{ 0x000098,   1, 0x01, 0x00001001 },
	{ 0x0000e3,   1, 0x01, 0x00000001 },
	{ 0x0000da,   1, 0x01, 0x00000001 },
	{ 0x0000b4,   4, 0x01, 0x88888888 },
	{ 0x0000f8,   1, 0x01, 0x00000003 },
	{ 0x0000fa,   1, 0x01, 0x00000001 },
	{ 0x0000b1,   2, 0x01, 0x00000001 },
	{ 0x00009f,   4, 0x01, 0x0000ffff },
	{ 0x0000a8,   1, 0x01, 0x0000ffff },
	{ 0x0000ad,   1, 0x01, 0x0000013e },
	{ 0x0000e1,   1, 0x01, 0x00000010 },
	{ 0x000290,  16, 0x01, 0x00000000 },
	{ 0x0003b0,  16, 0x01, 0x00000000 },
	{ 0x0002a0,  16, 0x01, 0x00000000 },
	{ 0x000420,  16, 0x01, 0x00000000 },
	{ 0x0002b0,  16, 0x01, 0x00000000 },
	{ 0x000430,  16, 0x01, 0x00000000 },
	{ 0x0002c0,  16, 0x01, 0x00000000 },
	{ 0x0004d0,  16, 0x01, 0x00000000 },
	{ 0x000720,  16, 0x01, 0x00000000 },
	{ 0x0008c0,  16, 0x01, 0x00000000 },
	{ 0x000890,  16, 0x01, 0x00000000 },
	{ 0x0008e0,  16, 0x01, 0x00000000 },
	{ 0x0008a0,  16, 0x01, 0x00000000 },
	{ 0x0008f0,  16, 0x01, 0x00000000 },
	{ 0x00094c,   1, 0x01, 0x000000ff },
	{ 0x00094d,   1, 0x01, 0xffffffff },
	{ 0x00094e,   1, 0x01, 0x00000002 },
	{ 0x0002f2,   2, 0x01, 0x00000001 },
	{ 0x0002f5,   1, 0x01, 0x00000001 },
	{ 0x0002f7,   1, 0x01, 0x00000001 },
	{ 0x000303,   1, 0x01, 0x00000001 },
	{ 0x0002e6,   1, 0x01, 0x00000001 },
	{ 0x000466,   1, 0x01, 0x00000052 },
	{ 0x000301,   1, 0x01, 0x3f800000 },
	{ 0x000304,   1, 0x01, 0x30201000 },
	{ 0x000305,   1, 0x01, 0x70605040 },
	{ 0x000306,   1, 0x01, 0xb8a89888 },
	{ 0x000307,   1, 0x01, 0xf8e8d8c8 },
	{ 0x00030a,   1, 0x01, 0x00ffff00 },
	{ 0x00030b,   1, 0x01, 0x0000001a },
	{ 0x00030c,   1, 0x01, 0x00000001 },
	{ 0x000318,   1, 0x01, 0x00000001 },
	{ 0x000340,   1, 0x01, 0x00000000 },
	{ 0x00037d,   1, 0x01, 0x00000006 },
	{ 0x0003a0,   1, 0x01, 0x00000002 },
	{ 0x0003aa,   1, 0x01, 0x00000001 },
	{ 0x0003a9,   1, 0x01, 0x00000001 },
	{ 0x000380,   1, 0x01, 0x00000001 },
	{ 0x000383,   1, 0x01, 0x00000011 },
	{ 0x000360,   1, 0x01, 0x00000040 },
	{ 0x000366,   2, 0x01, 0x00000000 },
	{ 0x000368,   1, 0x01, 0x00000fff },
	{ 0x000370,   2, 0x01, 0x00000000 },
	{ 0x000372,   1, 0x01, 0x000fffff },
	{ 0x000374,   1, 0x01, 0x00000100 },
	{ 0x00037a,   1, 0x01, 0x00000012 },
	{ 0x000619,   1, 0x01, 0x00000003 },
	{ 0x000811,   1, 0x01, 0x00000003 },
	{ 0x000812,   1, 0x01, 0x00000004 },
	{ 0x000813,   1, 0x01, 0x00000006 },
	{ 0x000814,   1, 0x01, 0x00000008 },
	{ 0x000815,   1, 0x01, 0x0000000b },
	{ 0x000800,   6, 0x01, 0x00000001 },
	{ 0x000632,   1, 0x01, 0x00000001 },
	{ 0x000633,   1, 0x01, 0x00000002 },
	{ 0x000634,   1, 0x01, 0x00000003 },
	{ 0x000635,   1, 0x01, 0x00000004 },
	{ 0x000654,   1, 0x01, 0x3f800000 },
	{ 0x000657,   1, 0x01, 0x3f800000 },
	{ 0x000655,   2, 0x01, 0x3f800000 },
	{ 0x0006cd,   1, 0x01, 0x3f800000 },
	{ 0x0007f5,   1, 0x01, 0x3f800000 },
	{ 0x0007dc,   1, 0x01, 0x39291909 },
	{ 0x0007dd,   1, 0x01, 0x79695949 },
	{ 0x0007de,   1, 0x01, 0xb9a99989 },
	{ 0x0007df,   1, 0x01, 0xf9e9d9c9 },
	{ 0x0007e8,   1, 0x01, 0x00003210 },
	{ 0x0007e9,   1, 0x01, 0x00007654 },
	{ 0x0007ea,   1, 0x01, 0x00000098 },
	{ 0x0007ec,   1, 0x01, 0x39291909 },
	{ 0x0007ed,   1, 0x01, 0x79695949 },
	{ 0x0007ee,   1, 0x01, 0xb9a99989 },
	{ 0x0007ef,   1, 0x01, 0xf9e9d9c9 },
	{ 0x0007f0,   1, 0x01, 0x00003210 },
	{ 0x0007f1,   1, 0x01, 0x00007654 },
	{ 0x0007f2,   1, 0x01, 0x00000098 },
	{ 0x0005a5,   1, 0x01, 0x00000001 },
	{ 0x0005aa,   1, 0x01, 0x00000002 },
	{ 0x0005cb,   1, 0x01, 0x00000004 },
	{ 0x0005d0,   1, 0x01, 0x20181008 },
	{ 0x0005d1,   1, 0x01, 0x40383028 },
	{ 0x0005d2,   1, 0x01, 0x60585048 },
	{ 0x0005d3,   1, 0x01, 0x80787068 },
	{ 0x000980, 128, 0x01, 0x00000000 },
	{ 0x000468,   1, 0x01, 0x00000004 },
	{ 0x00046c,   1, 0x01, 0x00000001 },
	{ 0x000470,  96, 0x01, 0x00000000 },
	{ 0x0005e0,  16, 0x01, 0x00000d10 },
	{ 0x000510,  16, 0x01, 0x3f800000 },
	{ 0x000520,   1, 0x01, 0x000002b6 },
	{ 0x000529,   1, 0x01, 0x00000001 },
	{ 0x000530,  16, 0x01, 0xffff0000 },
	{ 0x000550,  32, 0x01, 0xffff0000 },
	{ 0x000585,   1, 0x01, 0x0000003f },
	{ 0x000576,   1, 0x01, 0x00000003 },
	{ 0x00057b,   1, 0x01, 0x00000059 },
	{ 0x000586,   1, 0x01, 0x00000040 },
	{ 0x000582,   2, 0x01, 0x00000080 },
	{ 0x000595,   1, 0x01, 0x00400040 },
	{ 0x000596,   1, 0x01, 0x00000492 },
	{ 0x000597,   1, 0x01, 0x08080203 },
	{ 0x0005ad,   1, 0x01, 0x00000008 },
	{ 0x000598,   1, 0x01, 0x00020001 },
	{ 0x0005d4,   1, 0x01, 0x00000001 },
	{ 0x0005c2,   1, 0x01, 0x00000001 },
	{ 0x000638,   2, 0x01, 0x00000001 },
	{ 0x00063a,   1, 0x01, 0x00000002 },
	{ 0x00063b,   2, 0x01, 0x00000001 },
	{ 0x00063d,   1, 0x01, 0x00000002 },
	{ 0x00063e,   1, 0x01, 0x00000001 },
	{ 0x0008b8,   8, 0x01, 0x00000001 },
	{ 0x000900,   8, 0x01, 0x00000001 },
	{ 0x000908,   8, 0x01, 0x00000002 },
	{ 0x000910,  16, 0x01, 0x00000001 },
	{ 0x000920,   8, 0x01, 0x00000002 },
	{ 0x000928,   8, 0x01, 0x00000001 },
	{ 0x000662,   1, 0x01, 0x00000001 },
	{ 0x000648,   9, 0x01, 0x00000001 },
	{ 0x000674,   1, 0x01, 0x00000001 },
	{ 0x000658,   1, 0x01, 0x0000000f },
	{ 0x0007ff,   1, 0x01, 0x0000000a },
	{ 0x00066a,   1, 0x01, 0x40000000 },
	{ 0x00066b,   1, 0x01, 0x10000000 },
	{ 0x00066c,   2, 0x01, 0xffff0000 },
	{ 0x0007af,   2, 0x01, 0x00000008 },
	{ 0x0007f6,   1, 0x01, 0x00000001 },
	{ 0x0006b2,   1, 0x01, 0x00000055 },
	{ 0x0007ad,   1, 0x01, 0x00000003 },
	{ 0x000971,   1, 0x01, 0x00000008 },
	{ 0x000972,   1, 0x01, 0x00000040 },
	{ 0x000973,   1, 0x01, 0x0000012c },
	{ 0x00097c,   1, 0x01, 0x00000040 },
	{ 0x000975,   1, 0x01, 0x00000020 },
	{ 0x000976,   1, 0x01, 0x00000001 },
	{ 0x000977,   1, 0x01, 0x00000020 },
	{ 0x000978,   1, 0x01, 0x00000001 },
	{ 0x000957,   1, 0x01, 0x00000003 },
	{ 0x00095e,   1, 0x01, 0x20164010 },
	{ 0x00095f,   1, 0x01, 0x00000020 },
	{ 0x000a0d,   1, 0x01, 0x00000006 },
	{ 0x00097d,   1, 0x01, 0x0000000c },
	{ 0x000683,   1, 0x01, 0x00000006 },
	{ 0x000687,   1, 0x01, 0x003fffff },
	{ 0x0006a0,   1, 0x01, 0x00000005 },
	{ 0x000840,   1, 0x01, 0x00400008 },
	{ 0x000841,   1, 0x01, 0x08000080 },
	{ 0x000842,   1, 0x01, 0x00400008 },
	{ 0x000843,   1, 0x01, 0x08000080 },
	{ 0x000818,   8, 0x01, 0x00000000 },
	{ 0x000848,  16, 0x01, 0x00000000 },
	{ 0x000738,   1, 0x01, 0x00000000 },
	{ 0x0006aa,   1, 0x01, 0x00000001 },
	{ 0x0006ab,   1, 0x01, 0x00000002 },
	{ 0x0006ac,   1, 0x01, 0x00000080 },
	{ 0x0006ad,   2, 0x01, 0x00000100 },
	{ 0x0006b1,   1, 0x01, 0x00000011 },
	{ 0x0006bb,   1, 0x01, 0x000000cf },
	{ 0x0006ce,   1, 0x01, 0x2a712488 },
	{ 0x000739,   1, 0x01, 0x4085c000 },
	{ 0x00073a,   1, 0x01, 0x00000080 },
	{ 0x000786,   1, 0x01, 0x80000100 },
	{ 0x00073c,   1, 0x01, 0x00010100 },
	{ 0x00073d,   1, 0x01, 0x02800000 },
	{ 0x000787,   1, 0x01, 0x000000cf },
	{ 0x00078c,   1, 0x01, 0x00000008 },
	{ 0x000792,   1, 0x01, 0x00000001 },
	{ 0x000794,   3, 0x01, 0x00000001 },
	{ 0x000797,   1, 0x01, 0x000000cf },
	{ 0x000836,   1, 0x01, 0x00000001 },
	{ 0x00079a,   1, 0x01, 0x00000002 },
	{ 0x000833,   1, 0x01, 0x04444480 },
	{ 0x0007a1,   1, 0x01, 0x00000001 },
	{ 0x0007a3,   3, 0x01, 0x00000001 },
	{ 0x000831,   1, 0x01, 0x00000004 },
	{ 0x000b07,   1, 0x01, 0x00000002 },
	{ 0x000b08,   2, 0x01, 0x00000100 },
	{ 0x000b0a,   1, 0x01, 0x00000001 },
	{ 0x000a04,   1, 0x01, 0x000000ff },
	{ 0x000a0b,   1, 0x01, 0x00000040 },
	{ 0x00097f,   1, 0x01, 0x00000100 },
	{ 0x000a02,   1, 0x01, 0x00000001 },
	{ 0x000809,   1, 0x01, 0x00000007 },
	{ 0x00c221,   1, 0x01, 0x00000040 },
	{ 0x00c1b0,   8, 0x01, 0x0000000f },
	{ 0x00c1b8,   1, 0x01, 0x0fac6881 },
	{ 0x00c1b9,   1, 0x01, 0x00fac688 },
	{ 0x00c401,   1, 0x01, 0x00000001 },
	{ 0x00c402,   1, 0x01, 0x00010001 },
	{ 0x00c403,   2, 0x01, 0x00000001 },
	{ 0x00c40e,   1, 0x01, 0x00000020 },
	{ 0x00c413,   4, 0x01, 0x88888888 },
	{ 0x00c423,   1, 0x01, 0x0000ff00 },
	{ 0x00c420,   1, 0x01, 0x00880101 },
	{ 0x01e100,   1, 0x01, 0x00000001 },
	{}
};

const struct gf100_gr_pack
gm200_grctx_pack_icmd[] = {
	{ gm200_grctx_init_icmd_0 },
	{}
};

static const struct gf100_gr_init
gm200_grctx_init_b197_0[] = {
	{ 0x000800,   8, 0x40, 0x00000000 },
	{ 0x000804,   8, 0x40, 0x00000000 },
	{ 0x000808,   8, 0x40, 0x00000400 },
	{ 0x00080c,   8, 0x40, 0x00000300 },
	{ 0x000810,   1, 0x04, 0x000000cf },
	{ 0x000850,   7, 0x40, 0x00000000 },
	{ 0x000814,   8, 0x40, 0x00000040 },
	{ 0x000818,   8, 0x40, 0x00000001 },
	{ 0x00081c,   8, 0x40, 0x00000000 },
	{ 0x000820,   8, 0x40, 0x00000000 },
	{ 0x001c00,  16, 0x10, 0x00000000 },
	{ 0x001c04,  16, 0x10, 0x00000000 },
	{ 0x001c08,  16, 0x10, 0x00000000 },
	{ 0x001c0c,  16, 0x10, 0x00000000 },
	{ 0x001d00,  16, 0x10, 0x00000000 },
	{ 0x001d04,  16, 0x10, 0x00000000 },
	{ 0x001d08,  16, 0x10, 0x00000000 },
	{ 0x001d0c,  16, 0x10, 0x00000000 },
	{ 0x001f00,  16, 0x08, 0x00000000 },
	{ 0x001f04,  16, 0x08, 0x00000000 },
	{ 0x001f80,  16, 0x08, 0x00000000 },
	{ 0x001f84,  16, 0x08, 0x00000000 },
	{ 0x002000,   1, 0x04, 0x00000000 },
	{ 0x002040,   1, 0x04, 0x00000011 },
	{ 0x002080,   1, 0x04, 0x00000020 },
	{ 0x0020c0,   1, 0x04, 0x00000030 },
	{ 0x002100,   1, 0x04, 0x00000040 },
	{ 0x002140,   1, 0x04, 0x00000051 },
	{ 0x00200c,   6, 0x40, 0x00000001 },
	{ 0x002010,   1, 0x04, 0x00000000 },
	{ 0x002050,   1, 0x04, 0x00000000 },
	{ 0x002090,   1, 0x04, 0x00000001 },
	{ 0x0020d0,   1, 0x04, 0x00000002 },
	{ 0x002110,   1, 0x04, 0x00000003 },
	{ 0x002150,   1, 0x04, 0x00000004 },
	{ 0x000380,   4, 0x20, 0x00000000 },
	{ 0x000384,   4, 0x20, 0x00000000 },
	{ 0x000388,   4, 0x20, 0x00000000 },
	{ 0x00038c,   4, 0x20, 0x00000000 },
	{ 0x000700,   4, 0x10, 0x00000000 },
	{ 0x000704,   4, 0x10, 0x00000000 },
	{ 0x000708,   4, 0x10, 0x00000000 },
	{ 0x002800, 128, 0x04, 0x00000000 },
	{ 0x000a00,  16, 0x20, 0x00000000 },
	{ 0x000a04,  16, 0x20, 0x00000000 },
	{ 0x000a08,  16, 0x20, 0x00000000 },
	{ 0x000a0c,  16, 0x20, 0x00000000 },
	{ 0x000a10,  16, 0x20, 0x00000000 },
	{ 0x000a14,  16, 0x20, 0x00000000 },
	{ 0x000a18,  16, 0x20, 0x00006420 },
	{ 0x000a1c,  16, 0x20, 0x00000000 },
	{ 0x000c00,  16, 0x10, 0x00000000 },
	{ 0x000c04,  16, 0x10, 0x00000000 },
	{ 0x000c08,  16, 0x10, 0x00000000 },
	{ 0x000c0c,  16, 0x10, 0x3f800000 },
	{ 0x000d00,   8, 0x08, 0xffff0000 },
	{ 0x000d04,   8, 0x08, 0xffff0000 },
	{ 0x000e00,  16, 0x10, 0x00000000 },
	{ 0x000e04,  16, 0x10, 0xffff0000 },
	{ 0x000e08,  16, 0x10, 0xffff0000 },
	{ 0x000d40,   4, 0x08, 0x00000000 },
	{ 0x000d44,   4, 0x08, 0x00000000 },
	{ 0x001e00,   8, 0x20, 0x00000001 },
	{ 0x001e04,   8, 0x20, 0x00000001 },
	{ 0x001e08,   8, 0x20, 0x00000002 },
	{ 0x001e0c,   8, 0x20, 0x00000001 },
	{ 0x001e10,   8, 0x20, 0x00000001 },
	{ 0x001e14,   8, 0x20, 0x00000002 },
	{ 0x001e18,   8, 0x20, 0x00000001 },
	{ 0x001480,   8, 0x10, 0x00000000 },
	{ 0x001484,   8, 0x10, 0x00000000 },
	{ 0x001488,   8, 0x10, 0x00000000 },
	{ 0x003400, 128, 0x04, 0x00000000 },
	{ 0x00030c,   1, 0x04, 0x00000001 },
	{ 0x001944,   1, 0x04, 0x00000000 },
	{ 0x001514,   1, 0x04, 0x00000000 },
	{ 0x000d68,   1, 0x04, 0x0000ffff },
	{ 0x00121c,   1, 0x04, 0x0fac6881 },
	{ 0x000fac,   1, 0x04, 0x00000001 },
	{ 0x001538,   1, 0x04, 0x00000001 },
	{ 0x000fe0,   2, 0x04, 0x00000000 },
	{ 0x000fe8,   1, 0x04, 0x00000014 },
	{ 0x000fec,   1, 0x04, 0x00000040 },
	{ 0x000ff0,   1, 0x04, 0x00000000 },
	{ 0x00179c,   1, 0x04, 0x00000000 },
	{ 0x001228,   1, 0x04, 0x00000400 },
	{ 0x00122c,   1, 0x04, 0x00000300 },
	{ 0x001230,   1, 0x04, 0x00010001 },
	{ 0x0007f8,   1, 0x04, 0x00000000 },
	{ 0x001208,   1, 0x04, 0x00000000 },
	{ 0x0015b4,   1, 0x04, 0x00000001 },
	{ 0x0015cc,   1, 0x04, 0x00000000 },
	{ 0x001534,   1, 0x04, 0x00000000 },
	{ 0x000754,   1, 0x04, 0x00000001 },
	{ 0x000fb0,   1, 0x04, 0x00000000 },
	{ 0x0015d0,   1, 0x04, 0x00000000 },
	{ 0x0011e0,   4, 0x04, 0x88888888 },
	{ 0x00153c,   1, 0x04, 0x00000000 },
	{ 0x0016b4,   1, 0x04, 0x00000003 },
	{ 0x000fa4,   1, 0x04, 0x00000001 },
	{ 0x000fbc,   4, 0x04, 0x0000ffff },
	{ 0x000fa8,   1, 0x04, 0x0000ffff },
	{ 0x000df8,   2, 0x04, 0x00000000 },
	{ 0x001948,   1, 0x04, 0x00000000 },
	{ 0x001970,   1, 0x04, 0x00000001 },
	{ 0x00161c,   1, 0x04, 0x000009f0 },
	{ 0x000dcc,   1, 0x04, 0x00000010 },
	{ 0x0015e4,   1, 0x04, 0x00000000 },
	{ 0x001160,  32, 0x04, 0x25e00040 },
	{ 0x001880,  32, 0x04, 0x00000000 },
	{ 0x000f84,   2, 0x04, 0x00000000 },
	{ 0x0017c8,   2, 0x04, 0x00000000 },
	{ 0x0017d0,   1, 0x04, 0x000000ff },
	{ 0x0017d4,   1, 0x04, 0xffffffff },
	{ 0x0017d8,   1, 0x04, 0x00000002 },
	{ 0x0017dc,   1, 0x04, 0x00000000 },
	{ 0x0015f4,   2, 0x04, 0x00000000 },
	{ 0x001434,   2, 0x04, 0x00000000 },
	{ 0x000d74,   1, 0x04, 0x00000000 },
	{ 0x0013a4,   1, 0x04, 0x00000000 },
	{ 0x001318,   1, 0x04, 0x00000001 },
	{ 0x001080,   2, 0x04, 0x00000000 },
	{ 0x001088,   2, 0x04, 0x00000001 },
	{ 0x001090,   1, 0x04, 0x00000000 },
	{ 0x001094,   1, 0x04, 0x00000001 },
	{ 0x001098,   1, 0x04, 0x00000000 },
	{ 0x00109c,   1, 0x04, 0x00000001 },
	{ 0x0010a0,   2, 0x04, 0x00000000 },
	{ 0x001644,   1, 0x04, 0x00000000 },
	{ 0x000748,   1, 0x04, 0x00000000 },
	{ 0x000de8,   1, 0x04, 0x00000000 },
	{ 0x001648,   1, 0x04, 0x00000000 },
	{ 0x0012a4,   1, 0x04, 0x00000000 },
	{ 0x001120,   4, 0x04, 0x00000000 },
	{ 0x001118,   1, 0x04, 0x00000000 },
	{ 0x00164c,   1, 0x04, 0x00000000 },
	{ 0x001658,   1, 0x04, 0x00000000 },
	{ 0x001910,   1, 0x04, 0x00000290 },
	{ 0x001518,   1, 0x04, 0x00000000 },
	{ 0x00165c,   1, 0x04, 0x00000001 },
	{ 0x001520,   1, 0x04, 0x00000000 },
	{ 0x001604,   1, 0x04, 0x00000000 },
	{ 0x001570,   1, 0x04, 0x00000000 },
	{ 0x0013b0,   2, 0x04, 0x3f800000 },
	{ 0x00020c,   1, 0x04, 0x00000000 },
	{ 0x001670,   1, 0x04, 0x30201000 },
	{ 0x001674,   1, 0x04, 0x70605040 },
	{ 0x001678,   1, 0x04, 0xb8a89888 },
	{ 0x00167c,   1, 0x04, 0xf8e8d8c8 },
	{ 0x00166c,   1, 0x04, 0x00000000 },
	{ 0x001680,   1, 0x04, 0x00ffff00 },
	{ 0x0012d0,   1, 0x04, 0x00000003 },
	{ 0x00113c,   1, 0x04, 0x00000000 },
	{ 0x0012d4,   1, 0x04, 0x00000002 },
	{ 0x001684,   2, 0x04, 0x00000000 },
	{ 0x000dac,   2, 0x04, 0x00001b02 },
	{ 0x000db4,   1, 0x04, 0x00000000 },
	{ 0x00168c,   1, 0x04, 0x00000000 },
	{ 0x0015bc,   1, 0x04, 0x00000000 },
	{ 0x00156c,   1, 0x04, 0x00000000 },
	{ 0x00187c,   1, 0x04, 0x00000000 },
	{ 0x001110,   1, 0x04, 0x00000001 },
	{ 0x000dc0,   3, 0x04, 0x00000000 },
	{ 0x000f40,   5, 0x04, 0x00000000 },
	{ 0x001234,   1, 0x04, 0x00000000 },
	{ 0x001690,   1, 0x04, 0x00000000 },
	{ 0x000790,   5, 0x04, 0x00000000 },
	{ 0x00077c,   1, 0x04, 0x00000000 },
	{ 0x001000,   1, 0x04, 0x00000010 },
	{ 0x0010fc,   1, 0x04, 0x00000000 },
	{ 0x001290,   1, 0x04, 0x00000000 },
	{ 0x000218,   1, 0x04, 0x00000010 },
	{ 0x0012d8,   1, 0x04, 0x00000000 },
	{ 0x0012dc,   1, 0x04, 0x00000010 },
	{ 0x000d94,   1, 0x04, 0x00000001 },
	{ 0x00155c,   2, 0x04, 0x00000000 },
	{ 0x001564,   1, 0x04, 0x00000fff },
	{ 0x001574,   2, 0x04, 0x00000000 },
	{ 0x00157c,   1, 0x04, 0x000fffff },
	{ 0x001354,   1, 0x04, 0x00000000 },
	{ 0x001610,   1, 0x04, 0x00000012 },
	{ 0x001608,   2, 0x04, 0x00000000 },
	{ 0x00260c,   1, 0x04, 0x00000000 },
	{ 0x0007ac,   1, 0x04, 0x00000000 },
	{ 0x00162c,   1, 0x04, 0x00000003 },
	{ 0x000210,   1, 0x04, 0x00000000 },
	{ 0x000320,   1, 0x04, 0x00000000 },
	{ 0x000324,   6, 0x04, 0x3f800000 },
	{ 0x000750,   1, 0x04, 0x00000000 },
	{ 0x000760,   1, 0x04, 0x39291909 },
	{ 0x000764,   1, 0x04, 0x79695949 },
	{ 0x000768,   1, 0x04, 0xb9a99989 },
	{ 0x00076c,   1, 0x04, 0xf9e9d9c9 },
	{ 0x000770,   1, 0x04, 0x30201000 },
	{ 0x000774,   1, 0x04, 0x70605040 },
	{ 0x000778,   1, 0x04, 0x00009080 },
	{ 0x000780,   1, 0x04, 0x39291909 },
	{ 0x000784,   1, 0x04, 0x79695949 },
	{ 0x000788,   1, 0x04, 0xb9a99989 },
	{ 0x00078c,   1, 0x04, 0xf9e9d9c9 },
	{ 0x0007d0,   1, 0x04, 0x30201000 },
	{ 0x0007d4,   1, 0x04, 0x70605040 },
	{ 0x0007d8,   1, 0x04, 0x00009080 },
	{ 0x001004,   1, 0x04, 0x00000000 },
	{ 0x001240,   8, 0x04, 0x00000000 },
	{ 0x00037c,   1, 0x04, 0x00000001 },
	{ 0x000740,   1, 0x04, 0x00000000 },
	{ 0x001148,   1, 0x04, 0x00000000 },
	{ 0x000fb4,   1, 0x04, 0x00000000 },
	{ 0x000fb8,   1, 0x04, 0x00000002 },
	{ 0x001130,   1, 0x04, 0x00000002 },
	{ 0x000fd4,   2, 0x04, 0x00000000 },
	{ 0x001030,   1, 0x04, 0x20181008 },
	{ 0x001034,   1, 0x04, 0x40383028 },
	{ 0x001038,   1, 0x04, 0x60585048 },
	{ 0x00103c,   1, 0x04, 0x80787068 },
	{ 0x000744,   1, 0x04, 0x00000000 },
	{ 0x002600,   1, 0x04, 0x00000000 },
	{ 0x001918,   1, 0x04, 0x00000000 },
	{ 0x00191c,   1, 0x04, 0x00000900 },
	{ 0x001920,   1, 0x04, 0x00000405 },
	{ 0x001308,   1, 0x04, 0x00000001 },
	{ 0x001924,   1, 0x04, 0x00000000 },
	{ 0x0013ac,   1, 0x04, 0x00000000 },
	{ 0x00192c,   1, 0x04, 0x00000001 },
	{ 0x00193c,   1, 0x04, 0x00002c1c },
	{ 0x000d7c,   1, 0x04, 0x00000000 },
	{ 0x000f8c,   1, 0x04, 0x00000000 },
	{ 0x0002c0,   1, 0x04, 0x00000001 },
	{ 0x001510,   1, 0x04, 0x00000000 },
	{ 0x001940,   1, 0x04, 0x00000000 },
	{ 0x000ff4,   2, 0x04, 0x00000000 },
	{ 0x00194c,   2, 0x04, 0x00000000 },
	{ 0x001968,   1, 0x04, 0x00000000 },
	{ 0x001590,   1, 0x04, 0x0000003f },
	{ 0x0007e8,   4, 0x04, 0x00000000 },
	{ 0x00196c,   1, 0x04, 0x00000011 },
	{ 0x0002e4,   1, 0x04, 0x0000b001 },
	{ 0x00036c,   2, 0x04, 0x00000000 },
	{ 0x00197c,   1, 0x04, 0x00000000 },
	{ 0x000fcc,   2, 0x04, 0x00000000 },
	{ 0x0002d8,   1, 0x04, 0x00000040 },
	{ 0x001980,   1, 0x04, 0x00000080 },
	{ 0x001504,   1, 0x04, 0x00000080 },
	{ 0x001984,   1, 0x04, 0x00000000 },
	{ 0x000f60,   1, 0x04, 0x00000000 },
	{ 0x000f64,   1, 0x04, 0x00400040 },
	{ 0x000f68,   1, 0x04, 0x00002212 },
	{ 0x000f6c,   1, 0x04, 0x08080203 },
	{ 0x001108,   1, 0x04, 0x00000008 },
	{ 0x000f70,   1, 0x04, 0x00080001 },
	{ 0x000ffc,   1, 0x04, 0x00000000 },
	{ 0x001134,   1, 0x04, 0x00000000 },
	{ 0x000f1c,   1, 0x04, 0x00000000 },
	{ 0x0011f8,   1, 0x04, 0x00000000 },
	{ 0x001138,   1, 0x04, 0x00000001 },
	{ 0x000300,   1, 0x04, 0x00000001 },
	{ 0x0013a8,   1, 0x04, 0x00000000 },
	{ 0x001224,   1, 0x04, 0x00000000 },
	{ 0x0012ec,   1, 0x04, 0x00000000 },
	{ 0x001310,   1, 0x04, 0x00000000 },
	{ 0x001314,   1, 0x04, 0x00000001 },
	{ 0x001380,   1, 0x04, 0x00000000 },
	{ 0x001384,   4, 0x04, 0x00000001 },
	{ 0x001394,   1, 0x04, 0x00000000 },
	{ 0x00139c,   1, 0x04, 0x00000000 },
	{ 0x001398,   1, 0x04, 0x00000000 },
	{ 0x001594,   1, 0x04, 0x00000000 },
	{ 0x001598,   4, 0x04, 0x00000001 },
	{ 0x000f54,   3, 0x04, 0x00000000 },
	{ 0x0019bc,   1, 0x04, 0x00000000 },
	{ 0x000f9c,   2, 0x04, 0x00000000 },
	{ 0x0012cc,   1, 0x04, 0x00000000 },
	{ 0x0012e8,   1, 0x04, 0x00000000 },
	{ 0x00130c,   1, 0x04, 0x00000001 },
	{ 0x001360,   8, 0x04, 0x00000000 },
	{ 0x00133c,   2, 0x04, 0x00000001 },
	{ 0x001344,   1, 0x04, 0x00000002 },
	{ 0x001348,   2, 0x04, 0x00000001 },
	{ 0x001350,   1, 0x04, 0x00000002 },
	{ 0x001358,   1, 0x04, 0x00000001 },
	{ 0x0012e4,   1, 0x04, 0x00000000 },
	{ 0x00131c,   4, 0x04, 0x00000000 },
	{ 0x0019c0,   1, 0x04, 0x00000000 },
	{ 0x001140,   1, 0x04, 0x00000000 },
	{ 0x000dd0,   1, 0x04, 0x00000000 },
	{ 0x000dd4,   1, 0x04, 0x00000001 },
	{ 0x0002f4,   1, 0x04, 0x00000000 },
	{ 0x0019c4,   1, 0x04, 0x00000000 },
	{ 0x0019c8,   1, 0x04, 0x00001500 },
	{ 0x00135c,   1, 0x04, 0x00000000 },
	{ 0x000f90,   1, 0x04, 0x00000000 },
	{ 0x0019e0,   8, 0x04, 0x00000001 },
	{ 0x0019cc,   1, 0x04, 0x00000001 },
	{ 0x00111c,   1, 0x04, 0x00000001 },
	{ 0x0015b8,   1, 0x04, 0x00000000 },
	{ 0x001a00,   1, 0x04, 0x00001111 },
	{ 0x001a04,   7, 0x04, 0x00000000 },
	{ 0x000d6c,   2, 0x04, 0xffff0000 },
	{ 0x0010f8,   1, 0x04, 0x00001010 },
	{ 0x000d80,   5, 0x04, 0x00000000 },
	{ 0x000da0,   1, 0x04, 0x00000000 },
	{ 0x0007a4,   2, 0x04, 0x00000000 },
	{ 0x001508,   1, 0x04, 0x80000000 },
	{ 0x00150c,   1, 0x04, 0x40000000 },
	{ 0x001668,   1, 0x04, 0x00000000 },
	{ 0x000318,   2, 0x04, 0x00000008 },
	{ 0x000d9c,   1, 0x04, 0x00000001 },
	{ 0x000f14,   1, 0x04, 0x00000000 },
	{ 0x000374,   1, 0x04, 0x00000000 },
	{ 0x000378,   1, 0x04, 0x0000000c },
	{ 0x0007dc,   1, 0x04, 0x00000000 },
	{ 0x00074c,   1, 0x04, 0x00000055 },
	{ 0x001420,   1, 0x04, 0x00000003 },
	{ 0x001008,   1, 0x04, 0x00000008 },
	{ 0x00100c,   1, 0x04, 0x00000040 },
	{ 0x001010,   1, 0x04, 0x0000012c },
	{ 0x000d60,   1, 0x04, 0x00000040 },
	{ 0x001018,   1, 0x04, 0x00000020 },
	{ 0x00101c,   1, 0x04, 0x00000001 },
	{ 0x001020,   1, 0x04, 0x00000020 },
	{ 0x001024,   1, 0x04, 0x00000001 },
	{ 0x001444,   3, 0x04, 0x00000000 },
	{ 0x000360,   1, 0x04, 0x20164010 },
	{ 0x000364,   1, 0x04, 0x00000020 },
	{ 0x000368,   1, 0x04, 0x00000000 },
	{ 0x000da8,   1, 0x04, 0x00000030 },
	{ 0x000de4,   1, 0x04, 0x00000000 },
	{ 0x000204,   1, 0x04, 0x00000006 },
	{ 0x0002d0,   1, 0x04, 0x003fffff },
	{ 0x001220,   1, 0x04, 0x00000005 },
	{ 0x000fdc,   1, 0x04, 0x00000000 },
	{ 0x000f98,   1, 0x04, 0x00400008 },
	{ 0x001284,   1, 0x04, 0x08000080 },
	{ 0x001450,   1, 0x04, 0x00400008 },
	{ 0x001454,   1, 0x04, 0x08000080 },
	{ 0x000214,   1, 0x04, 0x00000000 },
	{}
};

const struct gf100_gr_pack
gm200_grctx_pack_mthd[] = {
	{ gm200_grctx_init_b197_0, 0xb197 },
	{ gf100_grctx_init_902d_0, 0x902d },
	{}
};

/*******************************************************************************
 * PGRAPH context implementation
 ******************************************************************************/

void
gm200_grctx_generate_tpcid(struct gf100_gr *gr)
{
	struct nvkm_device *device = gr->base.engine.subdev.device;
	int gpc, tpc, id;

	for (tpc = 0, id = 0; tpc < 4; tpc++) {
		for (gpc = 0; gpc < gr->gpc_nr; gpc++) {
			if (tpc < gr->tpc_nr[gpc]) {
				nvkm_wr32(device, TPC_UNIT(gpc, tpc, 0x698), id);
				nvkm_wr32(device, GPC_UNIT(gpc, 0x0c10 + tpc * 4), id);
				nvkm_wr32(device, TPC_UNIT(gpc, tpc, 0x088), id);
				id++;
			}
		}
	}
}

static void
gm200_grctx_generate_rop_active_fbps(struct gf100_gr *gr)
{
	struct nvkm_device *device = gr->base.engine.subdev.device;
	const u32 fbp_count = nvkm_rd32(device, 0x12006c);
	nvkm_mask(device, 0x408850, 0x0000000f, fbp_count); /* zrop */
	nvkm_mask(device, 0x408958, 0x0000000f, fbp_count); /* crop */
}

void
gm200_grctx_generate_405b60(struct gf100_gr *gr)
{
	struct nvkm_device *device = gr->base.engine.subdev.device;
	const u32 dist_nr = DIV_ROUND_UP(gr->tpc_total, 4);
	u32 dist[TPC_MAX / 4] = {};
	u32 gpcs[GPC_MAX] = {};
	u8  tpcnr[GPC_MAX];
	int tpc, gpc, i;

	memcpy(tpcnr, gr->tpc_nr, sizeof(gr->tpc_nr));

	/* won't result in the same distribution as the binary driver where
	 * some of the gpcs have more tpcs than others, but this shall do
	 * for the moment.  the code for earlier gpus has this issue too.
	 */
	for (gpc = -1, i = 0; i < gr->tpc_total; i++) {
		do {
			gpc = (gpc + 1) % gr->gpc_nr;
		} while(!tpcnr[gpc]);
		tpc = gr->tpc_nr[gpc] - tpcnr[gpc]--;

		dist[i / 4] |= ((gpc << 4) | tpc) << ((i % 4) * 8);
		gpcs[gpc] |= i << (tpc * 8);
	}

	for (i = 0; i < dist_nr; i++)
		nvkm_wr32(device, 0x405b60 + (i * 4), dist[i]);
	for (i = 0; i < gr->gpc_nr; i++)
		nvkm_wr32(device, 0x405ba0 + (i * 4), gpcs[i]);
}

void
gm200_grctx_generate_main(struct gf100_gr *gr, struct gf100_grctx *info)
{
	struct nvkm_device *device = gr->base.engine.subdev.device;
	const struct gf100_grctx_func *grctx = gr->func->grctx;
	u32 tmp;
	int i;

	gf100_gr_mmio(gr, gr->fuc_sw_ctx);

	nvkm_wr32(device, 0x404154, 0x00000000);

	grctx->bundle(info);
	grctx->pagepool(info);
	grctx->attrib(info);
	grctx->unkn(gr);

	gm200_grctx_generate_tpcid(gr);
	gf100_grctx_generate_r406028(gr);
	gk104_grctx_generate_r418bb8(gr);

	for (i = 0; i < 8; i++)
		nvkm_wr32(device, 0x4064d0 + (i * 0x04), 0x00000000);
	nvkm_wr32(device, 0x406500, 0x00000000);

	nvkm_wr32(device, 0x405b00, (gr->tpc_total << 8) | gr->gpc_nr);

	gm200_grctx_generate_rop_active_fbps(gr);

	for (tmp = 0, i = 0; i < gr->gpc_nr; i++)
		tmp |= ((1 << gr->tpc_nr[i]) - 1) << (i * 4);
	nvkm_wr32(device, 0x4041c4, tmp);

	gm200_grctx_generate_405b60(gr);

	gf100_gr_icmd(gr, grctx->icmd);
	nvkm_wr32(device, 0x404154, 0x00000800);
	gf100_gr_mthd(gr, grctx->mthd);

	nvkm_mask(device, 0x418e94, 0xffffffff, 0xc4230000);
	nvkm_mask(device, 0x418e4c, 0xffffffff, 0x70000000);
}

const struct gf100_grctx_func
gm200_grctx = {
	.main  = gm200_grctx_generate_main,
	.unkn  = gk104_grctx_generate_unkn,
	.icmd  = gm200_grctx_pack_icmd,
	.mthd  = gm200_grctx_pack_mthd,
	.bundle = gm107_grctx_generate_bundle,
	.bundle_size = 0x3000,
	.bundle_min_gpm_fifo_depth = 0x180,
	.bundle_token_limit = 0x780,
	.pagepool = gm107_grctx_generate_pagepool,
	.pagepool_size = 0x20000,
	.attrib = gm107_grctx_generate_attrib,
	.attrib_nr_max = 0x600,
	.attrib_nr = 0x400,
	.alpha_nr_max = 0x1800,
	.alpha_nr = 0x1000,
};
