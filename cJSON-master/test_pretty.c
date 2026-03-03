#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "cJSON.h"

void test_pretty_print() {
    printf("========================================\n");
    printf("cJSON 美化打印功能测试\n");
    printf("========================================\n\n");

    /* 创建测试数据 */
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        printf("创建根对象失败！\n");
        return;
    }
    
    cJSON_AddStringToObject(root, "name", "张三");
    cJSON_AddNumberToObject(root, "age", 25);
    cJSON_AddBoolToObject(root, "is_student", 1);
    cJSON_AddNullToObject(root, "extra");
    
    cJSON *address = cJSON_CreateObject();
    if (address) {
        cJSON_AddStringToObject(address, "city", "北京");
        cJSON_AddStringToObject(address, "district", "朝阳区");
        cJSON_AddNumberToObject(address, "zip", 100000);
        cJSON_AddItemToObject(root, "address", address);
    }
    
    cJSON *hobbies = cJSON_CreateArray();
    if (hobbies) {
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("读书"));
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("编程"));
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("游泳"));
        cJSON_AddItemToObject(root, "hobbies", hobbies);
    }

    /* 只测试美化打印，不测试普通打印 */
    printf("美化打印:\n");
    char *pretty = cJSON_PrintPretty(root);
    if (pretty) {
        printf("%s\n", pretty);
        free(pretty);
    } else {
        printf("美化打印失败！\n");
    }

    cJSON_Delete(root);
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    test_pretty_print();
    
    printf("\n按任意键退出...\n");
    getchar();
    
    return 0;
}