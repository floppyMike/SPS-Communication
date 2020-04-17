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

#define NOMINMAX

#define BCCWIN
#include "libnodave-0.8.5/nodavesimple.h"
#include "libnodave-0.8.5/setport.h"
#include "libnodave-0.8.5/openSocket.h"

#define ASIO_STANDALONE
#include "asio-1.12.2/include/asio.hpp"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#undef CONST

using asio::ip::tcp;

using namespace std::chrono_literals;

namespace rj = rapidjson;
