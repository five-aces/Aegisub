// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "libaegisub/log.h"

#include <cstdio>
#include <ctime>
#include <unistd.h>

namespace agi::log {
void EmitSTDOUT::log(SinkMessage const& sm) {
	time_t time = sm.time / 1000000000;
	tm tmtime;
	localtime_r(&time, &tmtime);

#ifdef LOG_WITH_FILE
	printf("%c %02d:%02d:%02d %-9ld <%-25s> [%s:%s:%d]  %.*s\n",
#else
	printf("%c %02d:%02d:%02d %-9ld <%-25s> [%s:%d]  %.*s\n",
#endif
		Severity_ID[sm.severity],
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		(long)(sm.time % 1000000000),
		sm.section,
#ifdef LOG_WITH_FILE
		sm.file,
#endif
		sm.func,
		sm.line,
		(int)sm.message.size(),
		sm.message.c_str());

	if (!isatty(fileno(stdout)))
		fflush(stdout);
}
}
