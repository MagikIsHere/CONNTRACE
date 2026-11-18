#include "conntrace.h"
#include "tcp.h"

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void report_windows_error(const char *operation, DWORD error)
{
	char message[512];
	DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								  NULL, error, 0, message, (DWORD)sizeof(message), NULL);

	if (length == 0) {
		fprintf(stderr, "%s failed (Windows error %lu).\n", operation, (unsigned long)error);
		return;
	}

	while (length > 0 && (message[length - 1] == '\r' || message[length - 1] == '\n')) {
		message[--length] = '\0';
	}
	fprintf(stderr, "%s failed: %s (Windows error %lu).\n",
			operation, message, (unsigned long)error);
}

static int print_ipv4_address(DWORD address_value, char output[INET_ADDRSTRLEN])
{
	IN_ADDR address;

	address.S_un.S_addr = address_value;
	if (InetNtopA(AF_INET, &address, output, INET_ADDRSTRLEN) == NULL) {
		report_windows_error("InetNtopA", GetLastError());
		return -1;
	}
	return 0;
}

const char *conntrace_tcp_state_name(uint32_t state)
{
	switch (state) {
	case MIB_TCP_STATE_CLOSED: return "CLOSED";
	case MIB_TCP_STATE_LISTEN: return "LISTEN";
	case MIB_TCP_STATE_SYN_SENT: return "SYN_SENT";
	case MIB_TCP_STATE_SYN_RCVD: return "SYN_RCVD";
	case MIB_TCP_STATE_ESTAB: return "ESTABLISHED";
	case MIB_TCP_STATE_FIN_WAIT1: return "FIN_WAIT1";
	case MIB_TCP_STATE_FIN_WAIT2: return "FIN_WAIT2";
	case MIB_TCP_STATE_CLOSE_WAIT: return "CLOSE_WAIT";
	case MIB_TCP_STATE_CLOSING: return "CLOSING";
	case MIB_TCP_STATE_LAST_ACK: return "LAST_ACK";
	case MIB_TCP_STATE_TIME_WAIT: return "TIME_WAIT";
	case MIB_TCP_STATE_DELETE_TCB: return "DELETE_TCB";
	default: return "UNKNOWN";
	}
}

int conntrace_print_tcp_snapshot(void)
{
	DWORD buffer_size = 0;
	DWORD result;
	void *buffer = NULL;
	PMIB_TCPTABLE_OWNER_PID table;
	time_t now;
	struct tm local_time;
	char timestamp[32];

	result = GetExtendedTcpTable(NULL, &buffer_size, FALSE, AF_INET,
								 TCP_TABLE_OWNER_PID_ALL, 0);
	if (result != ERROR_INSUFFICIENT_BUFFER) {
		report_windows_error("GetExtendedTcpTable (size query)", result);
		return -1;
	}

	for (unsigned int attempt = 0; attempt < 4U; ++attempt) {
		DWORD allocated_size = buffer_size;

		buffer = malloc((size_t)allocated_size);
		if (buffer == NULL) {
			fprintf(stderr, "Unable to allocate %lu bytes for the TCP table.\n",
					(unsigned long)allocated_size);
			return -1;
		}

		result = GetExtendedTcpTable(buffer, &allocated_size, FALSE, AF_INET,
									 TCP_TABLE_OWNER_PID_ALL, 0);
		if (result != ERROR_INSUFFICIENT_BUFFER) {
			buffer_size = allocated_size;
			break;
		}

		free(buffer);
		buffer = NULL;
		buffer_size = allocated_size;
	}

	if (buffer == NULL) {
		fprintf(stderr, "The TCP table changed repeatedly while being read; try again.\n");
		return -1;
	}
	if (result != NO_ERROR) {
		free(buffer);
		report_windows_error("GetExtendedTcpTable", result);
		return -1;
	}

	now = time(NULL);
	if (now == (time_t)-1 || localtime_s(&local_time, &now) != 0 ||
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &local_time) == 0) {
		free(buffer);
		fprintf(stderr, "Unable to format the current local time.\n");
		return -1;
	}

	table = (PMIB_TCPTABLE_OWNER_PID)buffer;
	printf("CONNTRACE v%s | TCP/IPv4 | %s | %lu connection(s)\n",
		   CONNTRACE_VERSION, timestamp, (unsigned long)table->dwNumEntries);
	printf("%-12s %-21s %-21s %s\n", "STATE", "LOCAL", "REMOTE", "PID");

	for (DWORD i = 0; i < table->dwNumEntries; ++i) {
		const MIB_TCPROW_OWNER_PID *row = &table->table[i];
		char local_address[INET_ADDRSTRLEN];
		char remote_address[INET_ADDRSTRLEN];

		if (print_ipv4_address(row->dwLocalAddr, local_address) != 0 ||
			print_ipv4_address(row->dwRemoteAddr, remote_address) != 0) {
			free(buffer);
			return -1;
		}

		printf("%-12s %s:%-5u %s:%-5u %lu\n",
			   conntrace_tcp_state_name(row->dwState),
			   local_address, (unsigned int)ntohs((u_short)row->dwLocalPort),
			   remote_address, (unsigned int)ntohs((u_short)row->dwRemotePort),
			   (unsigned long)row->dwOwningPid);
	}

	free(buffer);
	return 0;
}
