// $Id: test.h 8340 2009-09-22 03:49:50Z cedric.shih $
/*
 * Copyright (c) 2007-2008 Mantaray Technology, Incorporated.
 * Rm. A407, No.18, Si Yuan Street, Taipei, 100, Taiwan.
 * Phone: +886-2-23681570. Fax: +886-2-23682417.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted without specific written permission
 * from above copyright holder.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY MANTARAY TECHNOLOGY INCORPORATED
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * MANTARAY TECHNOLOGY INCORPORATED BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */

#ifndef JSOON_LOG_H_
#define JSOON_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

enum jsoon_level {
        JSOON_TRACE, /**< Trivial step tracing log. */
        JSOON_DEBUG, /**< Debugging log. */
        JSOON_INFO, /**< Informative log. */
        JSOON_WARN, /**< Warning log. */
        JSOON_ERROR, /**< Error log. */
        JSOON_FATAL, /**< Fatal error log. */
};

typedef void (*jsoon_logger)(int level, const char *func, unsigned int line,
		const char *fmt, va_list ap);

void jsoon_set_logger(jsoon_logger logger);

#ifdef __cplusplus
}
#endif

#endif /*JSOON_LOG_H_*/
