#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <stdio.h>
#include <sys/types.h>

void
extractLinks(xmlNode* a_node, FILE* csvFile) {
    for (xmlNode* cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            xmlChar* href = xmlGetProp(cur_node, (const xmlChar*) "href");
            if (href) {
                fprintf(csvFile, "%s\n", href);
                xmlFree(href);
            }
        }

        extractLinks(cur_node->children, csvFile);
    }
}

int
main(int argc, char** argv) {
    char* url = argv[1];
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Create and open a file for writing
        FILE* htmlFile = fopen("file.html", "w");
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

    xmlDoc* doc;
    xmlNode* root_element;

    doc = htmlReadFile("file.html", NULL, XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse document.\n");
        return 1;
    }

    FILE* csvFile = fopen("links.csv", "w");
    if (csvFile == NULL) {
        fprintf(stderr, "Failed to create CSV file.\n");
        return 1;
    }

    root_element = xmlDocGetRootElement(doc);
    extractLinks(root_element, csvFile);

    // Cleanup
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
