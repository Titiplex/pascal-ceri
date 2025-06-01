#pragma once

#ifndef TYPE_H
#define TYPE_H

#include <string>
#include "utils.h"

TYPE ArrayUnit(const std::string& name);
TYPE Identifier();
TYPE Number();
TYPE Float();
TYPE Char();
TYPE String();

#endif //TYPE_H
