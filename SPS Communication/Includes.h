#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <queue>
#include <charconv>
#include <optional>

#define BCCWIN
#include <nodavesimple.h>
#include <setport.h>
#include <openSocket.h>

#define ASIO_STANDALONE
#include <asio.hpp>

using asio::ip::tcp;
using std::chrono_literals;