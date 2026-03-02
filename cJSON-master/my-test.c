#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

int main() {
    // 创建一个简单的JSON对象
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        printf("Failed to create JSON object\n");
        return 1;
    }
    
    cJSON_AddStringToObject(root, "message", "Hello, cJSON!");
    
    // 将JSON对象转换为字符串
    char *json_str = cJSON_Print(root);
    if (json_str != NULL) {
        printf("JSON: %s\n", json_str);
        free(json_str);
    }
    
    // 释放内存
    cJSON_Delete(root);
    
    printf("程序执行成功！按回车键退出...\n");
    getchar();  // 等待用户按回车
    return 0;
}