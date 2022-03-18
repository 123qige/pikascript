#include "gtest/gtest.h"
extern "C" {
#include "PikaMain.h"
#include "PikaParser.h"
#include "PikaStdLib_MemChecker.h"
#include "PikaVM.h"
#include "dataArgs.h"
#include "dataMemory.h"
#include "dataStrs.h"
#include "pikaScript.h"
#include "pika_config_gtest.h"
}

extern PikaMemInfo pikaMemInfo;
TEST(pikaMain, init) {
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = pikaScriptInit();
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, new_mem) {
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    VMParameters* globals =
        obj_runDirect(pikaMain, (char*)
        "mem = PikaStdLib.MemChecker()\n"
        "mem.max()\n"
        );
    PikaObj* mem = (PikaObj*)args_getPtr(globals->list, (char*)"mem");
    PikaObj* memClassPtr = (PikaObj*)obj_getPtr(mem, (char*)"_clsptr");
    EXPECT_EQ((void*)memClassPtr, (void*)New_PikaStdLib_MemChecker);
    obj_deinit(pikaMain);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, list_new) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    VMParameters* globals =
        obj_runDirect(pikaMain, (char*)
        "list = PikaStdData.List()\n"
        "list.append(7)\n"
        "list.append('eee')\n"
        "len = list.len()\n"
        "a1 = list.get(0)\n"
        "a2 = list.get(1)\n"
        );
    /* collect */
    int len = obj_getInt(globals, (char*)"len");
    int a1 = obj_getInt(globals, (char*)"a1");
    char* a2 = obj_getStr(globals, (char*)"a2");

    /* assert */
    EXPECT_EQ(len, 2);
    EXPECT_EQ(a1, 7);
    EXPECT_STREQ(a2, "eee");

    /* deinit */
    // obj_deinit(globals);
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, int_float_convert) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    VMParameters* globals =
        obj_runDirect(pikaMain, (char*)
        "a = 1\n"
        "a_f = float(a)\n"
        "b = 1.3\n"
        "b_i = int(b)\n"
        );
    /* collect */
    int a = obj_getInt(globals, (char*)"a");
    float a_f = obj_getFloat(globals, (char*)"a_f");
    float b = obj_getFloat(globals, (char*)"b");
    int b_i = obj_getInt(globals, (char*)"b_i");

    /* assert */
    EXPECT_EQ(a, 1);
    EXPECT_FLOAT_EQ(a_f, 1);
    EXPECT_FLOAT_EQ(b, 1.3);
    EXPECT_FLOAT_EQ(b_i, 1);

    /* deinit */
    // obj_deinit(globals);
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, type_) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
        "type(1)\n"
        "b = 1.4\n"
        "type(b)\n"
        );
    /* collect */

    /* assert */

    /* deinit */
    // obj_deinit(globals);
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, a_signed) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)"a = -1\n");
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");

    /* assert */
    EXPECT_EQ(-1, a);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, a_mm) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)"a = -1.1 ** 3\n");
    /* collect */
    float a = obj_getFloat(pikaMain, (char*)"a");

    /* assert */
    EXPECT_FLOAT_EQ(-1.331, a);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, a_n1) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)"a = ~1\n");
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");

    /* assert */
    EXPECT_EQ(-2, a);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, a_l4) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)"a = 1<<4\n");
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");

    /* assert */
    EXPECT_EQ(16, a);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, less_equ) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
    "a = 0\n"
    "while a <= 10:\n"
    "    a = a + 1\n"
    "\n"
    );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");

    /* assert */
    EXPECT_EQ(11, a);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, and_or_not) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
    "a = 0\n"
    "b = 0\n"
    "c = 0\n"
    "if (1>2) or (2>1):\n"
    "    a = 1\n"
    "\n"
    "if (1>2) and (2>1):\n"
    "    b = 1\n"
    "\n"
    "if not (1>2):\n"
    "    c = 1\n"
    "\n"
    );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    int b = obj_getInt(pikaMain, (char*)"b");
    int c = obj_getInt(pikaMain, (char*)"c");

    /* assert */
    EXPECT_EQ(1, a);
    EXPECT_EQ(0, b);
    EXPECT_EQ(1, c);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, err_scop) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)"print('test'");
    /* collect */

    /* assert */

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, __init__) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)"pin = PikaStdDevice.GPIO()\n");
    /* collect */
    char* a = obj_getStr(pikaMain, (char*)"pin.pin");
    /* assert */
    EXPECT_STREQ(a, "PA0");

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, PikaStdData) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "dict = PikaStdData.Dict()\n"
    "dict.set('a', 1)\n"
    "dict.set('b', 2)\n"
    "dict.remove('b')\n"
    "a = dict.get('a')\n"
    "b = dict.get('b')\n"
    );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    int b = obj_getInt(pikaMain, (char*)"b");
    /* assert */
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, -999999999);

    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, iter) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
        "list = PikaStdData.List()\n"
        "list.append(7)\n"
        "list.append('eee')\n"
        "len = list.len()\n"
        "a1 = list.get(0)\n"
        "a2 = list.get(1)\n"
        "res = iter(list)\n"
        "\n"
        );
    /* collect */
    PikaObj* res = (PikaObj*)obj_getPtr(pikaMain, (char*)"res");
    PikaObj* list = (PikaObj*)obj_getPtr(pikaMain, (char*)"list");
    /* assert */
    EXPECT_EQ(res, list);
    /* deinit */
    // obj_deinit(globals);
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, list_for_loop) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
        "list = PikaStdData.List()\n"
        "list.append(7)\n"
        "list.append('eee')\n"
        "len = list.len()\n"
        "for item in list:\n"
        "    print(item)\n"
        "    a = item\n"
        "\n"
        );
    /* collect */
    char* a = obj_getStr(pikaMain, (char*)"a");
    /* assert */
    EXPECT_STREQ(a, (char*)"eee");
    /* deinit */
    // obj_deinit(globals);
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, range) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
        "r = range(10, 0)\n"
        "ir = iter(r)\n"
        "\n"
        );
    /* collect */
    /* assert */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_in_range) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
        "a = 0\n"
        "for i in range(0, 10):\n"
        "    print(i)\n"
        "    a = a + i\n"
        "\n"
        );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    /* assert */
    EXPECT_EQ(a, 45);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_for_in_range) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
         "a = 0\n"
         "for i in range(0, 10):\n"
         "    for k in range(0, 3):\n"
         "        print(k)\n"
         "        a = a + k\n"
         "\n"
        );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    /* assert */
    EXPECT_EQ(a, 30);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_if_break) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_runDirect(pikaMain, (char*)
         "a = 0\n"
         "for i in range(0, 10):\n"
         "    if i == 5:\n"
         "        break\n"
         "    a = a + i\n"
         "\n"
        );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    /* assert */
    EXPECT_EQ(a, 10);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_if_continue) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
         "a = 0\n"
         "for i in range(0, 10):\n"
         "    if i == 5:\n"
         "        continue\n"
         "    a = a + i\n"
         "\n"
        );
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    /* assert */
    EXPECT_EQ(a, 40);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

