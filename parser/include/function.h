#pragma once
#ifndef FUNCTION_H
#define FUNCTION_H
#include "utils.h"
#include <string>

inline Functions Functions;

void FunctionDeclarationPart();

TYPE FuncProc(const std::string& name, bool wantResult);

#endif //FUNCTION_H
