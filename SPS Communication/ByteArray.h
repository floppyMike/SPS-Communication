#pragma once

#include "Includes.h"

enum DB_Type { REMOTE, LOCAL, MAX };

template<typename VarSeq>
using VariableSequences = std::array<VarSeq, MAX>;