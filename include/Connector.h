#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Query.h"

inline auto query(asio::io_context *io, std::string_view host, std::string_view data)
{
	Session session(*io);

	// Connect to host
	tcp::resolver		 r(*io);
	tcp::resolver::query q(host.data(), "http");
	asio::connect(session.socket(), r.resolve(q));

	auto res = session.query(data);

	g_log.write(Logger::Catagory::INFO) << "Queried " << host;
	return res.content;
}

inline auto query_debug_get(std::string_view file)
{
	g_log.write(Logger::Catagory::WARN) << "Getting data from file " << file;
	std::ifstream in(file.data(), std::ios::in | std::ios::binary);
	if (!in)
		throw std::runtime_error(std::string("File ").append(file) + " doesn't exist.");

	// Get content from file (SLOW)
	std::stringstream s;
	s << in.rdbuf();
	return s.str();
}

inline auto query_debug_post(std::string_view file, std::string_view data)
{
	std::ofstream out(file.data(), std::ios::out | std::ios::binary);
	if (!out)
		throw std::runtime_error(std::string("File ").append(file) + " couldn't open.");

	out << data;
	return "#START\n#DATA\n{\"requesttimeout\": \"0\"}\n#END";
}
