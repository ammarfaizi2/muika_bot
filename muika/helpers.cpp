// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

namespace muika {

void ____pr_debug(const char *fmt, ...)
{
	char *ptr, *heap = nullptr;
	va_list ap, ap2;
	char buf[1024];
	int ret;

	ptr = buf;

	va_start(ap, fmt);
	va_copy(ap2, ap);
	ret = vsnprintf(buf, sizeof(buf), fmt, ap);

	if (ret >= (int)sizeof(buf)) {
		heap = (char *)malloc(ret + 1);
		if (heap) {
			ptr = heap;
			vsnprintf(heap, ret + 1, fmt, ap2);
		}
	}
	va_end(ap2);
	va_end(ap);

	printf("debug: %s\n", ptr);

	if (heap)
		free(heap);
}

} /* namespace muika */
