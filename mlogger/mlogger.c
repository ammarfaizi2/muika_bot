// SPDX-License-Identifier: GPL-2.0-only
#include <mlogger/mlogger.h>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct mk_logger {
	FILE		*fp;
	pthread_mutex_t	lock;
	int		level;
	int		owns_fp;
};

static const char *level_name(int level)
{
	switch (level) {
	case MK_LOG_DEBUG: return "DEBUG";
	case MK_LOG_INFO:  return "INFO";
	case MK_LOG_WARN:  return "WARN";
	case MK_LOG_ERROR: return "ERROR";
	case MK_LOG_FATAL: return "FATAL";
	default:           return "?";
	}
}

static mk_logger_t *alloc_logger(FILE *fp, int owns_fp)
{
	mk_logger_t *l;

	l = (mk_logger_t *)calloc(1, sizeof(*l));
	if (!l)
		return NULL;

	if (pthread_mutex_init(&l->lock, NULL) != 0) {
		free(l);
		return NULL;
	}

	l->fp = fp;
	l->owns_fp = owns_fp;
	l->level = MK_LOG_DEBUG;
	return l;
}

mk_logger_t *mk_logger_init(const char *log_file)
{
	FILE *fp;
	mk_logger_t *l;

	if (!log_file) {
		errno = EINVAL;
		return NULL;
	}

	fp = fopen(log_file, "ae");
	if (!fp)
		return NULL;

	l = alloc_logger(fp, 1);
	if (!l) {
		int saved = errno;
		fclose(fp);
		errno = saved;
		return NULL;
	}
	return l;
}

mk_logger_t *mk_logger_init_file(FILE *log_file)
{
	if (!log_file) {
		errno = EINVAL;
		return NULL;
	}
	return alloc_logger(log_file, 0);
}

void mk_logger_free(mk_logger_t *l)
{
	if (!l)
		return;

	if (l->owns_fp && l->fp)
		fclose(l->fp);

	pthread_mutex_destroy(&l->lock);
	free(l);
}

void mk_logger_flush(mk_logger_t *l)
{
	if (!l || !l->fp)
		return;

	pthread_mutex_lock(&l->lock);
	fflush(l->fp);
	pthread_mutex_unlock(&l->lock);
}

void mk_logger_set_level(mk_logger_t *l, int level)
{
	if (!l)
		return;

	pthread_mutex_lock(&l->lock);
	l->level = level;
	pthread_mutex_unlock(&l->lock);
}

int mk_logger_get_level(mk_logger_t *l)
{
	int lv;

	if (!l)
		return MK_LOG_DEBUG;

	pthread_mutex_lock(&l->lock);
	lv = l->level;
	pthread_mutex_unlock(&l->lock);
	return lv;
}

int mk_logger_should_log(mk_logger_t *l, int level)
{
	if (!l)
		return 0;
	return level >= mk_logger_get_level(l);
}

static void format_timestamp(char *buf, size_t sz)
{
	struct timespec ts;
	struct tm tm;

	clock_gettime(CLOCK_REALTIME, &ts);
	localtime_r(&ts.tv_sec, &tm);
	strftime(buf, sz, "%Y-%m-%d %H:%M:%S", &tm);
}

void mk_vlog(mk_logger_t *l, int level, const char *fmt, va_list ap)
{
	char ts[32];

	if (!l || !l->fp || !fmt)
		return;

	pthread_mutex_lock(&l->lock);
	if (level < l->level) {
		pthread_mutex_unlock(&l->lock);
		return;
	}

	format_timestamp(ts, sizeof(ts));
	fprintf(l->fp, "[%s] [%s] ", ts, level_name(level));
	vfprintf(l->fp, fmt, ap);
	fputc('\n', l->fp);
	fflush(l->fp);
	pthread_mutex_unlock(&l->lock);
}

void mk_log(mk_logger_t *l, int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	mk_vlog(l, level, fmt, ap);
	va_end(ap);
}

#define DEFINE_LEVEL(name, lv) \
void name(mk_logger_t *l, const char *fmt, ...) \
{ \
	va_list ap; \
	va_start(ap, fmt); \
	mk_vlog(l, lv, fmt, ap); \
	va_end(ap); \
}

DEFINE_LEVEL(mk_debug, MK_LOG_DEBUG)
DEFINE_LEVEL(mk_info,  MK_LOG_INFO)
DEFINE_LEVEL(mk_warn,  MK_LOG_WARN)
DEFINE_LEVEL(mk_error, MK_LOG_ERROR)
DEFINE_LEVEL(mk_fatal, MK_LOG_FATAL)
