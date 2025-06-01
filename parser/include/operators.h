#pragma once

#ifndef OPERATORS_H
#define OPERATORS_H

#include "utils.h"

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator();

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator();

OPREL RelationalOperator();

#endif //OPERATORS_H
