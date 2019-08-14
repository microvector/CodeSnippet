#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void parseSchool(xmlDocPtr doc, xmlNodePtr cur) {
    xmlChar *element;
    xmlChar *param;
    cur = cur->xmlChildrenNode;
    while (cur != nullptr) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)("phoneNumber")))) {
            element = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("phoneNumber: %s\n", element);
            xmlFree(element);
        }

        if ((!xmlStrcmp(cur->name, (const xmlChar *)("schoolmaster")))) {
            element = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("schoolmaster: %s\n", element);
            xmlFree(element);
        }

        if ((!xmlStrcmp(cur->name, (const xmlChar *)("address")))) {
            element = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("address: %s\n", element);

            // 获取标签的参数
            param = xmlGetProp(cur, (const xmlChar *)("part"));
            printf("param: %s\n", param);

            xmlFree(element);
            xmlFree(param);
        }
        cur = cur->next;
    }
    return;
}

void parseCompany(xmlDocPtr doc, xmlNodePtr cur) {
    xmlChar *element;
    xmlChar *param;
    cur = cur->xmlChildrenNode;
    while (cur != nullptr) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)("name")))) {
            element = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("name = %s\n", element);
            xmlFree(element);
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)("address")))) {
            element = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("address = %s\n", element);

            // 获取标签的参数
            param = xmlGetProp(cur, (const xmlChar *)("part"));
            printf("param: %s\n", param);

            xmlFree(element);
            xmlFree(param);
        }
        cur = cur->next;
    }
    return;
}
static void parseDoc(char *docname) {
    xmlDocPtr doc;   // 文档的树结构
    xmlNodePtr cur;  // 当前结构指针
    // 获取文档的树结构
    doc = xmlParseFile(docname);
    if (doc == nullptr) {
        fprintf(stderr, "Document not parsed successfully. \n");
        return;
    }
    // 获得指向这个结构的指针
    cur = xmlDocGetRootElement(doc);
    if (cur == nullptr) {
        fprintf(stderr, "empty document\n");
        xmlFreeDoc(doc);
        return;
    }
    // 确认文档的根类型是不是和我们定义的一致
    if (xmlStrcmp(cur->name, (const xmlChar *)("school"))) {
        fprintf(stderr, "document of the wrong type, root node != School");
        xmlFreeDoc(doc);
        return;
    }

    // 取得第一个子节点指针
    cur = cur->xmlChildrenNode;
    while (cur != nullptr) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)("schoolinfo")))) {
            printf("------------------parse school------------------------- \n");
            parseSchool(doc, cur);
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)("companyinfo")))) {
            printf("-------------------parse company------------------------ \n");
            parseCompany(doc, cur);
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    return;
}
int main(int argc, char **argv) {
    char *docname;
    if (argc <= 1) {
        printf("Usage: %s docname\n", argv[0]);
        return (0);
    }
    docname = argv[1];
    parseDoc(docname);
    return (1);
}