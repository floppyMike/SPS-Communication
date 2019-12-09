#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <thread>

#define BCCWIN
#include <nodavesimple.h>
#include <setport.h>

#define ASIO_STANDALONE
#include <asio.hpp>

using asio::ip::tcp;