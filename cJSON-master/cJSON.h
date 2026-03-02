/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* ******************************* 平台与符号导出配置 ******************************* */
/* 代码块注释：处理 Windows 和 Linux 等不同平台下的动态库符号导出/导入问题。
 * 通过定义不同的宏来控制 CJSON_PUBLIC，确保函数在编译为 DLL 时被导出，
 * 在使用 DLL 时被导入，而在静态编译或 Linux 下则无需特殊处理。
 */

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__ /* Windows 平台下的调用约定和符号导出定义 */

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the CJSON_API_VISIBILITY flag to "export" the same symbols the way CJSON_EXPORT_SYMBOLS does

*/

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall
 /* 默认行为是导出符号，以便于拷贝粘贴 .c 和 .h 文件进行编译 */
/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
 /* 非 Windows 平台 (Linux, macOS等) */
#define CJSON_CDECL
#define CJSON_STDCALL

/* 关键行注释：使用 GCC 或 Sun Studio 编译器的可见性属性，如果定义了 CJSON_API_VISIBILITY，
则将符号标记为 "default"（公开可见），否则隐藏。这有助于减小动态库的大小并避免符号冲突。
 */
#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
/* 项目版本号 */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 19

#include <stddef.h>/* 提供 size_t 定义 */

/* cJSON Types: */
/* ******************************* cJSON 类型定义 ******************************* */
/* 关键行注释：使用位掩码定义类型，允许一个节点同时具有多种属性（如引用和类型）。
 * 这也是为什么类型检查需要使用按位与 (&) 而不是直接等于 (==) 的原因。
 */
#define cJSON_Invalid (0)/* 无效类型 */
#define cJSON_False  (1 << 0)/* false */
#define cJSON_True   (1 << 1)/* true */
#define cJSON_NULL   (1 << 2) /* null */
#define cJSON_Number (1 << 3)/* 数字 */
#define cJSON_String (1 << 4) /* 字符串 */
#define cJSON_Array  (1 << 5)/* 数组 */
#define cJSON_Object (1 << 6)/* 对象 */
#define cJSON_Raw    (1 << 7) /* 原始 JSON 字符串，不进行转义处理 */

/* 关键行注释：附加属性位，用于内存管理优化。IsReference 表示此节点是另一个节点的引用，
 * 因此 cJSON_Delete 不应释放其子节点。StringIsConst 表示 string 成员指向常量区，
 * 不应由 cJSON 释放。
 */
#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
/* cJSON 核心结构体，代表一个 JSON 节点 */
typedef struct cJSON
{
    /* 代码块注释：用于构建双向链表，遍历数组或对象的所有子项。
    * 数组或对象的第一个节点的 prev 为 NULL，最后一个节点的 next 为 NULL。
     */
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* 代码块注释：如果当前节点是数组或对象，child 指向其包含的第一个子节点。
     * 通过 child -> next -> next ... 可以遍历所有子项。
     */
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* 节点的类型掩码，可以是上述类型（cJSON_Number 等）和附加属性（cJSON_IsReference）的组合 */
    /* The type of the item, as above. */
    int type;

     /* 节点值为字符串时使用，指向实际的字符串内容 */
    char *valuestring;

    /* 警告：valueint 已被废弃，为了保持二进制兼容性而保留。始终使用 valuedouble 并通过
     * cJSON_SetNumberValue 宏或 cJSON_SetNumberHelper 函数设置数值。 */
    int valueint;

    /* The item's number, if type==cJSON_Number */
    /* 节点值为数字时使用，存储双精度浮点数 */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
      /* 如果此节点是对象的一个键值对，string 指向键名 */
    char *string;
} cJSON;

/* 内存钩子结构体，允许用户自定义内存管理函数 */
typedef struct cJSON_Hooks
{
     /* 关键行注释：在 Windows 上，无论编译器的默认调用约定是什么，malloc/free 都是 CDECL。
     * 因此钩子函数也必须显式指定为 CDECL，以确保可以传递标准的 malloc/free。
     */
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(CJSON_CDECL *malloc_fn)(size_t sz);
      void (CJSON_CDECL *free_fn)(void *ptr);
} cJSON_Hooks;

