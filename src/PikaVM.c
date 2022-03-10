/*
 * This file is part of the PikaScript project.
 * http://github.com/pikastech/pikascript
 *
 * MIT License
 *
 * Copyright (c) 2021 lyon 李昂 liang6516@outlook.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __PIKA_OBJ_CLASS_IMPLEMENT
#include "PikaVM.h"
#include "BaseObj.h"
#include "PikaObj.h"
#include "PikaParser.h"
#include "PikaPlatform.h"
#include "dataQueue.h"
#include "dataQueueObj.h"
#include "dataStrs.h"

/* local head */
VMParameters* pikaVM_runAsmWithPars(PikaObj* self,
                                    VMParameters* locals,
                                    VMParameters* globals,
                                    char* pikaAsm);

struct VMState_t {
    VMParameters* locals;
    VMParameters* globals;
    Queue* q0;
    Queue* q1;
    int32_t jmp;
    char* pc;
    char* ASM_start;
};
typedef struct VMState_t VMState;

static int32_t __gotoNextLine(char* code) {
    int offset = 0;
    while (1) {
        if (code[offset] == '\n') {
            break;
        }
        offset++;
    }
    return offset + 1;
}

static int32_t __gotoLastLine(char* start, char* code) {
    int offset = -2;
    while (1) {
        char* codeNow = code + offset;
        if (codeNow == start) {
            offset--;
            break;
        }
        if (codeNow[0] == '\n') {
            break;
        }
        offset--;
    }
    return offset + 1;
}

static int __getThisBlockDeepth(char* start, char* code, int* offset) {
    int thisBlockDeepth = -1;
    char* codeNow = code + *offset;
    while (1) {
        *offset += __gotoLastLine(start, codeNow);
        codeNow = code + *offset;
        if (codeNow[0] == 'B') {
            thisBlockDeepth = codeNow[1] - '0';
            break;
        }
    }
    return thisBlockDeepth;
}

static int32_t __getAddrOffsetOfJUM(char* code) {
    int offset = 0;
    char* codeNow = code + offset;
    while (1) {
        offset += __gotoNextLine(codeNow);
        codeNow = code + offset;
        if (0 == strncmp(codeNow, "0 JMP -1", 8)) {
            return offset;
        }
    }
}

static int32_t __getAddrOffsetFromJmp(char* start, char* code, int32_t jmp) {
    int offset = 0;
    int thisBlockDeepth = __getThisBlockDeepth(start, code, &offset);
    char* codeNow = code + offset;
    int8_t blockNum = 0;
    if (jmp > 0) {
        offset = 0;
        codeNow = code + offset;
        while (1) {
            offset += __gotoNextLine(codeNow);
            codeNow = code + offset;
            if (codeNow[0] == 'B') {
                uint8_t blockDeepth = codeNow[1] - '0';
                if (blockDeepth <= thisBlockDeepth) {
                    blockNum++;
                }
            }
            if (blockNum >= jmp) {
                break;
            }
        }
    }
    if (jmp < 0) {
        while (1) {
            offset += __gotoLastLine(start, codeNow);
            codeNow = code + offset;
            if (codeNow[0] == 'B') {
                uint8_t blockDeepth = codeNow[1] - '0';
                if (blockDeepth == thisBlockDeepth) {
                    blockNum--;
                }
            }
            if (blockNum <= jmp) {
                break;
            }
        }
    }
    return offset;
}

int32_t __clearInvokeQueues(VMParameters* vm_pars) {
    for (char deepthChar = '0'; deepthChar < '9'; deepthChar++) {
        char deepth[2] = {0};
        deepth[0] = deepthChar;
        Queue* queue = (Queue*)args_getPtr(vm_pars->list, deepth);
        if (NULL != queue) {
            args_deinit(queue);
            args_removeArg(vm_pars->list, args_getArg(vm_pars->list, deepth));
        }
    }
    return 0;
}


typedef Arg* (*VM_instruct_handler)(PikaObj* self, VMState* vs, char* data);

static Arg* VM_instruction_handler_NON(PikaObj* self, VMState* vs, char* data) {
    return NULL;
}

