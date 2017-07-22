/*
 * Copyright (C) 2016 Marcin Kościelnicki <koriakin@0x04.net>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pgraph.h"
#include "pgraph_mthd.h"
#include "pgraph_class.h"
#include "nva.h"

namespace hwtest {
namespace pgraph {

static void adjust_orig_bundle(struct pgraph_state *state) {
	state->surf_unk800 = 0;
}

class MthdEmuCelsiusClip : public SingleMthdTest {
	int which;
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0x8000);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			if (which == 0)
				exp.kelvin_bundle_clip_h = val;
			else
				exp.kelvin_bundle_clip_v = val;
			pgraph_kelvin_bundle(&exp, 0x6d + which, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
public:
	MthdEmuCelsiusClip(hwtest::TestOptions &opt, uint32_t seed, const std::string &name, int trapbit, uint32_t cls, uint32_t mthd, int which)
	: SingleMthdTest(opt, seed, name, trapbit, cls, mthd), which(which) {}
};

class MthdKelvinClip : public SingleMthdTest {
	int which;
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0xf000f000);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			if (which == 0)
				exp.kelvin_bundle_clip_h = val;
			else
				exp.kelvin_bundle_clip_v = val;
			pgraph_kelvin_bundle(&exp, 0x6d + which, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
public:
	MthdKelvinClip(hwtest::TestOptions &opt, uint32_t seed, const std::string &name, int trapbit, uint32_t cls, uint32_t mthd, int which)
	: SingleMthdTest(opt, seed, name, trapbit, cls, mthd), which(which) {}
};

class MthdEmuCelsiusTexOffset : public SingleMthdTest {
	void adjust_orig_mthd() override {
		val &= ~0x7f;
		if (rnd() & 1) {
			val ^= 1 << (rnd() & 0x1f);
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0x7f);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			exp.kelvin_bundle_tex_offset[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x89 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexFormat : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 3)
				insrt(val, 0, 2, 1);
			if (rnd() & 3)
				insrt(val, 3, 2, 1);
			if (rnd() & 3)
				insrt(val, 5, 2, 1);
			if (!(rnd() & 3))
				insrt(val, 7, 5, 0x19 + rnd() % 4);
			if (rnd() & 1)
				insrt(val, 20, 4, extr(val, 16, 4));
			if (rnd() & 3)
				insrt(val, 24, 3, 3);
			if (rnd() & 3)
				insrt(val, 28, 3, 3);
			if (rnd() & 1) {
				if (rnd() & 3)
					insrt(val, 2, 1, 0);
				if (rnd() & 3)
					insrt(val, 12, 4, 1);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		int mips = extr(val, 12, 4);
		int su = extr(val, 16, 4);
		int sv = extr(val, 20, 4);
		int fmt = extr(val, 7, 5);
		if (!extr(val, 0, 2) || extr(val, 0, 2) == 3)
			return false;
		if (extr(val, 2, 1)) {
			if (su != sv)
				return false;
			if (su >= 0xa && (fmt == 6 || fmt == 7 || fmt == 0xb || fmt == 0xe || fmt == 0xf))
				return false;
			if (su >= 0xb)
				return false;
		}
		if (!extr(val, 3, 2) || extr(val, 3, 2) == 3)
			return false;
		if (!extr(val, 5, 2) || extr(val, 5, 2) == 3)
			return false;
		if (fmt == 0xd)
			return false;
		if (fmt >= 0x1d)
			return false;
		if (mips > 0xc || mips == 0)
			return false;
		if (fmt >= 0x19) {
			if (cls != 0x99)
				return false;
		}
		if (fmt >= 0x10) {
			if (extr(val, 2, 1))
				return false;
			if (extr(val, 24, 3) != 3)
				return false;
			if (extr(val, 28, 3) != 3)
				return false;
			if (mips != 1)
				return false;
		}
		if (su > 0xb || sv > 0xb)
			return false;
		if (extr(val, 24, 3) < 1 || extr(val, 24, 3) > 3)
			return false;
		if (extr(val, 28, 3) < 1 || extr(val, 28, 3) > 3)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			int mode = extr(val, 2, 1) ? 3 : 1;
			if (idx == 0)
				insrt(exp.kelvin_bundle_unk067, 0, 3, mode);
			else
				insrt(exp.kelvin_bundle_unk067, 5, 5, mode);
			pgraph_kelvin_bundle(&exp, 0x67, exp.kelvin_bundle_unk067, true);
			insrt(exp.kelvin_bundle_tex_wrap[idx], 0, 3, extr(val, 24, 3));
			insrt(exp.kelvin_bundle_tex_wrap[idx], 4, 1, extr(val, 27, 1));
			insrt(exp.kelvin_bundle_tex_wrap[idx], 8, 3, extr(val, 28, 3));
			insrt(exp.kelvin_bundle_tex_wrap[idx], 12, 1, extr(val, 31, 1));
			pgraph_kelvin_bundle(&exp, 0x6f + idx, exp.kelvin_bundle_tex_wrap[idx], true);
			int mips = extr(val, 12, 4);
			int su = extr(val, 16, 4);
			int sv = extr(val, 20, 4);
			if (mips > su && mips > sv)
				mips = std::max(su, sv) + 1;
			insrt(exp.kelvin_bundle_tex_format[idx], 1, 1, extr(val, 1, 1));
			insrt(exp.kelvin_bundle_tex_format[idx], 2, 1, extr(val, 2, 1));
			insrt(exp.kelvin_bundle_tex_format[idx], 4, 1, extr(val, 4, 1));
			insrt(exp.kelvin_bundle_tex_format[idx], 5, 1, extr(val, 6, 1));
			insrt(exp.kelvin_bundle_tex_format[idx], 8, 5, extr(val, 7, 5));
			insrt(exp.kelvin_bundle_tex_format[idx], 13, 2, 0);
			insrt(exp.kelvin_bundle_tex_format[idx], 16, 4, mips);
			insrt(exp.kelvin_bundle_tex_format[idx], 20, 4, su);
			insrt(exp.kelvin_bundle_tex_format[idx], 24, 4, sv);
			pgraph_kelvin_bundle(&exp, 0x81 + idx, exp.kelvin_bundle_tex_format[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexControl : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		if (extr(val, 31, 1))
			return false;
		if (extr(val, 5, 1))
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			insrt(exp.kelvin_xf_mode_c[1], idx * 16, 1, extr(val, 30, 1));
			pgraph_kelvin_xf_mode(&exp);
			exp.kelvin_bundle_tex_control[idx] = val & 0x7fffffff;

			pgraph_kelvin_bundle(&exp, 0x73 + idx, exp.kelvin_bundle_tex_control[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexPitch : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			val &= ~0xffff;
			if (!(rnd() & 3)) {
				val &= 0xe00f0000;
			}
			if (!(rnd() & 3))
				val = 0;
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
				val ^= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0xffff) && !!(val & 0xfff80000);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			exp.kelvin_bundle_tex_pitch[idx] = val & 0xffff0000;
			pgraph_kelvin_bundle(&exp, 0x77 + idx, exp.kelvin_bundle_tex_pitch[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexUnk238 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			exp.kelvin_bundle_tex_unk238[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x7b + idx, exp.kelvin_bundle_tex_unk238[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexRect : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 1) {
				val &= ~0xf000f800;
			}
			if (rnd() & 1) {
				if (rnd() & 1) {
					val &= ~0xffff;
				} else {
					val &= ~0xffff0000;
				}
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		if (extr(val, 16, 1))
			return false;
		if (!extr(val, 0, 16) || extr(val, 0, 16) >= 0x800)
			return false;
		if (!extr(val, 17, 15) || extr(val, 17, 15) >= 0x800)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			exp.kelvin_bundle_tex_rect[idx] = val & 0x1fff1fff;
			pgraph_kelvin_bundle(&exp, 0x85 + idx, exp.kelvin_bundle_tex_rect[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexFilter : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 3)
				insrt(val, 13, 11, 0);
			if (rnd() & 3)
				insrt(val, 24, 4, 1);
			if (rnd() & 3)
				insrt(val, 28, 4, 1);
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		if (extr(val, 13, 11))
			return false;
		if (extr(val, 24, 4) < 1 || extr(val, 24, 4) > 6)
			return false;
		if (extr(val, 28, 4) < 1 || extr(val, 28, 4) > 2)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			insrt(exp.kelvin_bundle_tex_filter[idx], 0, 13, extr(val, 0, 13));
			insrt(exp.kelvin_bundle_tex_filter[idx], 16, 4, extr(val, 24, 4));
			insrt(exp.kelvin_bundle_tex_filter[idx], 20, 2, 0);
			insrt(exp.kelvin_bundle_tex_filter[idx], 24, 2, extr(val, 28, 2));
			insrt(exp.kelvin_bundle_tex_filter[idx], 26, 2, 0);
			pgraph_kelvin_bundle(&exp, 0x7d + idx, exp.kelvin_bundle_tex_filter[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexPalette : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			val &= ~0x3c;
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0x3e);
	}
	void emulate_mthd() override {
		if (!extr(exp.nsource, 1, 1)) {
			exp.kelvin_bundle_tex_palette[idx] = val & ~0x32;
			pgraph_kelvin_bundle(&exp, 0x8d + idx, exp.kelvin_bundle_tex_palette[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdEmuCelsiusTexColorKey : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool can_warn() override {
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1)) {
			warn(4);
		} else {
			if (!extr(exp.nsource, 1, 1)) {
				exp.kelvin_bundle_tex_color_key[idx] = val;
				pgraph_kelvin_bundle(&exp, 0x1c + idx, val, true);
			}
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexOffset : public SingleMthdTest {
	void adjust_orig_mthd() override {
		val &= ~0x7f;
		if (rnd() & 1) {
			val ^= 1 << (rnd() & 0x1f);
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0x7f);
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_offset[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x89 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexFormat : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 3)
				insrt(val, 0, 2, 1);
			if (rnd() & 3)
				insrt(val, 6, 2, 0);
			if (rnd() & 3)
				insrt(val, 15, 1, 0);
			if (rnd() & 3)
				insrt(val, 20, 4, 3);
			if (rnd() & 3)
				insrt(val, 24, 4, 3);
			if (rnd() & 3)
				insrt(val, 28, 4, 0);
			if (rnd() & 1)
				insrt(val, 24, 4, extr(val, 20, 4));
			if (rnd() & 3) {
				if (extr(val, 4, 2) == 2)
					insrt(val, 28, 4, 0);
				if (extr(val, 4, 2) == 1)
					insrt(val, 24, 8, 0);
			}
			if (rnd() & 1) {
				if (rnd() & 3)
					insrt(val, 2, 1, 0);
				if (rnd() & 3)
					insrt(val, 16, 4, 1);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		bool cube = extr(val, 2, 1);
		bool border = extr(val, 3, 1);
		int mode = extr(val, 4, 2);
		int fmt = extr(val, 8, 7);
		int mips = extr(val, 16, 4);
		int su = extr(val, 20, 4);
		int sv = extr(val, 24, 4);
		int sw = extr(val, 28, 4);
		bool rect = (fmt >= 0x10 && fmt <= 0x18) || (fmt >= 0x1b && fmt <= 0x26) || (fmt >= 0x2e && fmt <= 0x31) || (fmt >= 0x34 && fmt <= 0x37) || (fmt >= 0x3d && fmt <= 0x41);
		bool zcomp = (fmt >= 0x2a && fmt <= 0x31);
		if (!extr(val, 0, 2) || extr(val, 0, 2) == 3)
			return false;
		if (fmt >= 0x42)
			return false;
		switch (fmt) {
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0d:
			case 0x21:
			case 0x22:
			case 0x23:
				return false;
		}
		if (rect) {
			if (cube)
				return false;
			if (mips != 1)
				return false;
			if (mode == 3)
				return false;
		}
		if (cube) {
			if (mode != 2)
				return false;
			if (zcomp)
				return false;
			if (su != sv)
				return false;
		}
		if (mode == 0) {
			return false;
		} else if (mode == 1) {
			int max = border ? 0xc : 0xb;
			if (su > max)
				return false;
			if (sv || sw)
				return false;
		} else if (mode == 2) {
			int max = border ? 0xc : 0xb;
			if (su > max || sv > max)
				return false;
			if (sw)
				return false;
		} else if (mode == 3) {
			int max = border ? 0x9 : 0x8;
			if (su > max || sv > max || sw > max)
				return false;
			if (zcomp)
				return false;
		}
		if (extr(val, 6, 2))
			return false;
		if (extr(val, 15, 1))
			return false;
		if (mips > 0xd || mips == 0)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			int mode = extr(val, 4, 2);
			int fmt = extr(val, 8, 7);
			bool zcomp = (fmt >= 0x2a && fmt <= 0x31);
			int mips = extr(val, 16, 4);
			int su = extr(val, 20, 4);
			int sv = extr(val, 24, 4);
			int sw = extr(val, 28, 4);
			if (mips > su && mips > sv && mips > sw)
				mips = std::max(su, std::max(sv, sw)) + 1;
			insrt(exp.kelvin_xf_mode_c[1 - (idx >> 1)], (idx & 1) * 16 + 2, 1, mode == 3 || zcomp);
			pgraph_kelvin_xf_mode(&exp);
			insrt(exp.kelvin_bundle_tex_format[idx], 1, 3, extr(val, 1, 3));
			insrt(exp.kelvin_bundle_tex_format[idx], 6, 2, mode);
			insrt(exp.kelvin_bundle_tex_format[idx], 8, 7, fmt);
			insrt(exp.kelvin_bundle_tex_format[idx], 16, 4, mips);
			insrt(exp.kelvin_bundle_tex_format[idx], 20, 4, su);
			insrt(exp.kelvin_bundle_tex_format[idx], 24, 4, sv);
			insrt(exp.kelvin_bundle_tex_format[idx], 28, 4, sw);
			pgraph_kelvin_bundle(&exp, 0x81 + idx, exp.kelvin_bundle_tex_format[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexWrap : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			val &= 0x01171717;
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		int mode_s = extr(val, 0, 3);
		int mode_t = extr(val, 8, 3);
		int mode_r = extr(val, 16, 3);
		if (val & ~0x01171717)
			return false;
		if (mode_s < 1 || mode_s > 5)
			return false;
		if (mode_t < 1 || mode_t > 5)
			return false;
		if (mode_r < 1 || mode_r > 5)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_wrap[idx] = val & 0x01171717;
			pgraph_kelvin_bundle(&exp, 0x6f + idx, exp.kelvin_bundle_tex_wrap[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexControl : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		if (extr(val, 31, 1))
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			insrt(exp.kelvin_xf_mode_c[1 - (idx >> 1)], (idx & 1) * 16, 1, extr(val, 30, 1));
			pgraph_kelvin_xf_mode(&exp);
			exp.kelvin_bundle_tex_control[idx] = val & 0x7fffffff;

			pgraph_kelvin_bundle(&exp, 0x73 + idx, exp.kelvin_bundle_tex_control[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexPitch : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			val &= ~0xffff;
			if (!(rnd() & 3)) {
				val &= 0xe00f0000;
			}
			if (!(rnd() & 3))
				val = 0;
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
				val ^= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0xffff) && !!(val & 0xfff80000);
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_pitch[idx] = val & 0xffff0000;
			pgraph_kelvin_bundle(&exp, 0x77 + idx, exp.kelvin_bundle_tex_pitch[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexFilter : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 3)
				insrt(val, 13, 3, 1);
			if (rnd() & 3)
				insrt(val, 16, 6, 1);
			if (rnd() & 3)
				insrt(val, 22, 2, 0);
			if (rnd() & 3)
				insrt(val, 24, 4, 1);
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val ^= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		int unk = extr(val, 13, 3);
		int min = extr(val, 16, 6);
		int mag = extr(val, 24, 4);
		if (extr(val, 22, 2))
			return false;
		if (unk != 1 && unk != 2)
			return false;
		if (min < 1 || min > 7)
			return false;
		if (mag != 1 && mag != 2 && mag != 4)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_filter[idx] = val & 0xff3fffff;
			pgraph_kelvin_bundle(&exp, 0x7d + idx, exp.kelvin_bundle_tex_filter[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexRect : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			if (rnd() & 1) {
				val &= ~0xf000f000;
			}
			if (rnd() & 1) {
				if (rnd() & 1) {
					val &= ~0xffff;
				} else {
					val &= ~0xffff0000;
				}
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		if (!extr(val, 0, 16) || extr(val, 0, 16) > 0x1000)
			return false;
		if (!extr(val, 16, 16) || extr(val, 16, 16) > 0x1000)
			return false;
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_rect[idx] = val & 0x1fff1fff;
			pgraph_kelvin_bundle(&exp, 0x85 + idx, exp.kelvin_bundle_tex_rect[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexPalette : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (rnd() & 1) {
			val &= ~0x32;
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
			if (rnd() & 1) {
				val |= 1 << (rnd() & 0x1f);
			}
		}
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return !(val & 0x32);
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_palette[idx] = val & ~0x32;
			pgraph_kelvin_bundle(&exp, 0x8d + idx, exp.kelvin_bundle_tex_palette[idx], true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexBorderColor : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_border_color[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x03 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk10 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (!(rnd() & 0xf))
			insrt(val, 0, 31, 0x40ffffe0 + (rnd() & 0x3f));
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return extr(val, 0, 31) <= 0x41000000;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk10[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x07 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk11 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (!(rnd() & 0xf))
			insrt(val, 0, 31, 0x40ffffe0 + (rnd() & 0x3f));
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return extr(val, 0, 31) <= 0x41000000;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk11[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x0a + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk12 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (!(rnd() & 0xf))
			insrt(val, 0, 31, 0x40ffffe0 + (rnd() & 0x3f));
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return extr(val, 0, 31) <= 0x41000000;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk12[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x0d + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk13 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		if (!(rnd() & 0xf))
			insrt(val, 0, 31, 0x40ffffe0 + (rnd() & 0x3f));
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return extr(val, 0, 31) <= 0x41000000;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk13[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x10 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk14 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk14[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x13 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

class MthdKelvinTexUnk15 : public SingleMthdTest {
	void adjust_orig_mthd() override {
		adjust_orig_bundle(&orig);
	}
	bool is_valid_val() override {
		return true;
	}
	void emulate_mthd() override {
		if (extr(exp.kelvin_unkf5c, 0, 1))
			nv04_pgraph_blowup(&exp, 0x40000);
		if (extr(exp.kelvin_unkf5c, 4, 1) && extr(exp.debug[3], 3, 1))
			nv04_pgraph_blowup(&exp, 0x80000);
		if (!extr(exp.nsource, 1, 1) && !extr(exp.nsource, 18, 1) && !extr(exp.nsource, 19, 1)) {
			exp.kelvin_bundle_tex_unk15[idx] = val;
			pgraph_kelvin_bundle(&exp, 0x16 + idx, val, true);
		}
	}
	using SingleMthdTest::SingleMthdTest;
};

std::vector<SingleMthdTest *> EmuCelsius::mthds() {
	std::vector<SingleMthdTest *> res = {
		new MthdNop(opt, rnd(), "nop", -1, cls, 0x100),
		new MthdNotify(opt, rnd(), "notify", 0, cls, 0x104),
		new MthdWarning(opt, rnd(), "warning", 2, cls, 0x108),
		new MthdState(opt, rnd(), "state", -1, cls, 0x10c),
		new MthdSync(opt, rnd(), "sync", 1, cls, 0x110),
		new MthdPmTrigger(opt, rnd(), "pm_trigger", -1, cls, 0x140),
		new MthdDmaNotify(opt, rnd(), "dma_notify", 3, cls, 0x180),
		new UntestedMthd(opt, rnd(), "dma_tex_a", 4, cls, 0x184), // XXX
		new UntestedMthd(opt, rnd(), "dma_tex_b", 5, cls, 0x188), // XXX
		new UntestedMthd(opt, rnd(), "dma_vtx", 6, cls, 0x18c), // XXX
		new UntestedMthd(opt, rnd(), "dma_state", 7, cls, 0x190), // XXX
		new MthdDmaSurf(opt, rnd(), "dma_surf_color", 8, cls, 0x194, 2, SURF_NV10),
		new MthdDmaSurf(opt, rnd(), "dma_surf_zeta", 9, cls, 0x198, 3, SURF_NV10),
		new MthdEmuCelsiusClip(opt, rnd(), "clip_h", 10, cls, 0x200, 0),
		new MthdEmuCelsiusClip(opt, rnd(), "clip_v", 10, cls, 0x204, 1),
		new MthdSurf3DFormat(opt, rnd(), "surf_format", 11, cls, 0x208, true),
		new MthdSurfPitch2(opt, rnd(), "surf_pitch_2", 12, cls, 0x20c, 2, 3, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "color_offset", 13, cls, 0x210, 2, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "zeta_offset", 14, cls, 0x214, 3, SURF_NV10),
		new MthdEmuCelsiusTexOffset(opt, rnd(), "tex_offset", 15, cls, 0x218, 2),
		new MthdEmuCelsiusTexFormat(opt, rnd(), "tex_format", 16, cls, 0x220, 2),
		new MthdEmuCelsiusTexControl(opt, rnd(), "tex_control", 17, cls, 0x228, 2),
		new MthdEmuCelsiusTexPitch(opt, rnd(), "tex_pitch", 18, cls, 0x230, 2),
		new MthdEmuCelsiusTexUnk238(opt, rnd(), "tex_unk238", 19, cls, 0x238, 2),
		new MthdEmuCelsiusTexRect(opt, rnd(), "tex_rect", 20, cls, 0x240, 2),
		new MthdEmuCelsiusTexFilter(opt, rnd(), "tex_filter", 21, cls, 0x248, 2),
		new MthdEmuCelsiusTexPalette(opt, rnd(), "tex_palette", 22, cls, 0x250, 2),
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x260, 8), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x280, 8), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x2a0, 2), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x2a8), // XXX
		new MthdEmuCelsiusTexColorKey(opt, rnd(), "tex_color_key", -1, cls, 0x2ac, 2),
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x2b4), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x2b8, 2), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x2c0, 0x10), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x300, 0x40), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x400, 0x100), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x800, 0x200), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1000, 0x400), // XXX
	};
	if (cls == 0x56) {
	} else {
		res.insert(res.end(), {
			new UntestedMthd(opt, rnd(), "unk114", -1, cls, 0x114), // XXX
			new MthdFlipSet(opt, rnd(), "flip_write", -1, cls, 0x120, 1, 1),
			new MthdFlipSet(opt, rnd(), "flip_read", -1, cls, 0x124, 1, 0),
			new MthdFlipSet(opt, rnd(), "flip_modulo", -1, cls, 0x128, 1, 2),
			new MthdFlipBumpWrite(opt, rnd(), "flip_bump_write", -1, cls, 0x12c, 1),
			new UntestedMthd(opt, rnd(), "flip_unk130", -1, cls, 0x130),
		});
	}
	return res;
}

std::vector<SingleMthdTest *> Kelvin::mthds() {
	std::vector<SingleMthdTest *> res = {
		new MthdNop(opt, rnd(), "nop", -1, cls, 0x100),
		new MthdNotify(opt, rnd(), "notify", -1, cls, 0x104),
		new MthdWarning(opt, rnd(), "warning", -1, cls, 0x108),
		new MthdState(opt, rnd(), "state", -1, cls, 0x10c),
		new MthdSync(opt, rnd(), "sync", -1, cls, 0x110),
		new MthdFlipSet(opt, rnd(), "flip_write", -1, cls, 0x120, 1, 1),
		new MthdFlipSet(opt, rnd(), "flip_read", -1, cls, 0x124, 1, 0),
		new MthdFlipSet(opt, rnd(), "flip_modulo", -1, cls, 0x128, 1, 2),
		new MthdFlipBumpWrite(opt, rnd(), "flip_bump_write", -1, cls, 0x12c, 1),
		new UntestedMthd(opt, rnd(), "flip_unk130", -1, cls, 0x130),
		new MthdPmTrigger(opt, rnd(), "pm_trigger", -1, cls, 0x140),
		new MthdDmaNotify(opt, rnd(), "dma_notify", -1, cls, 0x180),
		new UntestedMthd(opt, rnd(), "dma_tex_a", -1, cls, 0x184), // XXX
		new UntestedMthd(opt, rnd(), "dma_tex_b", -1, cls, 0x188), // XXX
		new UntestedMthd(opt, rnd(), "dma_state", -1, cls, 0x190), // XXX
		new MthdDmaSurf(opt, rnd(), "dma_surf_color", -1, cls, 0x194, 2, SURF_NV10),
		new MthdDmaSurf(opt, rnd(), "dma_surf_zeta", -1, cls, 0x198, 3, SURF_NV10),
		new UntestedMthd(opt, rnd(), "dma_vtx_a", -1, cls, 0x19c), // XXX
		new UntestedMthd(opt, rnd(), "dma_vtx_b", -1, cls, 0x1a0), // XXX
		new UntestedMthd(opt, rnd(), "dma_fence", -1, cls, 0x1a4), // XXX
		new UntestedMthd(opt, rnd(), "dma_query", -1, cls, 0x1a8), // XXX
		new MthdKelvinClip(opt, rnd(), "clip_h", -1, cls, 0x200, 0),
		new MthdKelvinClip(opt, rnd(), "clip_v", -1, cls, 0x204, 1),
		new MthdSurf3DFormat(opt, rnd(), "surf_format", -1, cls, 0x208, true),
		new MthdSurfPitch2(opt, rnd(), "surf_pitch_2", -1, cls, 0x20c, 2, 3, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "color_offset", -1, cls, 0x210, 2, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "zeta_offset", -1, cls, 0x214, 3, SURF_NV10),
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x218, 2), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x220, 8), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x240, 0x10), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x280, 0x20), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x300, 0x40), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x400, 0x100), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x800, 0x200), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1000, 0x200), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1800, 0x80), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1a00, 0x40), // XXX
		new MthdKelvinTexOffset(opt, rnd(), "tex_offset", -1, cls, 0x1b00, 4, 0x40),
		new MthdKelvinTexFormat(opt, rnd(), "tex_format", -1, cls, 0x1b04, 4, 0x40),
		new MthdKelvinTexWrap(opt, rnd(), "tex_wrap", -1, cls, 0x1b08, 4, 0x40),
		new MthdKelvinTexControl(opt, rnd(), "tex_control", -1, cls, 0x1b0c, 4, 0x40),
		new MthdKelvinTexPitch(opt, rnd(), "tex_pitch", -1, cls, 0x1b10, 4, 0x40),
		new MthdKelvinTexFilter(opt, rnd(), "tex_filter", -1, cls, 0x1b14, 4, 0x40),
		new MthdKelvinTexRect(opt, rnd(), "tex_rect", -1, cls, 0x1b1c, 4, 0x40),
		new MthdKelvinTexPalette(opt, rnd(), "tex_palette", -1, cls, 0x1b20, 4, 0x40),
		new MthdKelvinTexBorderColor(opt, rnd(), "tex_border_color", -1, cls, 0x1b24, 4, 0x40),
		new MthdKelvinTexUnk10(opt, rnd(), "tex_unk10", -1, cls, 0x1b68, 3, 0x40),
		new MthdKelvinTexUnk11(opt, rnd(), "tex_unk11", -1, cls, 0x1b6c, 3, 0x40),
		new MthdKelvinTexUnk12(opt, rnd(), "tex_unk12", -1, cls, 0x1b70, 3, 0x40),
		new MthdKelvinTexUnk13(opt, rnd(), "tex_unk13", -1, cls, 0x1b74, 3, 0x40),
		new MthdKelvinTexUnk14(opt, rnd(), "tex_unk14", -1, cls, 0x1b78, 3, 0x40),
		new MthdKelvinTexUnk15(opt, rnd(), "tex_unk15", -1, cls, 0x1b7c, 3, 0x40),
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1c00, 0x100), // XXX
	};
	if (cls == 0x597) {
		res.insert(res.end(), {
			new UntestedMthd(opt, rnd(), "dma_clipid", -1, cls, 0x1ac), // XXX
			new UntestedMthd(opt, rnd(), "dma_zcull", -1, cls, 0x1b0), // XXX
		});
	}
	return res;
}

std::vector<SingleMthdTest *> Rankine::mthds() {
	std::vector<SingleMthdTest *> res = {
		new MthdNop(opt, rnd(), "nop", -1, cls, 0x100),
		new MthdNotify(opt, rnd(), "notify", -1, cls, 0x104),
		new MthdWarning(opt, rnd(), "warning", -1, cls, 0x108),
		new MthdState(opt, rnd(), "state", -1, cls, 0x10c),
		new MthdSync(opt, rnd(), "sync", -1, cls, 0x110),
		new MthdFlipSet(opt, rnd(), "flip_write", -1, cls, 0x120, 1, 1),
		new MthdFlipSet(opt, rnd(), "flip_read", -1, cls, 0x124, 1, 0),
		new MthdFlipSet(opt, rnd(), "flip_modulo", -1, cls, 0x128, 1, 2),
		new MthdFlipBumpWrite(opt, rnd(), "flip_bump_write", -1, cls, 0x12c, 1),
		new UntestedMthd(opt, rnd(), "flip_unk130", -1, cls, 0x130),
		new MthdPmTrigger(opt, rnd(), "pm_trigger", -1, cls, 0x140),
		new MthdDmaNotify(opt, rnd(), "dma_notify", -1, cls, 0x180),
		new UntestedMthd(opt, rnd(), "dma_tex_a", -1, cls, 0x184), // XXX
		new UntestedMthd(opt, rnd(), "dma_tex_b", -1, cls, 0x188), // XXX
		new MthdDmaSurf(opt, rnd(), "dma_surf_color_b", -1, cls, 0x18c, 6, SURF_NV10),
		new UntestedMthd(opt, rnd(), "dma_state", -1, cls, 0x190), // XXX
		new MthdDmaSurf(opt, rnd(), "dma_surf_color_a", -1, cls, 0x194, 2, SURF_NV10),
		new MthdDmaSurf(opt, rnd(), "dma_surf_zeta", -1, cls, 0x198, 3, SURF_NV10),
		new UntestedMthd(opt, rnd(), "dma_vtx_a", -1, cls, 0x19c), // XXX
		new UntestedMthd(opt, rnd(), "dma_vtx_b", -1, cls, 0x1a0), // XXX
		new UntestedMthd(opt, rnd(), "dma_fence", -1, cls, 0x1a4), // XXX
		new UntestedMthd(opt, rnd(), "dma_query", -1, cls, 0x1a8), // XXX
		new UntestedMthd(opt, rnd(), "dma_clipid", -1, cls, 0x1ac), // XXX
		new UntestedMthd(opt, rnd(), "dma_zcull", -1, cls, 0x1b0), // XXX
		new UntestedMthd(opt, rnd(), "clip_h", -1, cls, 0x200), // XXX
		new UntestedMthd(opt, rnd(), "clip_v", -1, cls, 0x204), // XXX
		new MthdSurf3DFormat(opt, rnd(), "surf_format", -1, cls, 0x208, true),
		new MthdSurfPitch2(opt, rnd(), "surf_pitch_2", -1, cls, 0x20c, 2, 3, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "color_offset", -1, cls, 0x210, 2, SURF_NV10),
		new MthdSurfOffset(opt, rnd(), "zeta_offset", -1, cls, 0x214, 3, SURF_NV10),
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x218, 2), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x220, 8), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x240, 0x10), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x280, 0x20), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x300, 0x40), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x400, 0x100), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x800, 0x200), // XXX
		new UntestedMthd(opt, rnd(), "meh", -1, cls, 0x1000, 0x400), // XXX
	};
	return res;
}

}
}