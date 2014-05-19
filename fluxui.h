
#ifndef FLUXUI_H_
#define FLUXUI_H_

#include <stdarg.h>
#include "ppapi/c/pp_var.h"

struct PP_Var CStrToVar(const char* str);
char* VprintfToNewString(const char* format, va_list args);
char* PrintfToNewString(const char* format, ...);
struct PP_Var PrintfToVar(const char* format, ...);
uint32_t VarToCStr(struct PP_Var var, char* buffer, uint32_t length);

#endif /* FLUXUI_H_ */
