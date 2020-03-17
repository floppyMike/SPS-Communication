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

#define BCCWIN
#include <nodavesimple.h>
#include <setport.h>
#include <openSocket.h>

#define ASIO_STANDALONE
#include <asio.hpp>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using asio::ip::tcp;
using namespace std::chrono_literals;