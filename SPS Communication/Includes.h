#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <queue>
#include <charconv>
#include <optional>
#include <numeric>
#include <array>
#include <ctime>

//Remove Max define
#define NOMINMAX

#include "nodavesimple.h"
#include "setport.h"
#include "openSocket.h"

//Remove uc define
#undef uc

#define ASIO_STANDALONE
#include "asio.hpp"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#undef CONST

using asio::ip::tcp;

using namespace std::chrono_literals;

namespace rj = rapidjson;