static Arg* VM_instruction_handler_NEW(PikaObj* self, VMState* vs, char* data) {
    Arg* origin_arg = obj_getArg(vs->locals, data);
    Arg* new_arg = arg_copy(origin_arg);
    origin_arg = arg_setType(origin_arg, ARG_TYPE_POINTER);
    arg_setType(new_arg, ARG_TYPE_FREE_OBJECT);
    return new_arg;
}

static Arg* VM_instruction_handler_REF(PikaObj* self, VMState* vs, char* data) {
    if (strEqu(data, (char*)"True")) {
        return arg_setInt(NULL, "", 1);
    }
    if (strEqu(data, (char*)"False")) {
        return arg_setInt(NULL, "", 0);
    }
    /* find in local list first */
    Arg* arg = arg_copy(obj_getArg(vs->locals, data));
    if (NULL == arg) {
        /* find in global list second */
        arg = arg_copy(obj_getArg(vs->globals, data));
    }
    ArgType arg_type = arg_getType(arg);
    if (ARG_TYPE_OBJECT == arg_type) {
        arg = arg_setType(arg, ARG_TYPE_POINTER);
    }
    return arg;
}

static Arg* VM_instruction_handler_RUN(PikaObj* self, VMState* vs, char* data) {
    Args buffs = {0};
    Arg* return_arg = NULL;
    VMParameters* sub_locals = NULL;
    char* methodPath = data;
    PikaObj* method_host_obj;
    Arg* method_arg;
    Method method_ptr;
    ArgType method_type = ARG_TYPE_NONE;
    char* method_dec;
    char* type_list;
    char* method_code;
    char* sys_out;
    /* return arg directly */
    if (strEqu(data, "")) {
        return_arg = arg_copy(queue_popArg(vs->q1));
        goto RUN_exit;
    }

    /* get method host obj */
    method_host_obj = obj_getObj(self, methodPath, 1);
    if (NULL == method_host_obj) {
        method_host_obj = obj_getObj(vs->locals, methodPath, 1);
    }
    if (NULL == method_host_obj) {
        /* error, not found object */
        args_setErrorCode(vs->locals->list, 1);
        args_setSysOut(vs->locals->list, "[error] runner: object no found.");
        goto RUN_exit;
    }
    /* get method in local */
    method_arg = obj_getMethod(method_host_obj, methodPath);
    if (NULL == method_arg) {
        method_arg = obj_getMethod(vs->globals, methodPath);
    }
    /* assert method*/
    if (NULL == method_arg) {
        /* error, method no found */
        args_setErrorCode(vs->locals->list, 2);
        args_setSysOut(vs->locals->list, "[error] runner: method no found.");
        goto RUN_exit;
    }
    /* get method Ptr */
    method_ptr = methodArg_getPtr(method_arg);
    /* get method Decleartion */
    method_dec = strsCopy(&buffs, methodArg_getDec(method_arg));
    method_type = arg_getType(method_arg);
    arg_deinit(method_arg);

    /* get type list */
    type_list = strsCut(&buffs, method_dec, '(', ')');

    if (type_list == NULL) {
        /* typeList no found */
        args_setErrorCode(vs->locals->list, 3);
        args_setSysOut(vs->locals->list, "[error] runner: type list no found.");
        goto RUN_exit;
    }

    sub_locals = New_PikaObj();
    Arg* call_arg = NULL;
    uint8_t call_arg_index = 0;
    while (1) {
        /* load 'self' as the first arg when call object method */
        if ((method_type == ARG_TYPE_OBJECT_METHOD) && (call_arg_index == 0)) {
            call_arg = arg_setPtr(NULL, "", ARG_TYPE_POINTER, method_host_obj);
        } else {
            call_arg = arg_copy(queue_popArg(vs->q1));
        }
        /* exit when there is no arg in queue */
        if (NULL == call_arg) {
            break;
        }
        char* argDef = strsPopToken(&buffs, type_list, ',');
        char* argName = strsGetFirstToken(&buffs, argDef, ':');
        call_arg = arg_setName(call_arg, argName);
        args_setArg(sub_locals->list, call_arg);
        call_arg_index++;
    }

    obj_setErrorCode(method_host_obj, 0);
    obj_setSysOut(method_host_obj, "");

    /* run method */
    method_code = (char*)method_ptr;
    if (method_code[0] == 'B' && method_code[2] == '\n') {
        /* VM method */
        sub_locals = pikaVM_runAsmWithPars(method_host_obj, sub_locals,
                                           vs->globals, method_code);
        /* get method return */
        return_arg = arg_copy(args_getArg(sub_locals->list, (char*)"return"));
    } else {
        /* native method */
        method_ptr(method_host_obj, sub_locals->list);
        /* get method return */
        return_arg = arg_copy(args_getArg(sub_locals->list, (char*)"return"));
    }

    /* transfer sysOut */
    sys_out = obj_getSysOut(method_host_obj);
    if (NULL != sys_out) {
        args_setSysOut(vs->locals->list, sys_out);
    }
    /* transfer errCode */
    if (0 != obj_getErrorCode(method_host_obj)) {
        /* method error */
        args_setErrorCode(vs->locals->list, 6);
    }

    goto RUN_exit;
RUN_exit:
    if (NULL != sub_locals) {
        obj_deinit(sub_locals);
    }
    strsDeinit(&buffs);
    return return_arg;
}

