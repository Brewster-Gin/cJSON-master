#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

int main() {
    // 创建一个简单的JSON对象
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "message", "Hello, cJSON!");
    cJSON_AddNumberToObject(root, "version", 1.0);
    
    // 将JSON对象转换为字符串
    char *json_str = cJSON_Print(root);
    printf("生成的JSON: %s\n", json_str);
    
    // 释放内存
    cJSON_Delete(root);
    free(json_str);
    printf("cJSON 测试成功!\n");
    return 0;
}