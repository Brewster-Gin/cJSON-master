#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "cJSON.h"

void test_pretty_print() {
    printf("========================================\n");
    printf("cJSON 美化打印功能测试\n");
    printf("========================================\n\n");

    /* 创建根对象 */
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        printf("创建根对象失败！\n");
        return;
    }
    
    /* 添加基本类型 */
    cJSON_AddStringToObject(root, "name", "张三");
    cJSON_AddNumberToObject(root, "age", 25);
    cJSON_AddBoolToObject(root, "is_student", 1);
    cJSON_AddNullToObject(root, "extra");
    
    /* 创建地址对象 */
    cJSON *address = cJSON_CreateObject();
    if (address) {
        cJSON_AddStringToObject(address, "city", "北京");
        cJSON_AddStringToObject(address, "district", "朝阳区");
        cJSON_AddNumberToObject(address, "zip", 100000);
        
        /* 关键：将address对象添加到root对象中 */
        cJSON_AddItemToObject(root, "address", address);
    } else {
        printf("创建address对象失败！\n");
    }
    
    /* 创建爱好数组 */
    cJSON *hobbies = cJSON_CreateArray();
    if (hobbies) {
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("读书"));
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("编程"));
        cJSON_AddItemToArray(hobbies, cJSON_CreateString("游泳"));
        
        /* 将hobbies数组添加到root对象中 */
        cJSON_AddItemToObject(root, "hobbies", hobbies);
    } else {
        printf("创建hobbies数组失败！\n");
    }

    /* 验证root对象是否包含数据 */
    if (root->child == NULL) {
        printf("警告：root对象为空！\n");
    } else {
        printf("root对象包含数据，开始打印...\n\n");
    }

    /* 测试普通打印 */
    printf("普通打印:\n");
    char *normal = cJSON_Print(root);
    if (normal) {
        printf("%s\n\n", normal);
        free(normal);
    } else {
        printf("普通打印返回NULL\n");
    }

    /* 测试美化打印 */
    printf("美化打印:\n");
    char *pretty = cJSON_PrintPretty(root);
    if (pretty) {
        printf("%s\n", pretty);
        free(pretty);
    } else {
        printf("美化打印返回NULL\n");
    }

    cJSON_Delete(root);
}

int main() {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    printf("程序开始运行...\n");
    printf("当前工作目录: ");
    system("cd");  // 显示当前目录
    
    test_pretty_print();
    
    printf("\n按任意键退出...\n");
    getchar();
    
    return 0;
}