/* the log_buff of printf */
extern char log_buff[LOG_BUFF_MAX][LOG_SIZE];
TEST(pikaMain, print_in_def) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    /* the test is used to fix too many print in def */
    /* clear the log_buff */
    memset(log_buff, 0, LOG_BUFF_MAX * LOG_SIZE);
    obj_runDirect(pikaMain, (char*)
        "def test_print():\n"
        "    print('test')\n"
        "test_print()\n"
        );
    /* collect */
    /* assert */
    /* should only print once, so the second log (log_buff[1]) shuold be '\0' */
    EXPECT_STREQ(log_buff[0], "test\r\n");
    EXPECT_STREQ(log_buff[1], "");
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, prime_100) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
            "num = 0\n"
            "i = 2\n"
            "for i in range(2 , 100):\n"
            "    j=2\n"
            "    is_prime = 1\n"
            "    for j in range(2 , i):\n"
            "        if i%j==0 :\n"
            "            is_prime = 0\n"
            "            break\n"
            "    if is_prime:\n"
            "        num = num + i\n"
            "\n");
    /* collect */
    int num = obj_getInt(pikaMain, (char*)"num");
    /* assert */
    EXPECT_EQ(num, 1060);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

/* the log_buff of printf */
extern char log_buff[LOG_BUFF_MAX][LOG_SIZE];
TEST(pikaMain, for_in_dict) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "print('__start__')\n"
    "dict = PikaStdData.Dict()\n"
    "dict.set('a', 1)\n"
    "dict.set('b', 2)\n"
    "dict.remove('b')\n"
    "for item in dict:\n"
    "    print(item)\n"
    "\n"
    );
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], "1\r\n");
    EXPECT_STREQ(log_buff[1], "__start__\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, str_add) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "a = 'a'\n"
    "b = 'b'\n"
    "c = a + b\n"
    "d = 1\n"
    "e = 1.2\n"
    "g = c + str(d)\n"
    "h = c + str(d) + str(e)\n"
    "\n"
    );
    /* collect */
    char* c = obj_getStr(pikaMain, (char*)"c");
    char* g = obj_getStr(pikaMain, (char*)"g");
    char* h = obj_getStr(pikaMain, (char*)"h");
    /* assert */
    EXPECT_STREQ(c, (char*)"ab");
    EXPECT_STREQ(g, (char*)"ab1");
    EXPECT_STREQ(h, (char*)"ab11.200000");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, str_add_print) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "print('test: ' + str(2233))\n"
    "\n"
    );
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], "test: 2233\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, int_float_to_str) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "a = str(1)\n"
    "b = str(1.2)\n"
    "\n"
    );
    /* collect */
    char* a = obj_getStr(pikaMain, (char*)"a");
    char* b = obj_getStr(pikaMain, (char*)"b");
    /* assert */
    EXPECT_STREQ(a, "1");
    EXPECT_STREQ(b, "1.200000");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, str_eq) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    obj_run(pikaMain, (char*)
    "a = '1ee'\n"
    "b = '1ee'\n"
    "c = '1e'\n"
    "if a==b:\n"
    "    res1 = 1\n"
    "else:\n"
    "    res1 = 0\n"
    "if a==c:\n"
    "    res2 = 1\n"
    "else:\n"
    "    res2 = 0\n"
    "\n"
    );
    /* collect */
    int res1 = obj_getInt(pikaMain, (char*)"res1");
    int res2 = obj_getInt(pikaMain, (char*)"res2");
    /* assert */
    EXPECT_EQ(res1, 1);
    EXPECT_EQ(res2, 0);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, print_with_enter) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)"print('test\\n')\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"test\n\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, print_with_2enter) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)"print('test\\r\\n\\n')\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"test\r\n\n\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, print_ddd) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)"print(\"[Info]: in Python config ...\")\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"[Info]: in Python config ...\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_in_string) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "s = PikaStdData.String()\n"
    "s.set('test')\n"
    "for c in s:\n"
    "    print(c)\n"
    "\n"
    );
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[4], (char*)"BEGIN\n");
    EXPECT_STREQ(log_buff[3], (char*)"t\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"e\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"s\r\n");
    EXPECT_STREQ(log_buff[0], (char*)"t\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

// TEST(pikaMain, obj_no_free) {
//     /* init */
//     pikaMemInfo.heapUsedMax = 0;
//     PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
//     /* run */
//     __platform_printf((char*)"BEGIN\n");
//     obj_run(pikaMain, (char*)
//     "s = PikaStdData.String()\n"
//     "s.set('test')\n"
//     "s = PikaStdData.String()\n"
//     "s.set('test')\n"
//     "\n"
//     );
//     /* collect */
//     /* assert */
//     /* deinit */
//     obj_deinit(pikaMain);
//     EXPECT_EQ(pikaMemNow(), 0);
// }

TEST(pikaMain, list__set__) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "list = PikaStdData.List()\n"
    "__set__(list, 0, 2)\n"
    "res = __get__(list, 0)\n"
    "\n"
    );
    /* collect */
    int res = obj_getInt(pikaMain, (char*)"res");
    /* assert */
    EXPECT_EQ(res, 2);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, string__get__) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "s = PikaStdData.String()\n"
    "s.set('test')\n"
    "res = __get__(s, 2)\n"
    "\n"
    );
    /* collect */
    char* res = obj_getStr(pikaMain, (char*)"res");
    /* assert */
    EXPECT_STREQ(res, "s");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, dict__set__get) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "list = PikaStdData.Dict()\n"
    "__set__(list, 'a', 2)\n"
    "res = __get__(list, 'a')\n"
    "\n"
    );
    /* collect */
    int res = obj_getInt(pikaMain, (char*)"res");
    /* assert */
    EXPECT_EQ(res, 2);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, str___get____set__) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "s = 'test'\n"
    "res = __get__(s, 2)\n"
    "__set__(s, 2, 'q', 's')\n"
    "\n"
    );
    /* collect */
    char* res = obj_getStr(pikaMain, (char*)"res");
    char* s = obj_getStr(pikaMain, (char*)"s");
    /* assert */
    EXPECT_STREQ(res, (char*)"s");
    EXPECT_STREQ(s, (char*)"teqt");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, str__index__) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "s = 'test'\n"
    "res = s[2]\n"
    "res2 = 'eqrt'[2]\n"
    "s[2] = 'q'\n"
    "\n"
    );
    /* collect */
    char* res = obj_getStr(pikaMain, (char*)"res");
    char* res2 = obj_getStr(pikaMain, (char*)"res2");
    char* s = obj_getStr(pikaMain, (char*)"s");
    /* assert */
    EXPECT_STREQ(res, (char*)"s");
    EXPECT_STREQ(res2, (char*)"r");
    EXPECT_STREQ(s, (char*)"teqt");
    // EXPECT_STREQ(s, (char*)"teqt");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, list_index) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "list = PikaStdData.List()\n"
    "list[0] = 2\n"
    "res = list[0]\n"
    "\n"
    );
    /* collect */
    int res = obj_getInt(pikaMain, (char*)"res");
    /* assert */
    EXPECT_EQ(res, 2);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, dict_index) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\n");
    obj_run(pikaMain, (char*)
    "dict = PikaStdData.Dict()\n"
    "dict['a'] = 2\n"
    "res = dict['a']\n"
    "\n"
    );
    /* collect */
    int res = obj_getInt(pikaMain, (char*)"res");
    /* assert */
    EXPECT_EQ(res, 2);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, task_run_once) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    obj_run(pikaMain,(char*)
            "def todo1():\n"
            "    print('task 1 running...')\n"
            "def todo2():\n"
            "    print('task 2 running...')\n"
            "task = GTestTask.Task()\n"
            "task.call_always(todo1)\n"
            "task.call_always(todo2)\n"
            "task.run_once()\n"
            "\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"task 2 running...\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"task 1 running...\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, task_run_when) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    obj_run(pikaMain,(char*)
            "def todo1():\n"
            "    print('task 1 running...')\n"
            "def todo2():\n"
            "    print('task 2 running...')\n"
            "def todo3():\n"
            "    print('task 3 running...')\n"
            "def when3():\n"
            "    return True\n"
            "task = GTestTask.Task()\n"
            "task.call_always(todo1)\n"
            "task.call_always(todo2)\n"
            "task.call_when(todo3, when3)\n"
            "task.run_once()\n"
            "\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"task 3 running...\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"task 2 running...\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"task 1 running...\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, task_run_period_until) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(pikaMain,(char*)
            "def todo1():\n"
            "    print('task 1 running...')\n"
            "def todo2():\n"
            "    print('task 2 running...')\n"
            "def todo3():\n"
            "    print('task 3 running...')\n"
            "def when3():\n"
            "    return True\n"
            "task = GTestTask.Task()\n"
            "task.call_period_ms(todo1, 200)\n"
            "task.call_period_ms(todo2, 500)\n"
            "# task.call_when(todo3, when3)\n"
            "task.run_until_ms(1000)\n"
            "\n");
    /* collect */
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"task 1 running...\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"task 1 running...\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"task 2 running...\r\n");
    EXPECT_STREQ(log_buff[3], (char*)"task 1 running...\r\n");
    EXPECT_STREQ(log_buff[4], (char*)"task 1 running...\r\n");
    EXPECT_STREQ(log_buff[5], (char*)"task 2 running...\r\n");
    EXPECT_STREQ(log_buff[6], (char*)"task 1 running...\r\n");
    EXPECT_STREQ(log_buff[7], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, fun_call) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(pikaMain,(char*)
            "def fun(a,b,c,d,e):\n"
            "    print(a)\n"
            "    print(b)\n"
            "    print(c)\n"
            "    print(d)\n"
            "    print(e)\n"
            "fun(10,20,30,40, 'xxx')\n"
            "\n");
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"xxx\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"40\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"30\r\n");
    EXPECT_STREQ(log_buff[3], (char*)"20\r\n");
    EXPECT_STREQ(log_buff[4], (char*)"10\r\n");
    EXPECT_STREQ(log_buff[5], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, global) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(pikaMain,(char*)
        "x = 0\n"
        "a = 0\n"
        "z = 0\n"
        "def test_global():\n"
        "    global x\n"
        "    global y, z\n"
        "    x = 1\n"
        "    a = 1\n"
        "    z = 1\n"
        "    print('test global')\n"
        "test_global()\n"
        "\n"
    );
    /* assert */
    int x = obj_getInt(pikaMain, (char*)"x");
    int a = obj_getInt(pikaMain, (char*)"a");
    int z = obj_getInt(pikaMain, (char*)"z");
    EXPECT_EQ(x, 1);
    EXPECT_EQ(a, 0);
    EXPECT_EQ(z, 1);
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, complex_str) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(pikaMain, (char*)"print('test,test')\n");
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"test,test\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, synac_err_1) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(pikaMain, (char*)"print('testtest)\n");
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"[error]: Syntax error.\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_arg) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    Args* buffs = New_strBuff();
    char * pikaAsm = Parser_multiLineToAsm(buffs, (char*)
        "class Test(PikaStdLib.PikaObj):\n"
        "    x = 1\n"
        "\n"
        "test = Test()\n"
        "print(test.x)\n"
    );
    printf("%s", pikaAsm);
    pikaVM_runAsm(pikaMain, pikaAsm);
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"1\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_def) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
    "class Test():\n"
    "    x = 1\n"
    "    def hello(self, x):\n"
    "        return x + 2\n"
    "test = Test()\n"
    "x = test.hello(test.x)\n"
    "print(x)\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"3\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_def_print) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
    "class Test():\n"
    "    x = 1\n"
    "    def hi(self):\n"
    "        print('hi' + str(self.x))\n"
    "test = Test()\n"
    "test.hi()\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"hi1\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_demo_1) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
        "class MyClass:\n"
        "    i = 12345\n"
        "    def f(self):\n"
        "        return 'hello world'\n"
        "        \n"
        "x = MyClass()\n"
        " \n"
        "print(x.i)\n"
        "print(x.f())\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"hello world\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"12345\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_demo_2) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
        "class Complex:\n"
        "    def __init__(self):\n"
        "        self.r = 3.0\n"
        "        self.i = -4.5\n"
        "x = Complex()\n"
        "print(x.r)\n"
        "print(x.i)\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"-4.500000\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"3.000000\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_demo_3) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
        "class people:\n"
        "    def speak(self):\n"
        "        print('i am a people')\n"
        " \n"
        "class student(people):\n"
        "    def speak(self):\n"
        "        print('i am a student')\n"
        " \n"
        "p = people()\n"
        "s = student()\n"
        "p.speak()\n"
        "s.speak()\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[2], (char*)"BEGIN\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"i am a people\r\n");
    EXPECT_STREQ(log_buff[0], (char*)"i am a student\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_demo_save_asm) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    obj_run(self, (char*)
        "class people:\n"
        "    def speak(self):\n"
        "        print('i am a people')\n"
        " \n"
        "class student(people):\n"
        "    def speak(self):\n"
        "        print('i am a student')\n"
        " \n"
    );
    obj_run(self, (char*)
        "p = people()\n"
        "s = student()\n"
        "p.speak()\n"
        "s.speak()\n"
    );
    /* assert */
    EXPECT_STREQ(log_buff[2], (char*)"BEGIN\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"i am a people\r\n");
    EXPECT_STREQ(log_buff[0], (char*)"i am a student\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, for_if_continue_byte_code) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    pikaVM_run(pikaMain, (char*)
         "a = 0\n"
         "for i in range(0, 10):\n"
         "    if i == 5:\n"
         "        continue\n"
         "    a = a + i\n"
         "\n");
    /* collect */
    int a = obj_getInt(pikaMain, (char*)"a");
    /* assert */
    EXPECT_EQ(a, 40);
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, print_in_def_byte_code) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* pikaMain = newRootObj((char*)"pikaMain", New_PikaMain);
    /* run */
    __platform_printf((char*)"BEGIN\r\n");
    pikaVM_run(pikaMain, (char*)
        "def test_print():\n"
        "    print('test')\n"
        "test_print()\n"
        );
    /* collect */
    /* assert */
    /* should only print once, so the second log (log_buff[1]) shuold be '\0' */
    EXPECT_STREQ(log_buff[0], "test\r\n");
    EXPECT_STREQ(log_buff[1], "BEGIN\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    /* mem check */
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(pikaMain, class_demo_1_file) {
    /* init */
    pikaMemInfo.heapUsedMax = 0;
    /* run */
    PikaObj* self = newRootObj((char*)"pikaMain", New_PikaMain);
    __platform_printf((char*)"BEGIN\r\n");
    char lines[] =
        "class MyClass:\n"
        "    i = 12345\n"
        "    def f(self):\n"
        "        return 'hello world'\n"
        "        \n"
        "x = MyClass()\n"
        " \n"
        "print(x.i)\n"
        "print(x.f())\n";
    Parser_multiLineToFile((char*)lines);
    char bytecodebuff[4096] = {0};
    FILE* f = __platform_fopen("pika_bytecode.bin", "r");
    fread(bytecodebuff, 1, 4096, f);
    pikaVM_runByteCode(self, (uint8_t*)bytecodebuff);
    /* assert */
    EXPECT_STREQ(log_buff[0], (char*)"hello world\r\n");
    EXPECT_STREQ(log_buff[1], (char*)"12345\r\n");
    EXPECT_STREQ(log_buff[2], (char*)"BEGIN\r\n");
    /* deinit */
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}
