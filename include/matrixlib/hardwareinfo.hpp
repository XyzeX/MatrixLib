#pragma once

#include <thread>

#if defined(_WIN32)
	#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
	#include <cpuid.h>
	#include <immintrin.h> // _xgetbv
#endif

namespace mat
{
	struct HardwareInfo
	{
		bool avx = false;
		bool avx2 = false;
		bool avx512f = false;
		size_t threadCount = 1;

		static HardwareInfo Detect();
	};

	inline void cpuid(int regs[4], int leaf, int subleaf)
	{
#ifdef _WIN32
		__cpuidex(regs, leaf, subleaf);
#else
		__cpuid_count(leaf, subleaf, regs[0], regs[1], regs[2], regs[3]);
#endif
	}

	inline HardwareInfo HardwareInfo::Detect()
	{
		HardwareInfo hwInfo;

		// -------------------------
		// AVX compatability
		// -------------------------

		// --- Leaf 1: basic features ---
		int regs[4];
		cpuid(regs, 1, 0);

		const int ecx1 = regs[2];
		const bool osxsave = (regs[2] & (1 << 27)) != 0;
		const bool avx_hw = (ecx1 & (1 << 28)) != 0;

		// --- OS support check (XGETBV) ---
		bool avx_os = false;
		bool avx512_os = false;

		if (osxsave && avx_hw)
		{
			const unsigned long long xcr0 = _xgetbv(0); // GCC/Clang also expose this via immintrin.h

			// XMM (bit 1) + YMM (bit 2)
			avx_os = (xcr0 & 0x6) == 0x6;

			// AVX-512 requires: opmask (5), ZMM_hi256 (6), hi16_ZMM (7)
			avx512_os = (xcr0 & 0xE0) == 0xE0;
		}

		// --- Leaf 7: extended features ---
		cpuid(regs, 7, 0);

		const int ebx7 = regs[1];
		const bool avx2_hw = (ebx7 & (1 << 5)) != 0;
		const bool avx512f_hw = (ebx7 & (1 << 16)) != 0;

		hwInfo.avx = avx_hw && avx_os;
		hwInfo.avx2 = avx2_hw && avx_os;
		hwInfo.avx512f = avx512f_hw && avx512_os;

		// -------------------------
		// Threading capabilities
		// -------------------------

		const unsigned int hc = std::thread::hardware_concurrency();
		hwInfo.threadCount = (hc == 0) ? 1 : static_cast<size_t>(hc);

		// -------------------------
		// GPU capabilities 
		// -------------------------
		
		// TBD

		return hwInfo;
	}
}
