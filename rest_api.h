#include <curl/curl.h>
#include "parson.h"

// TODO: move to config

char *SERVER_URL = "";
char *SERVER_TOKEN = "";

int rest_api_vsca(const char *imsi, int bytes_in, int bytes_out);