/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef MLOGGER__MLOGGER_H
#define MLOGGER__MLOGGER_H

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mk_log_level {
	MK_LOG_DEBUG = 0,
	MK_LOG_INFO  = 1,
	MK_LOG_WARN  = 2,
	MK_LOG_ERROR = 3,
	MK_LOG_FATAL = 4,
};

typedef struct mk_logger mk_logger_t;

/*
 * Construct a logger that writes to the file at @log_file. The file is
 * opened in append mode and owned by the logger (closed by
 * mk_logger_free()). Returns NULL on error and sets errno.
 */
mk_logger_t *mk_logger_init(const char *log_file);

/*
 * Construct a logger that writes to an already-open FILE *. The caller
 * retains ownership of the FILE and is responsible for closing it after
 * mk_logger_free().
 */
mk_logger_t *mk_logger_init_file(FILE *log_file);

void mk_logger_free(mk_logger_t *l);
void mk_logger_flush(mk_logger_t *l);
void mk_logger_set_level(mk_logger_t *l, int level);
int  mk_logger_get_level(mk_logger_t *l);
int  mk_logger_should_log(mk_logger_t *l, int level);

void mk_log(mk_logger_t *l, int level, const char *fmt, ...)
	__attribute__((format(printf, 3, 4)));
void mk_vlog(mk_logger_t *l, int level, const char *fmt, va_list ap)
	__attribute__((format(printf, 3, 0)));

void mk_debug(mk_logger_t *l, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void mk_info(mk_logger_t *l, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void mk_warn(mk_logger_t *l, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void mk_error(mk_logger_t *l, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void mk_fatal(mk_logger_t *l, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MLOGGER__MLOGGER_H */