typedef int cJSON_bool;/* 使用 int 作为布尔类型，1 表示 true，0 表示 false */

/* Limits how deeply nested arrays/objects can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
/* 安全限制，防止解析超深层级的 JSON，导致栈溢出 */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* Limits the length of circular references can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
/* 限制循环引用的检测深度 */
#ifndef CJSON_CIRCULAR_LIMIT
#define CJSON_CIRCULAR_LIMIT 10000
#endif

/* ******************************* 核心 API 函数声明 ******************************* */

/* 函数级注释：获取 cJSON 库的版本字符串。
 * @return 指向版本字符串的指针，该字符串在静态内存中，不需要释放。
 */
/* returns the version of cJSON as a string */
CJSON_PUBLIC(const char*) cJSON_Version(void);

/* 函数级注释：初始化 cJSON 的内存管理钩子。
 * @param hooks 指向 cJSON_Hooks 结构体的指针，包含自定义的 malloc/free 函数。
 *               如果传入 NULL，则恢复使用标准 C 库的 malloc/free。
 * @note 此函数应在任何其他 cJSON 函数之前调用，以设置全局内存分配器。
 */
/* Supply malloc, realloc and free functions to cJSON */
CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks);

/* 函数级注释：解析 JSON 字符串并构建 cJSON 树。
 * @param value 以 null 结尾的 JSON 字符串。
 * @return 成功返回指向根节点的指针，失败返回 NULL。
 * @note 返回的根节点需要在使用后调用 cJSON_Delete() 手动释放。
 */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value);

/* 函数级注释：解析指定长度的 JSON 字符串，不依赖 null 终止符。
 * @param value JSON 字符串指针。
 * @param buffer_length 字符串的最大解析长度。
 * @return 成功返回指向根节点的指针，失败返回 NULL。
 * @note 用于解析可能不包含 null 终止符的 JSON 数据块。
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length);

/* 函数级注释：带选项的解析函数。
 * @param value 以 null 结尾的 JSON 字符串。
 * @param return_parse_end 如果不为 NULL，函数会将解析停止的位置写入此指针。
 *                          如果解析失败，此指针指向错误发生的大致位置。
 * @param require_null_terminated 是否要求 JSON 字符串在末尾有 null 终止符。
 * @return 成功返回根节点，失败返回 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);

/* 函数级注释：带选项且指定长度的解析函数。
 * @param value JSON 字符串指针。
 * @param buffer_length 字符串的最大解析长度。
 * @param return_parse_end 同上。
 * @param require_null_terminated 同上。
 * @return 成功返回根节点，失败返回 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated);

/* 函数级注释：将 cJSON 树渲染为格式化的 JSON 字符串（带缩进和换行）。
 * @param item 要渲染的 cJSON 节点。
 * @return 成功返回一个动态分配的字符串，需要调用者使用 cJSON_free() 释放。
 *         失败返回 NULL。
 */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item);

/* 函数级注释：将 cJSON 树渲染为无格式的 JSON 字符串（紧凑模式）。
 * @param item 要渲染的 cJSON 节点。
 * @return 成功返回一个动态分配的字符串，需要调用者使用 cJSON_free() 释放。
 *         失败返回 NULL。
 */
CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item);

/* 函数级注释：使用缓冲策略渲染 JSON 字符串，减少内存分配次数。
 * @param item 要渲染的 cJSON 节点。
 * @param prebuffer 预估的缓冲区大小。预估准确可减少 realloc 调用。
 * @param fmt 格式标志，1 为格式化输出，0 为无格式输出。
 * @return 成功返回动态分配的字符串，需要调用者使用 cJSON_free() 释放。
 *         失败返回 NULL。
 */
CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);