static Arg* VM_instruction_handler_STR(PikaObj* self, VMState* vs, char* data) {
    Arg* strArg = New_arg(NULL);
    return arg_setStr(strArg, "", data);
}

typedef enum {
    IS_INIT_OBJ_TRUE,
    IS_INIT_OBJ_FALSE,
} is_init_obj_t;

static Arg* __VM_OUT(PikaObj* self,
                     VMState* vs,
                     char* data,
                     is_init_obj_t is_init_obj) {
    Arg* outArg = arg_copy(queue_popArg(vs->q0));
    ArgType outArg_type = arg_getType(outArg);
    PikaObj* hostObj = vs->locals;
    /* match global_list */
    if (args_isArgExist(vs->locals->list, "__gl")) {
        char* global_list = args_getStr(vs->locals->list, "__gl");
        /* use a arg as buff */
        Arg* global_list_arg = arg_setStr(NULL, "", global_list);
        char* global_list_buff = arg_getStr(global_list_arg);
        /* for each arg arg in global_list */
        char token_buff[32] = {0};
        for (int i = 0; i < strCountSign(global_list, ',') + 1; i++) {
            char* global_arg = strPopToken(token_buff, global_list_buff, ',');
            /* matched global arg, hostObj set to global */
            if (strEqu(global_arg, data)) {
                hostObj = vs->globals;
            }
        }
        arg_deinit(global_list_arg);
    }
    /* use RunAs object */
    if (args_isArgExist(vs->locals->list, "__runAs")) {
        hostObj = args_getPtr(vs->locals->list, "__runAs");
    }
    /* set free object to nomal object */
    if (ARG_TYPE_FREE_OBJECT == outArg_type) {
        arg_setType(outArg, ARG_TYPE_OBJECT);
    }
    /* ouput arg to locals */
    obj_setArg(hostObj, data, outArg);
    if ((ARG_TYPE_MATE_OBJECT == outArg_type) ||
        (ARG_TYPE_FREE_OBJECT == outArg_type)) {
        if (is_init_obj == IS_INIT_OBJ_TRUE) {
            /* found a mate_object */
            /* init object */
            PikaObj* new_obj = obj_getObj(hostObj, data, 0);
            /* run __init__() when init obj */
            Arg* init_method_arg = NULL;
            init_method_arg = obj_getMethod(new_obj, "__init__");
            if (NULL != init_method_arg) {
                arg_deinit(init_method_arg);
                pikaVM_runAsm(new_obj,
                              "B0\n"
                              "0 RUN __init__\n");
            }
        }
    }
    arg_deinit(outArg);
    return NULL;
}

static Arg* VM_instruction_handler_OUT(PikaObj* self, VMState* vs, char* data) {
    return __VM_OUT(self, vs, data, IS_INIT_OBJ_TRUE);
}

/* run as */
static Arg* VM_instruction_handler_RAS(PikaObj* self, VMState* vs, char* data) {
    if (strEqu(data, "$origin")) {
        /* use origin object to run */
        obj_removeArg(vs->locals, "__runAs");
        return NULL;
    }
    /* use "data" object to run */
    PikaObj* runAs = obj_getObj(vs->locals, data, 0);
    args_setPtr(vs->locals->list, "__runAs", runAs);
    return NULL;
}

