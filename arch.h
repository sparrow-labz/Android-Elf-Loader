#ifndef _ARCH_H_
#define _ARCH_H_

#if defined(OS_LINUX)
	#if defined(ARCH_ARMEL)
		#include "sysdep/linux/armel/arch.h"
	#elif defined(ARCH_ARMEL_THUMB)
		#include "sysdep/linux/armel_thumb/arch.h"
	#elif defined(ARCH_ARMEB)
		#include "sysdep/linux/armeb/arch.h"
	#elif defined(ARCH_ARMEB_THUMB)
		#include "sysdep/linux/armeb_thumb/arch.h"
	#elif defined(ARCH_ARM64)
		#include "sysdep/linux/arm64/arch.h"
	#else
		#error "No architecture specified!"
	#endif
#else
	#error "No OS specified!"
#endif
#endif // _ARCH_H_
