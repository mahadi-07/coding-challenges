#define SET "SET"
#define GET "GET"
#define EXISTS "EXISTS"

#define DEFAULT_EXPIRES_AT_MS 0 /* 0 = no expiry (plain SET) */

int start_server(int port);

/* Execute one RESP command. Returns a malloc'd reply string (caller frees). */
char *exec_command(const char *request);
