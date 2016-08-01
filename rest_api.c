#include "rest_api.h"

struct write_buffer {
  char *memory;
  size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct write_buffer *mem = (struct write_buffer *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int rest_api_vsca(const char *imsi, int bytes_in, int bytes_out)
{
    CURL *curl;
	CURLcode return_code;
	struct curl_slist *header = NULL;
	char *auth_header[100];
	char *post_payload[100];
	struct write_buffer chunk;
	int total = 0;

    chunk.memory = malloc(1);   /* will be grown as needed by the realloc above */
  	chunk.size = 0;             /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	if(curl) {

	    // auth token

		sprintf(auth_header, "Authorization: Token %s", SERVER_TOKEN);
		header = curl_slist_append(header, auth_header);

		// post params

		sprintf(post_payload, "imsi=%s&bytes_in=%i&bytes_out=%i", imsi, bytes_in, bytes_out);

		// set opts

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_URL, SERVER_URL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_payload);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// TODO: verify sertificate
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    	// call rest api

		return_code = curl_easy_perform(curl);

		if(return_code != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(return_code));
		} else {
			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code != 200) {
				fprintf(stderr, "HTTP code %i, details: %s\n", http_code, chunk);
			} else {
				printf("Response from server: %s\n", chunk.memory);

				// parse json

				JSON_Value *json_root = json_parse_string(chunk.memory);
				JSON_Object *json_object = json_value_get_object(json_root);
				total = (int)json_object_get_number(json_object, "total");
			}

		}

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return total;
}