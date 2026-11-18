#include "tcp.h"

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	static const struct {
		uint32_t value;
		const char *name;
	} cases[] = {
		{ MIB_TCP_STATE_CLOSED, "CLOSED" },
		{ MIB_TCP_STATE_LISTEN, "LISTEN" },
		{ MIB_TCP_STATE_SYN_SENT, "SYN_SENT" },
		{ MIB_TCP_STATE_SYN_RCVD, "SYN_RCVD" },
		{ MIB_TCP_STATE_ESTAB, "ESTABLISHED" },
		{ MIB_TCP_STATE_FIN_WAIT1, "FIN_WAIT1" },
		{ MIB_TCP_STATE_FIN_WAIT2, "FIN_WAIT2" },
		{ MIB_TCP_STATE_CLOSE_WAIT, "CLOSE_WAIT" },
		{ MIB_TCP_STATE_CLOSING, "CLOSING" },
		{ MIB_TCP_STATE_LAST_ACK, "LAST_ACK" },
		{ MIB_TCP_STATE_TIME_WAIT, "TIME_WAIT" },
		{ MIB_TCP_STATE_DELETE_TCB, "DELETE_TCB" },
		{ UINT32_MAX, "UNKNOWN" }
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
		const char *actual = conntrace_tcp_state_name(cases[i].value);
		if (strcmp(actual, cases[i].name) != 0) {
			fprintf(stderr, "State %lu: expected %s, got %s\n",
					(unsigned long)cases[i].value, cases[i].name, actual);
			return 1;
		}
	}

	puts("TCP state-name tests passed.");
	return 0;
}
