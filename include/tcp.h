#ifndef CONNTRACE_TCP_H
#define CONNTRACE_TCP_H

#include <stdint.h>

const char *conntrace_tcp_state_name(uint32_t state);
int conntrace_print_tcp_snapshot(void);

#endif