static Arg* VM_instruction_handler_NUM(PikaObj* self, VMState* vs, char* data) {
    Arg* numArg = New_arg(NULL);
    if (strIsContain(data, '.')) {
        return arg_setFloat(numArg, "", atof(data));
    }
    return arg_setInt(numArg, "", fast_atoi(data));
}

static Arg* VM_instruction_handler_JMP(PikaObj* self, VMState* vs, char* data) {
    vs->jmp = fast_atoi(data);
    return NULL;
}

static Arg* VM_instruction_handler_JEZ(PikaObj* self, VMState* vs, char* data) {
    int offset = 0;
    int thisBlockDeepth = __getThisBlockDeepth(vs->ASM_start, vs->pc, &offset);
    Arg* assertArg = arg_copy(queue_popArg(vs->q0));
    int assert = arg_getInt(assertArg);
    arg_deinit(assertArg);
    char __else[] = "__else0";
    __else[6] = '0' + thisBlockDeepth;
    args_setInt(self->list, __else, !assert);
    if (0 == assert) {
        /* set __else flag */
        vs->jmp = fast_atoi(data);
    }
    return NULL;
}

static Arg* VM_instruction_handler_OPT(PikaObj* self, VMState* vs, char* data) {
    Arg* outArg = NULL;
    Arg* arg1 = arg_copy(queue_popArg(vs->q1));
    Arg* arg2 = arg_copy(queue_popArg(vs->q1));
    ArgType type_arg1 = arg_getType(arg1);
    ArgType type_arg2 = arg_getType(arg2);
    int num1_i = 0;
    int num2_i = 0;
    float num1_f = 0.0;
    float num2_f = 0.0;
    /* get int and float num */
    if (type_arg1 == ARG_TYPE_INT) {
        num1_i = arg_getInt(arg1);
        num1_f = (float)num1_i;
    } else if (type_arg1 == ARG_TYPE_FLOAT) {
        num1_f = arg_getFloat(arg1);
        num1_i = (int)num1_f;
    }
    if (type_arg2 == ARG_TYPE_INT) {
        num2_i = arg_getInt(arg2);
        num2_f = (float)num2_i;
    } else if (type_arg2 == ARG_TYPE_FLOAT) {
        num2_f = arg_getFloat(arg2);
        num2_i = (int)num2_f;
    }
    if (strEqu("+", data)) {
        if ((type_arg1 == ARG_TYPE_STRING) && (type_arg2 == ARG_TYPE_STRING)) {
            char* num1_s = NULL;
            char* num2_s = NULL;
            Args str_opt_buffs = {0};
            num1_s = arg_getStr(arg1);
            num2_s = arg_getStr(arg2);
            char* opt_str_out = strsAppend(&str_opt_buffs, num1_s, num2_s);
            outArg = arg_setStr(outArg, "", opt_str_out);
            strsDeinit(&str_opt_buffs);
            goto OPT_exit;
        }
        /* match float */
        if ((type_arg1 == ARG_TYPE_FLOAT) || type_arg2 == ARG_TYPE_FLOAT) {
            outArg = arg_setFloat(outArg, "", num1_f + num2_f);
            goto OPT_exit;
        }
        /* int is default */
        outArg = arg_setInt(outArg, "", num1_i + num2_i);
        goto OPT_exit;
    }
    if (strEqu("-", data)) {
        if (type_arg2 == ARG_TYPE_NONE) {
            if (type_arg1 == ARG_TYPE_INT) {
                outArg = arg_setInt(outArg, "", -num1_i);
                goto OPT_exit;
            }
            if (type_arg1 == ARG_TYPE_FLOAT) {
                outArg = arg_setFloat(outArg, "", -num1_f);
                goto OPT_exit;
            }
        }
        if ((type_arg1 == ARG_TYPE_FLOAT) || type_arg2 == ARG_TYPE_FLOAT) {
            outArg = arg_setFloat(outArg, "", num1_f - num2_f);
            goto OPT_exit;
        }
        outArg = arg_setInt(outArg, "", num1_i - num2_i);
        goto OPT_exit;
    }
    if (strEqu("*", data)) {
        if ((type_arg1 == ARG_TYPE_FLOAT) || type_arg2 == ARG_TYPE_FLOAT) {
            outArg = arg_setFloat(outArg, "", num1_f * num2_f);
            goto OPT_exit;
        }
        outArg = arg_setInt(outArg, "", num1_i * num2_i);
        goto OPT_exit;
    }
    if (strEqu("/", data)) {
        outArg = arg_setFloat(outArg, "", num1_f / num2_f);
        goto OPT_exit;
    }
    if (strEqu("<", data)) {
        outArg = arg_setInt(outArg, "", num1_f < num2_f);
        goto OPT_exit;
    }
    if (strEqu(">", data)) {
        outArg = arg_setInt(outArg, "", num1_f > num2_f);
        goto OPT_exit;
    }
    if (strEqu("%", data)) {
        outArg = arg_setInt(outArg, "", num1_i % num2_i);
        goto OPT_exit;
    }
    if (strEqu("**", data)) {
        float res = 1;
        for (int i = 0; i < num2_i; i++) {
            res = res * num1_f;
        }
        outArg = arg_setFloat(outArg, "", res);
        goto OPT_exit;
    }
    if (strEqu("//", data)) {
        outArg = arg_setInt(outArg, "", num1_i / num2_i);
        goto OPT_exit;
    }
    if (strEqu("==", data)) {
        /* string compire */
        if ((type_arg1 == ARG_TYPE_STRING) && (type_arg2 == ARG_TYPE_STRING)) {
            outArg = arg_setInt(outArg, "",
                                strEqu(arg_getStr(arg1), arg_getStr(arg2)));
            goto OPT_exit;
        }
        /* default: int and float */
        outArg =
            arg_setInt(outArg, "",
                       (num1_f - num2_f) * (num1_f - num2_f) < (float)0.000001);
        goto OPT_exit;
    }
    if (strEqu("!=", data)) {
        outArg = arg_setInt(
            outArg, "",
            !((num1_f - num2_f) * (num1_f - num2_f) < (float)0.000001));
        goto OPT_exit;
    }
    if (strEqu(">=", data)) {
        outArg = arg_setInt(
            outArg, "",
            (num1_f > num2_f) ||
                ((num1_f - num2_f) * (num1_f - num2_f) < (float)0.000001));
        goto OPT_exit;
    }
    if (strEqu("<=", data)) {
        outArg = arg_setInt(
            outArg, "",
            (num1_f < num2_f) ||
                ((num1_f - num2_f) * (num1_f - num2_f) < (float)0.000001));
        goto OPT_exit;
    }
    if (strEqu("&", data)) {
        outArg = arg_setInt(outArg, "", num1_i & num2_i);
        goto OPT_exit;
    }
    if (strEqu("|", data)) {
        outArg = arg_setInt(outArg, "", num1_i | num2_i);
        goto OPT_exit;
    }
    if (strEqu("~", data)) {
        outArg = arg_setInt(outArg, "", ~num1_i);
        goto OPT_exit;
    }
    if (strEqu(">>", data)) {
        outArg = arg_setInt(outArg, "", num1_i >> num2_i);
        goto OPT_exit;
    }
    if (strEqu("<<", data)) {
        outArg = arg_setInt(outArg, "", num1_i << num2_i);
        goto OPT_exit;
    }
    if (strEqu(" and ", data)) {
        outArg = arg_setInt(outArg, "", num1_i && num2_i);
        goto OPT_exit;
    }
    if (strEqu(" or ", data)) {
        outArg = arg_setInt(outArg, "", num1_i || num2_i);
        goto OPT_exit;
    }
    if (strEqu(" not ", data)) {
        outArg = arg_setInt(outArg, "", !num1_i);
        goto OPT_exit;
    }
OPT_exit:
    arg_deinit(arg1);
    arg_deinit(arg2);
    if (NULL != outArg) {
        return outArg;
    }
    return NULL;
}