/* 函数级注释：将 cJSON 树渲染到预先分配的缓冲区中。
 * @param item 要渲染的 cJSON 节点。
 * @param buffer 目标缓冲区指针。
 * @param length 缓冲区长度。
 * @param format 格式标志，1 为格式化，0 为无格式。
 * @return 成功返回 1，缓冲区不足或其他错误返回 0。
 * @note 此函数不会动态分配内存，调用者全权负责缓冲区管理。
 *       由于 cJSON 对所需长度的估算可能不完全准确，建议分配比预估多 5 字节。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);

/* 函数级注释：递归删除整个 cJSON 树，释放所有相关内存。
 * @param item 要删除的根节点。
 * @note 如果 item 为 NULL，函数直接返回，不执行任何操作。
 */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item);

/* 函数级注释：获取数组或对象中元素的数量。
 * @param array 数组或对象节点。
 * @return 元素个数。如果输入为 NULL 或不是数组/对象，返回 0。
 */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array);

/* 函数级注释：通过索引从数组中获取子项。
 * @param array 数组节点。
 * @param index 要获取的元素的索引（从 0 开始）。
 * @return 指向子项的指针，如果索引越界或输入无效，返回 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);

/* 函数级注释：从对象中获取指定名称的项（不区分大小写）。
 * @param object 对象节点。
 * @param string 键名。
 * @return 指向匹配的子项的指针，如果未找到或输入无效，返回 NULL。
 * @deprecated 推荐使用 cJSON_GetObjectItemCaseSensitive 以避免歧义。
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);

/* 函数级注释：从对象中获取指定名称的项（区分大小写）。
 * @param object 对象节点。
 * @param string 键名。
 * @return 指向匹配的子项的指针，如果未找到或输入无效，返回 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);

/* 函数级注释：检查对象中是否存在指定名称的项（不区分大小写）。
 * @param object 对象节点。
 * @param string 键名。
 * @return 存在返回 1，不存在或输入无效返回 0。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);

/* 函数级注释：获取解析错误的位置。
 * @return 指向错误位置的指针。当 cJSON_Parse 系列函数返回 NULL 时，
 *         此函数可用于获取错误信息。如果最后一次解析成功，则返回 NULL。
 */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* 函数级注释：获取节点的字符串值。
 * @param item 目标节点。
 * @return 如果节点是字符串类型，返回 valuestring 指针；否则返回 NULL。
 */
CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item);

/* 函数级注释：获取节点的数字值。
 * @param item 目标节点。
 * @return 如果节点是数字类型，返回 valuedouble；否则返回 0。
 */
CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item);

/* ******************************* 类型检查函数 ******************************* */
/* 函数级注释：以下函数用于检查节点的具体类型。它们通过按位与操作检查 type 成员。
 * 如果节点为 NULL，所有检查均返回 0 (false)。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item);/* 检查是否为 true 或 false */
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item);

/* ******************************* 创建节点函数 ******************************* */
/* 函数级注释：以下函数用于创建各种类型的 cJSON 节点。返回的节点由调用者负责管理，
 * 不再需要时需通过 cJSON_Delete 释放。
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool boolean);/* 根据 boolean 创建 true 或 false */
CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num);
CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string);/* 内部会拷贝字符串 */
/* raw json */
CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw);/* 创建原始 JSON 节点，内部会拷贝 raw 字符串 */
CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void);

/* 函数级注释：创建字符串引用节点。节点内的 valuestring 直接指向传入的字符串，
 * 不会进行拷贝。这意味着传入的字符串必须在整个节点生命周期内有效，且不会被 cJSON_Delete 释放。
 * @param string 指向常量或外部管理字符串的指针。
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string);
/* 函数级注释：创建对象/数组引用节点。新节点不会拷贝子节点，而是引用已有的子节点。
 * @param child 要被引用的子节点。
 * @return 新的引用节点，其 child 指针指向传入的 child。
 * @note 当删除引用节点时，不会删除其引用的子节点。
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateObjectReference(const cJSON *child);
CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child);

/* ******************************* 批量创建数组函数 ******************************* */
/* 函数级注释：根据传入的数组批量创建 cJSON 数组节点。
 * @param numbers 源数组。
 * @param count 元素个数。
 * @return 新创建的 cJSON 数组节点，失败返回 NULL。
 * @warning count 不能大于 numbers 数组的实际长度，否则会访问越界。
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char *const *strings, int count);/* 内部会拷贝字符串 */

