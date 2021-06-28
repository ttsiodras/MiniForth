#ifndef __HELPERS_H__
#define __HELPERS_H__

typedef enum SuccessOrFailure {
    FAILURE,
    SUCCESS
} SuccessOrFailure;

void memory_info();
void dprintf(const char *fmt, ...);
SuccessOrFailure error(const char *msg);
SuccessOrFailure error(const char *msg, const char *data);

#endif