static Arg* VM_instruction_handler_DEF(PikaObj* self, VMState* vs, char* data) {
    char* methodPtr = vs->pc;
    int offset = 0;
    int thisBlockDeepth = __getThisBlockDeepth(vs->ASM_start, vs->pc, &offset);
    PikaObj* hostObj = vs->locals;
    uint8_t is_in_class = 0;
    /* use RunAs object */
    if (args_isArgExist(vs->locals->list, "__runAs")) {
        hostObj = args_getPtr(vs->locals->list, "__runAs");
        is_in_class = 1;
    }
    while (1) {
        if ((methodPtr[0] == 'B') &&
            (methodPtr[1] - '0' == thisBlockDeepth + 1)) {
            if (is_in_class) {
                class_defineObjectMethod(hostObj, data, (Method)methodPtr);
            } else {
                class_defineMethod(hostObj, data, (Method)methodPtr);
            }
            break;
        }
        offset += __gotoNextLine(methodPtr);
        methodPtr = vs->pc + offset;
    }
    return NULL;
}

static Arg* VM_instruction_handler_RET(PikaObj* self, VMState* vs, char* data) {
    /* exit jmp signal */
    vs->jmp = -999;
    Arg* returnArg = arg_copy(queue_popArg(vs->q0));
    method_returnArg(vs->locals->list, returnArg);
    return NULL;
}

