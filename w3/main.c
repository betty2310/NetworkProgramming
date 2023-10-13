#include "crawler.h"
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <stdio.h>
#include <sys/types.h>

#define HTML_FILE  "file.html"
#define CSV_OUTPUT "links.csv"

void extractLinks(xmlNode* a_node, FILE* csvFile);

int
main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <url>\nExample: ./resolver https://github.com\n",
                argv[0]);
        return 1;
    }
    char* url = argv[1];

    crawl(url, HTML_FILE);

    xmlDoc* doc;
    xmlNode* root_element;

    doc = htmlReadFile(HTML_FILE, NULL, XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse document.\n");
        return 1;
    }

    FILE* csvFile = fopen(CSV_OUTPUT, "w");
    if (csvFile == NULL) {
        fprintf(stderr, "Failed to create CSV file.\n");
        return 1;
    }

    root_element = xmlDocGetRootElement(doc);
    extractLinks(root_element, csvFile);

    xmlFreeDoc(doc);
    xmlCleanupParser();
}

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