/* ******************************* 添加节点到数组/对象 ******************************* */
/* 函数级注释：将 item 追加到数组的末尾。
 * @param array 目标数组。
 * @param item 要添加的节点。
 * @return 成功返回 1，失败返回 0。
 * @note 添加成功后，item 的生命周期由 array 管理，不应再单独释放 item。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON *array, cJSON *item);

/* 函数级注释：将 item 作为键值对添加到对象中。
 * @param object 目标对象。
 * @param string 键名（内部会拷贝该字符串）。
 * @param item 值节点。
 * @return 成功返回 1，失败返回 0。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);

/* 函数级注释：将 item 作为键值对添加到对象中，但键名使用常量字符串。
 * @param object 目标对象。
 * @param string 常量键名（不会拷贝，直接引用）。
 * @param item 值节点。
 * @return 成功返回 1，失败返回 0。
 * @warning 确保 string 指针在 object 的生命周期内始终有效。在使用此函数后，
 *          如果要修改 item->string，必须先检查 (item->type & cJSON_StringIsConst) 是否为 0。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);

/* 函数级注释：将 item 的引用添加到数组/对象中。
 * @param array/object 目标容器。
 * @param item 被引用的节点。
 * @return 成功返回 1，失败返回 0。
 * @note 这相当于创建一个引用节点并添加，容器不会获得 item 的所有权。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item);

/* ******************************* 移除/删除节点 ******************************* */
/* 函数级注释：从父节点中分离指定的子节点。
 * @param parent 父节点（数组或对象）。
 * @param item 要分离的子节点指针。
 * @return 返回被分离的节点（即 item 本身），调用者现在负责释放它。
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item);

/* 函数级注释：通过索引从数组中分离节点。
 * @param array 数组节点。
 * @param which 索引。
 * @return 返回被分离的节点，调用者现在负责释放它。如果失败返回 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which);

/* 函数级注释：通过索引从数组中删除并释放节点。
 * @param array 数组节点。
 * @param which 索引。
 * @note 此函数会直接释放节点内存，无需调用者额外操作。
 */
CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which);

/* 函数级注释：通过键名从对象中分离节点（不区分大小写/区分大小写）。
 * @param object 对象节点。
 * @param string 键名。
 * @return 返回被分离的节点，调用者负责释放。
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* 函数级注释：通过键名从对象中删除并释放节点。
 */
CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* ******************************* 插入/替换节点 ******************************* */
/* 函数级注释：在数组的指定位置插入新节点，原有节点及其后节点向右移动。
 * @param array 数组节点。
 * @param which 插入位置索引（0 表示插入到开头）。
 * @param newitem 新节点。
 * @return 成功返回 1，失败返回 0。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem); /* Shifts pre-existing items to the right. */

/* 函数级注释：通过指针直接替换子节点。
 * @param parent 父节点。
 * @param item 要被替换的旧子节点指针。
 * @param replacement 新节点。
 * @return 成功返回 1，失败返回 0。
 * @note 替换后，旧节点被分离并释放，新节点被添加。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement);

/* 函数级注释：通过索引替换数组中的节点。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);

/* 函数级注释：通过键名替换对象中的节点。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,const char *string,cJSON *newitem);

/* ******************************* 复制与比较 ******************************* */
/* 函数级注释：递归复制一个 cJSON 节点。
 * @param item 要复制的节点。
 * @param recurse 如果为非 0，则同时复制其所有子节点；如果为 0，只复制节点本身，不复制 child。
 * @return 返回新的节点副本，需要调用者使用 cJSON_Delete 释放。
 * @note 返回的新节点的 next 和 prev 指针始终为 NULL。
 */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse);