static Arg* VM_instruction_handler_NEL(PikaObj* self, VMState* vs, char* data) {
    int offset = 0;
    int thisBlockDeepth = __getThisBlockDeepth(vs->ASM_start, vs->pc, &offset);
    char __else[] = "__else0";
    __else[6] = '0' + thisBlockDeepth;
    if (0 == args_getInt(self->list, __else)) {
        /* set __else flag */
        vs->jmp = fast_atoi(data);
    }
    return NULL;
}

static Arg* VM_instruction_handler_DEL(PikaObj* self, VMState* vs, char* data) {
    obj_removeArg(vs->locals, data);
    return NULL;
}

static Arg* VM_instruction_handler_EST(PikaObj* self, VMState* vs, char* data) {
    Arg* arg = obj_getArg(vs->locals, data);
    if (arg == NULL) {
        return arg_setInt(NULL, "", 0);
    }
    if (ARG_TYPE_NULL == arg_getType(arg)) {
        return arg_setInt(NULL, "", 0);
    }
    return arg_setInt(NULL, "", 1);
}

static Arg* VM_instruction_handler_BRK(PikaObj* self, VMState* vs, char* data) {
    /* break jmp signal */
    vs->jmp = -998;
    return NULL;
}

static Arg* VM_instruction_handler_CTN(PikaObj* self, VMState* vs, char* data) {
    /* continue jmp signal */
    vs->jmp = -997;
    return NULL;
}

static Arg* VM_instruction_handler_GLB(PikaObj* self, VMState* vs, char* data) {
    Arg* global_list_buff = NULL;
    char* global_list = args_getStr(vs->locals->list, "__gl");
    /* create new global_list */
    if (NULL == global_list) {
        args_setStr(vs->locals->list, "__gl", data);
        goto exit;
    }
    /* append to exist global_list */
    global_list_buff = arg_setStr(NULL, "", global_list);
    global_list_buff = arg_strAppend(global_list_buff, ",");
    global_list_buff = arg_strAppend(global_list_buff, data);
    args_setStr(vs->locals->list, "__gl", arg_getStr(global_list_buff));
    goto exit;
exit:
    if (NULL != global_list_buff) {
        arg_deinit(global_list_buff);
    }
    return NULL;
}

const VM_instruct_handler VM_instruct_handler_table[__INSTRCUTION_CNT] = {
#define __INS_TABLE
#include "__instruction_table.cfg"
};

static enum Instruct __getInstruct(char* line) {
#define __INS_COMPIRE
#include "__instruction_table.cfg"
    return NON;
}

