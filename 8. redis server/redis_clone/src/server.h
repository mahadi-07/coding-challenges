#define SET "SET"

int start_server(int port);

/* Execute one RESP command. Returns a malloc'd reply string (caller frees). */
char *exec_command(const char *request);
