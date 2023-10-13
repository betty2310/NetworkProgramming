#include "crawler.h"
#include <stdio.h>

int
crawl(char* url, char* filename) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Create and open a file for writing
        FILE* htmlFile = fopen(filename, "w");
        if (!htmlFile) {
            fprintf(stderr, "Failed to open the output file.\n");
            return 1;
        }

        // Set the file as the output stream
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, htmlFile);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            fclose(htmlFile);
            return 1;
        }

        fclose(htmlFile);   // Close the output file
        curl_easy_cleanup(curl);
    }
    return 0;
}