#ifndef ENUMS_H
#define ENUMS_H

enum Type { K_BOOL, K_NUMBER, K_STRING, K_ANY, K_ARRAY, K_DICT, K_VOID };
enum ValueType { V_BOOL, V_INT, V_FLOAT, V_STRING, V_VOID };
enum Role { DEFAULT, PARAM, CALL_PARAM };

#endif