int32_t pikaVM_runAsmLine(PikaObj* self,
                          VMParameters* locals,
                          VMParameters* globals,
                          char* pikaAsm,
                          int32_t lineAddr) {
    Args buffs = {0};
    VMState vs = {
        .locals = locals,
        .globals = globals,
        .q0 = NULL,
        .q1 = NULL,
        .jmp = 0,
        .pc = pikaAsm + lineAddr,
        .ASM_start = pikaAsm,
    };
    char* line = strsGetLine(&buffs, vs.pc);
    int32_t nextAddr = lineAddr + strGetSize(line) + 1;
    enum Instruct instruct;
    char invokeDeepth0[2] = {0}, invokeDeepth1[2] = {0};
    char* data;
    Arg* resArg;

    /* Found new script Line, clear the queues*/
    if ('B' == line[0]) {
        args_setErrorCode(locals->list, 0);
        args_setSysOut(locals->list, (char*)"");
        __clearInvokeQueues(locals);
        goto nextLine;
    }
    invokeDeepth0[0] = line[0];
    invokeDeepth1[0] = line[0] + 1;
    instruct = __getInstruct(line);
    data = line + 6;

    vs.q0 = args_getPtr(locals->list, invokeDeepth0);
    vs.q1 = args_getPtr(locals->list, invokeDeepth1);
    if (NULL == vs.q0) {
        vs.q0 = New_queue();
        args_setPtr(locals->list, invokeDeepth0, vs.q0);
    }
    if (NULL == vs.q1) {
        vs.q1 = New_queue();
        args_setPtr(locals->list, invokeDeepth1, vs.q1);
    }
    /* run instruct */
    resArg = VM_instruct_handler_table[instruct](self, &vs, data);
    if (NULL != resArg) {
        queue_pushArg(vs.q0, resArg);
    }
    goto nextLine;
nextLine:
    strsDeinit(&buffs);
    /* exit */
    if (-999 == vs.jmp) {
        return -99999;
    }
    /* break or continue */
    if ((-998 == vs.jmp) || (-997 == vs.jmp)) {
        int32_t loop_end_addr = __getAddrOffsetOfJUM(vs.pc);
        /* break */
        if (-998 == vs.jmp) {
            loop_end_addr += __gotoNextLine(vs.pc + loop_end_addr);
            return lineAddr + loop_end_addr;
        }
        if (-997 == vs.jmp) {
            loop_end_addr += __gotoLastLine(pikaAsm, vs.pc + loop_end_addr);
            return lineAddr + loop_end_addr;
        }
    }
    if (vs.jmp != 0) {
        return lineAddr + __getAddrOffsetFromJmp(pikaAsm, vs.pc, vs.jmp);
    }
    return nextAddr;
}

VMParameters* pikaVM_runAsmWithPars(PikaObj* self,
                                    VMParameters* locals,
                                    VMParameters* globals,
                                    char* pikaAsm) {
    int lineAddr = 0;
    int size = strGetSize(pikaAsm);
    args_setErrorCode(locals->list, 0);
    args_setSysOut(locals->list, (char*)"");
    while (lineAddr < size) {
        if (lineAddr == -99999) {
            break;
        }
        char* thisLine = pikaAsm + lineAddr;
        lineAddr = pikaVM_runAsmLine(self, locals, globals, pikaAsm, lineAddr);
        char* sysOut = args_getSysOut(locals->list);
        uint8_t errcode = args_getErrorCode(locals->list);
        if (!strEqu("", sysOut)) {
            __platform_printf("%s\r\n", sysOut);
        }
        if (0 != errcode) {
            Args buffs = {0};
            char* onlyThisLine = strsGetFirstToken(&buffs, thisLine, '\n');
            __platform_printf("[info] input commond: %s\r\n", onlyThisLine);
            strsDeinit(&buffs);
        }
    }
    __clearInvokeQueues(locals);

    return locals;
}

VMParameters* pikaVM_runAsm(PikaObj* self, char* pikaAsm) {
    return pikaVM_runAsmWithPars(self, self, self, pikaAsm);
}

VMParameters* pikaVM_run(PikaObj* self, char* multiLine) {
    Args buffs = {0};
    Args* buffs_p = &buffs;
    VMParameters* globals = NULL;
    char* pikaAsm = Parser_multiLineToAsm(&buffs, multiLine);
    if (NULL == pikaAsm) {
        __platform_printf("[error]: Syntax error.\r\n");
        globals = NULL;
        goto exit;
    }
    /* if do not save pikaAsm to flash */
    if ((1 == __platform_save_pikaAsm("")) &&
        (!obj_isArgExist(self, "__asm"))) {
        obj_setStr(self, "__asm", pikaAsm);
        strsDeinit(&buffs);
        buffs_p = NULL;
        pikaAsm = obj_getStr(self, "__asm");
    }
    globals = pikaVM_runAsm(self, pikaAsm);
    goto exit;
exit:
    if (NULL != buffs_p) {
        strsDeinit(&buffs);
    }
    return globals;
}

