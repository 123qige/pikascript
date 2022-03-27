#include "gtest/gtest.h"
extern "C" {
#include "PikaVM.h"
#include "dataArgs.h"
#include "dataString.h"
}
static int mem;
TEST(args, test1) {
    mem = pikaMemNow();
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"a", 1);
    int a = args_getInt(args, (char*)"a");
    EXPECT_EQ(a, 1);
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, test4) {
    int32_t testint = 12333;
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"testint", testint);
    EXPECT_EQ(12333, args_getInt(args, (char*)"testint"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, test2) {
    Args* args = New_args(NULL);
    int64_t int64Out = 0;
    void* pointer = NULL;
    char* strOut = NULL;
    args_setInt(args, (char*)"int64Test", (int64_t)22221);
    args_setPtr(args, (char*)"pointerTest", (void*)2222322);
    args_setStr(args, (char*)"strTest", (char*)"teeeds");

    int64Out = args_getInt(args, (char*)"int64Test");
    pointer = args_getPtr(args, (char*)"pointerTest");
    strOut = args_getStr(args, (char*)"strTest");

    EXPECT_EQ(int64Out, 22221);
    EXPECT_EQ((uint64_t)pointer, 2222322);
    EXPECT_EQ(1, strEqu((char*)"teeeds", strOut));
    EXPECT_EQ(args_getType(args, (char*)"int64Test"), ARG_TYPE_INT);
    EXPECT_EQ(args_getType(args, (char*)"pointerTest"), ARG_TYPE_POINTER);
    EXPECT_EQ(args_getType(args, (char*)"strTest"), ARG_TYPE_STRING);
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, test3) {
    Args* args1 = New_args(NULL);
    Args* args2 = New_args(NULL);

    args_setFloat(args1, (char*)"argtest1", 2.883);
    args_setStr(args1, (char*)"strtest1", (char*)"teststr");
    args_copyArgByName(args1, (char*)"argtest1", args2);
    args_copyArgByName(args1, (char*)"strtest1", args2);

    EXPECT_EQ(2.8830f, args_getFloat(args1, (char*)"argtest1"));
    args_deinit(args1);
    args_deinit(args2);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, buff) {
    Args* buffs = New_args(NULL);
    args_getBuff(buffs, 256);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, update) {
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"testInt", 124);
    args_setInt(args, (char*)"testInt", 125);
    EXPECT_EQ(125, args_getInt(args, (char*)"testInt"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, print_int) {
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"testInt", 124);
    EXPECT_STREQ((char*)"124", args_print(args, (char*)"testInt"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

// TEST(args, test5) {
// int32_t testInt = 124;
// Args* args = New_args(NULL);
// args_bind(args, (char*)"i", (char*)"testInt", &testInt);
// char* type = args_getType(args, (char*)"testInt");
// args_print(args, (char*)"testInt");
// EXPECT_STREQ((char*)"124", args_print(args, (char*)"testInt"));
// args_deinit(args);
// EXPECT_EQ(pikaMemNow(), 0);
// }

TEST(args, test6) {
    Args* args = New_args(NULL);
    args_setFloat(args, (char*)"testfloat", 1.42);
    EXPECT_STREQ((char*)"1.420000", args_print(args, (char*)"testfloat"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

// TEST(args, test7) {
// Args* args = New_args(NULL);
// float floatBindTest = 2.314;
// args_bind(args, (char*)"f", (char*)"floatBind", &floatBindTest);
// EXPECT_STREQ((char*)"2.314000", args_print(args, (char*)"floatBind"));
// args_deinit(args);
// EXPECT_EQ(pikaMemNow(), 0);
// }

TEST(args, test8) {
    Args* args = New_args(NULL);
    args_setStr(args, (char*)"testString", (char*)"test string print");
    EXPECT_STREQ((char*)"test string print",
                 args_print(args, (char*)"testString"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

// TEST(args, test9) {
// Args* args = New_args(NULL);
// char strBindTest[] = "test string bind";
// args_bindStr(args, (char*)"testStringBind", (char**)&strBindTest);
// EXPECT_STREQ((char*)"test string bind",
//  args_print(args, (char*)"testStringBind"));
// args_deinit(args);
// EXPECT_EQ(pikaMemNow(), 0);
// }

TEST(args, test12) {
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"test", 13);
    args_setInt(args, (char*)"kw", 25);
    EXPECT_EQ(2, args_getSize(args));
    EXPECT_EQ(13, (int)args_getInt(args, (char*)"test"));
    args_removeArg(args, args_getArg(args, (char*)"test"));
    EXPECT_EQ(1, args_getSize(args));
    EXPECT_EQ(-999999999, (int)args_getInt(args, (char*)"test"));
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

struct test {
    int a;
    float b;
    int c;
};
TEST(args, struct_) {
    Args* args = New_args(NULL);
    struct test test_struct = {1, 2.2, 4};
    args_setStruct(args, (char*)"test", test_struct);
    struct test* out_struct = (struct test*)args_getStruct(args, (char*)"test");
    EXPECT_EQ(out_struct->a, 1);
    EXPECT_FLOAT_EQ(out_struct->b, 2.2);
    EXPECT_EQ(out_struct->c, 4);
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, mem) {
    EXPECT_EQ(pikaMemNow(), mem);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, index) {
    Args* args = New_args(NULL);
    args_pushArg(args, arg_setInt(NULL, (char*)"", 1));
    args_pushArg(args, arg_setFloat(NULL, (char*)"", 2.4));

    int a = arg_getInt(args_getArg_index(args, 1));
    float b = arg_getFloat(args_getArg_index(args, 0));
    /* assert */
    EXPECT_EQ(a, 1);
    EXPECT_FLOAT_EQ(b, 2.4);
    /* check memory */
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, int_float_convert) {
    Args* args = New_args(NULL);
    args_setInt(args, (char*)"a", 10);
    args_setFloat(args, (char*)"b", 2.333);

    float a = args_getFloat(args, (char*)"a");
    int b = args_getInt(args, (char*)"b");
    /* assert */
    EXPECT_EQ(a, 10);
    EXPECT_FLOAT_EQ(b, 2);
    /* check memory */
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, heap_struct) {
    ByteCodeFrame bytecode_frame;
    byteCodeFrame_init(&bytecode_frame);
    bytecode_frame.const_pool.size = 100;
    Args* args = New_args(NULL);
    args_setHeapStruct(args, (char*)"a", bytecode_frame, byteCodeFrame_deinit);
    ByteCodeFrame* bf_out =
        (ByteCodeFrame*)args_getHeapStruct(args, (char*)"a");
    /* assert */
    EXPECT_EQ(bf_out->const_pool.size, 100);
    /* check memory */
    args_deinit(args);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(args, args_move) {
    Args* args = New_args(NULL);
    Args* args2 = New_args(NULL);
    args_setInt(args, (char*)"a", 100);
    args_moveArg(args, args2, args_getArg(args, (char*)"a"));
    /* assert */
    EXPECT_EQ(-999999999, args_getInt(args, (char*)"a"));
    EXPECT_EQ(100, args_getInt(args2, (char*)"a"));
    /* deinit */
    args_deinit(args);
    args_deinit(args2);
    EXPECT_EQ(pikaMemNow(), 0);
}
