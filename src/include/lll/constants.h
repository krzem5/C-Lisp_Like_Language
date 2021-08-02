#ifndef __LLL_CONSTANTS_H__
#define __LLL_CONSTANTS_H__ 1
#include <stdint.h>



#define LLL_ERROR_UNKNOWN 0
#define LLL_ERROR_INTERNAL_STACK_OVERFLOW 1
#define LLL_ERROR_UNMATCHED_OPEN_PARENTHESES 2
#define LLL_ERROR_UNMATCHED_CLOSE_PARENTHESES 3
#define LLL_ERROR_UNMATCHED_OPEN_QUOTE 4
#define LLL_ERROR_UNMATCHED_CURLY_OPEN_BRACKETS 5
#define LLL_ERROR_UNMATCHED_CURLY_CLOSE_BRACKETS 6
#define LLL_ERROR_UNMATCHED_QUOTES 7
#define LLL_ERROR_EMPTY_CHAR_STRING 8
#define LLL_ERROR_UNTERMINATED_CHAR_STRING 9
#define LLL_ERROR_UNTERMINATED_ESCAPE_SEQUENCE 10
#define LLL_ERROR_UNKNOWN_ESCAPE_CHARACTER 11
#define LLL_ERROR_UNTERMINATED_HEX_ESCAPE_SEQUENCE 12
#define LLL_ERROR_UNKNOWN_HEXADECIMAL_CHARCTER 13
#define LLL_ERROR_UNKNOWN_DECIMAL_CHARCTER 14
#define LLL_ERROR_UNKNOWN_OCTAL_CHARCTER 15
#define LLL_ERROR_UNKNOWN_BINARY_CHARCTER 16
#define LLL_ERROR_UNKNOWN_SYMBOL 17
#define LLL_ERROR_UNKNOWN_IDENTIFIER_CHARACTER 18
#define LLL_ERROR_UNEXPECTED_CHARACTER 19
#define LLL_ERROR_SYMBOL_TOO_LONG 20
#define LLL_ERROR_NO_SYMBOL 21
#define LLL_ERROR_TOO_MANY_ARGUMENTS 22
#define LLL_ERROR_TOO_MANY_STATEMENTS 23
#define LLL_ERROR_UNKNOWN_IDENTIFIER 24
#define LLL_ERROR_INTERNAL_FUNCTION_NAME_TOO_LONG 25
#define LLL_ERROR_INTERNAL_FUNCTION_NAME_NOT_ASCII 26
#define LLL_ERROR_UNKNOWN_INTERNAL_FUNCTION 27
#define LLL_ERROR_NO_STACK 28
#define LLL_ERROR_DIVISION_BY_ZERO 29
#define LLL_ERROR_INVALID_FILE_FORMAT 30
#define LLL_ERROR_INVALID_INSTRUCTION 31
#define LLL_ERROR_STACK_CORRUPTED 32
#define LLL_ERROR_INVALID_INSTRUCTION_INDEX 33
#define LLL_ERROR_ASSERTION 255
#define LLL_MAX_COMPILATION_ERROR LLL_ERROR_INTERNAL_FUNCTION_NAME_NOT_ASCII

#define LLL_RETURN_ERROR 0
#define LLL_RETURN_NO_ERROR 1

#define LLL_OBJECT_TYPE_UNKNOWN 0
#define LLL_OBJECT_TYPE_CHAR 1
#define LLL_OBJECT_TYPE_INT 2
#define LLL_OBJECT_TYPE_FLOAT 3
#define LLL_OBJECT_TYPE_STRING 4
#define LLL_OBJECT_TYPE_IDENTIFIER 5
#define LLL_OBJECT_TYPE_PRINT 6
#define LLL_OBJECT_TYPE_INPUT 7
#define LLL_OBJECT_TYPE_AND 8
#define LLL_OBJECT_TYPE_OR 9
#define LLL_OBJECT_TYPE_NOT 10
#define LLL_OBJECT_TYPE_ASSIGN 11
#define LLL_OBJECT_TYPE_FUNC 12
#define LLL_OBJECT_TYPE_INTERNAL_FUNC 13
#define LLL_OBJECT_TYPE_CALL 14
#define LLL_OBJECT_TYPE_IF 15
#define LLL_OBJECT_TYPE_FOR 16
#define LLL_OBJECT_TYPE_WHILE 17
#define LLL_OBJECT_TYPE_LOOP 18
#define LLL_OBJECT_TYPE_ADD 19
#define LLL_OBJECT_TYPE_SUB 20
#define LLL_OBJECT_TYPE_MULT 21
#define LLL_OBJECT_TYPE_DIV 22
#define LLL_OBJECT_TYPE_FLOOR_DIV 23
#define LLL_OBJECT_TYPE_MOD 24
#define LLL_OBJECT_TYPE_BIT_AND 25
#define LLL_OBJECT_TYPE_BIT_OR 26
#define LLL_OBJECT_TYPE_BIT_XOR 27
#define LLL_OBJECT_TYPE_BIT_NOT 28
#define LLL_OBJECT_TYPE_LESS 29
#define LLL_OBJECT_TYPE_LESS_EQUAL 30
#define LLL_OBJECT_TYPE_EQUAL 31
#define LLL_OBJECT_TYPE_NOT_EQUAL 32
#define LLL_OBJECT_TYPE_MORE 33
#define LLL_OBJECT_TYPE_MORE_EQUAL 34
#define LLL_OBJECT_TYPE_RETURN 35
#define LLL_OBJECT_TYPE_EXIT 36
#define LLL_OBJECT_TYPE_OPERATION_LIST 37
#define LLL_OBJECT_TYPE_DEBUG_DATA 38
#define LLL_OBJECT_TYPE_NOP 255
#define LLL_IS_OBJECT_TYPE_NOT_TYPE(o) ((o)->t>LLL_OBJECT_TYPE_IDENTIFIER)