/* 函数级注释：递归比较两个 cJSON 节点是否相等。
 * @param a, b 要比较的节点。
 * @param case_sensitive 比较对象键名时是否区分大小写。
 * @return 如果相等返回 1，否则返回 0。如果 a 或 b 为 NULL，也返回 0。
 */
CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive);

/* ******************************* 工具函数 ******************************* */
/* 函数级注释：压缩 JSON 字符串，移除所有空白字符（空格、制表符、换行符、回车符）。
 * @param json 要压缩的字符串。此操作是原地修改的，因此字符串必须是可写的。
 * @warning 输入指针不能指向只读常量区，否则会导致程序崩溃。
 */
CJSON_PUBLIC(void) cJSON_Minify(char *json);

/* ******************************* 便捷添加函数 ******************************* */
/* 函数级注释：以下是一系列便捷函数，用于在创建值的同时将其添加到对象中。
 * 它们相当于先调用 cJSON_CreateXxx，再调用 cJSON_AddItemToObject。
 * @param object 目标对象。
 * @param name 键名。
 * @param ... 值参数。
 * @return 成功返回新创建的值的节点指针，失败返回 NULL。
 */
CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw);
CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);/* 创建空对象并添加 */
CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON * const object, const char * const name);/* 创建空数组并添加 */

/* ******************************* 辅助宏与内联函数 ******************************* */
/* 函数级注释：同时设置节点的整数和双精度值。推荐使用此宏来设置数值，以避免 valueint 和 valuedouble 不一致。
 * @param object 目标节点。
 * @param number 整数值。
 */
#define cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))

/* 函数级注释：辅助函数，用于设置节点的双精度值。
 * @param object 目标节点。
 * @param number 双精度值。
 * @return 返回设置后的值。
 */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number);

/* 函数级注释：设置节点的数值。可以接受整数或浮点数，内部调用 cJSON_SetNumberHelper。
 */
#define cJSON_SetNumberValue(object, number) ((object != NULL) ? cJSON_SetNumberHelper(object, (double)number) : (number))

/* 函数级注释：更改 cJSON_String 类型节点的 valuestring。
 * @param object 目标节点（必须是字符串类型）。
 * @param valuestring 新的字符串值（内部会进行拷贝）。
 * @return 返回指向新字符串的指针，失败返回 NULL。
 */
CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON *object, const char *valuestring);

/* 函数级注释：设置布尔类型节点的值。
 * @param object 目标节点。
 * @param boolValue 要设置的布尔值。
 * @return 如果成功（节点是布尔类型），返回节点的新类型值；否则返回 cJSON_Invalid。
 */
#define cJSON_SetBoolValue(object, boolValue) ( \
    (object != NULL && ((object)->type & (cJSON_False|cJSON_True))) ? \
    (object)->type=((object)->type &(~(cJSON_False|cJSON_True)))|((boolValue)?cJSON_True:cJSON_False) : \
    cJSON_Invalid\
)

/* 宏级注释：便捷宏，用于遍历数组或对象的所有子项。
 * @param element 循环变量，每次迭代指向当前子项。
 * @param array 要遍历的数组或对象节点。
 * 示例: cJSON *child; cJSON_ArrayForEach(child, parent_array) { ... }
 */
#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* 函数级注释：使用已设置的钩子函数分配内存。如果未设置钩子，则使用标准 malloc。
 * @param size 要分配的大小。
 * @return 分配的内存指针，失败返回 NULL。
 */
CJSON_PUBLIC(void *) cJSON_malloc(size_t size);

/* 函数级注释：使用已设置的钩子函数释放内存。如果未设置钩子，则使用标准 free。
 * @param object 要释放的内存指针。
 */
CJSON_PUBLIC(void) cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
