#pragma once

#ifndef PROPOSITION_H
#define PROPOSITION_H

#include "utils.h"

TYPE Factor();

// Term := Factor {MultiplicativeOperator Factor}
TYPE Term();

TYPE SimpleExpression();

TYPE Expression();

#endif //PROPOSITION_H