#define LLL_MAX_SHORT_IDENTIFIER_LENGTH 15
#define LLL_IDENTIFIER_GET_ARRAY_ID(i) ((i)&0xf)
#define LLL_IDENTIFIER_GET_ARRAY_INDEX(i) ((i)>>4)
#define LLL_IDENTIFIER_ADD_INDEX(i,j) ((i)+((j)<<4))
#define LLL_CREATE_IDENTIFIER(i,j) (((i)<<4)|(j))

#define LLL_ASSEMBLY_INSTRUCTION_TYPE_NOP 0
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_POP 1
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_POP_TWO 2
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_ROT 3
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_ROT_POP 4
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_DUP 5
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_INT 6
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_MINUS_ONE 7
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_ZERO 8
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_ONE 9
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_TWO 10
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_THREE 11
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_FOUR 12
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_FLOAT 13
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_CHAR 14
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_LOAD 15
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_LOADS 16
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE 17
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_POP 18
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_MINUS_ONE 19
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_ZERO 20
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_ONE 21
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_TWO 22
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_THREE 23
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_FOUR 24
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JMP 25
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JB 26
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JBE 27
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JA 28
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JAE 29
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JE 30
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JNE 31
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JZ 32
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_JNZ 33
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_NOT 34
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_INC 35
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_DEC 36
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_ADD 37
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_SUB 38
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_MULT 39
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_DIV 40
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_FDIV 41
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_MOD 42
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_AND 43
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_OR 44
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_XOR 45
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_INV 46
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT 47
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_CHAR 48
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_STR 49
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_VAR 50
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_CALL 51
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_CALL_POP 52
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET 53
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_INT 54
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_ZERO 55
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_FLOAT 56
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_CHAR 57
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_STR 58
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_VAR 59
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_END 60
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_END_ZERO 61
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_END_ONE 62
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RESERVED0 126
#define LLL_ASSEMBLY_INSTRUCTION_TYPE_RESERVED1 127
#define LLL_ASSEMBLY_INSTRUCTION_INPLACE 128
#define LLL_ASSEMBLY_INSTRUCTION_RELATIVE 128
#define LLL_ASSEMBLY_INSTRUCTION_GET_TYPE(ai) ((ai)->t&0x7f)
#define LLL_ASSEMBLY_INSTRUCTION_IS_INPLACE(ai) ((ai)->t>>7)
#define LLL_ASSEMBLY_INSTRUCTION_IS_RELATIVE(ai) ((ai)->t>>7)

#define LLL_RUNTIME_OBJECT_TYPE_INT 0
#define LLL_RUNTIME_OBJECT_TYPE_FLOAT 1
#define LLL_RUNTIME_OBJECT_TYPE_CHAR 2
#define LLL_RUNTIME_OBJECT_TYPE_STRING 3
#define LLL_RUNTIME_OBJECT_RESERVED0 128
#define LLL_RUNTIME_OBJECT_GET_TYPE(r) ((r)->t&0x7f)

#define LLL_COMPARE_RESULT_BELOW 0
#define LLL_COMPARE_RESULT_EQUAL 1
#define LLL_COMPARE_RESULT_ABOVE 2
#define LLL_COMPARE_RESULT_ZERO 3
#define LLL_COMPARE_RESULT_NONZERO 4
#define LLL_COMPARE_RESULT_ERROR 255

#define LLL_END_OF_DATA UINT16_MAX
#define LLL_READ_FROM_INPUT_DATA_STREAM(is) ((is)->rf((is)))
#define LLL_READ_BUFFER_FROM_INPUT_DATA_STREAM(is,bf,sz) ((is)->rbf((is),(bf),(sz)))
#define LLL_INPUT_DATA_STREAM_RESTART_LINE(is,lp) ((is)->rlf((is),(lp)))
#define LLL_GET_INPUT_DATA_STREAM_OFFSET(is) ((is)->_off)
#define LLL_GET_INPUT_DATA_STREAM_LINE_NUMBER(is) ((is)->_lc)
#define LLL_GET_INPUT_DATA_STREAM_LINE_OFFSET(is) ((is)->_loff)

#define LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,c) ((os)->wcf((os),(c)))
#define LLL_WRITE_STRING_TO_OUTPUT_DATA_STREAM(os,s) ((os)->wsf((os),(s)))
#define LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,bf,sz) ((os)->wf((os),(bf),(sz)))

#define LLL_MAX_FUNCTION_INDEX UINT32_MAX
#define LLL_MAX_OBJECT_OFFSET UINT32_MAX
#define LLL_MAX_SCOPE UINT32_MAX
#define LLL_MAX_STRING_INDEX UINT32_MAX
#define LLL_MAX_VARIABLE_INDEX UINT32_MAX



#endif
