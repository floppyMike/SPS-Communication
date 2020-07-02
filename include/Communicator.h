#pragma once

#include "Includes.h"
#include "ServerInterface.h"
#include "Interpeter.h"
#include "ByteArray.h"
#include "VariableSequence.h"

#ifndef SPS_NOT_AVAILABLE
#	include "SPS.h"
#	include "SPSIO.h"
#endif

class RunTime
{
public:
	RunTime(asio::io_context *io, std::string_view host)
		: m_host(host)
		, m_server(io)
	{
	}

	auto initialize_sps(std::string_view sps_host, std::string_view sps_port)
	{
#ifndef SPS_NOT_AVAILABLE
		m_sps.connect(sps_host, sps_port);
#endif // SPS_NOT_AVAILABLE
	}

	auto pair_up() { m_server.pair_up(m_host); }

	auto request_varsequence()
	{
		// Get json from server
		auto json = m_server.get(m_host);

		// Merge json and interpret into managable variable sequences
		Interpreter interpret;
		auto		root_data = json.var("data");

		if (auto s = root_data.safe_var("settings"); s.has_value())
			interpret.prepare_seqs(s.value());
		else
			interpret.prepare_seqs();

		interpret.prepare_vars("interpret.txt");

		if (auto s = root_data.safe_var("data"); s.has_value())
			interpret.fill_vars(s.value());

		// Give sequences
		return interpret.give_seqs();
	}

	auto init_variables(VariableSequences &vars)
	{
#ifndef SPS_NOT_AVAILABLE
		// Get SPS bytes and merge it with known variable sequences
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::MUTABLE], m_sps.in(vars[DB_Type::MUTABLE].db(), vars[DB_Type::MUTABLE].total_byte_size()));
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::CONST], m_sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
		// Test conversion
		ByteArrayConverter::from_byte_array(vars[DB_Type::MUTABLE],
											ByteArrayConverter::to_byte_array(vars[DB_Type::MUTABLE]));
		ByteArrayConverter::from_byte_array(vars[DB_Type::CONST],
											ByteArrayConverter::to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE
	}

	auto update_sps(VariableSequences &vars)
	{
		// Convert and report mutable bytes
		auto data_write = ByteArrayConverter::to_byte_array(vars[DB_Type::MUTABLE]);
		g_log.write(Logger::Catagory::INFO) << "Bytes to be written:\n" << data_write;

#ifndef SPS_NOT_AVAILABLE
		// Write mutable bytes to SPS and read constant bytes from SPS
		m_sps.out(vars[DB_Type::MUTABLE].db(), data_write);
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::CONST], m_sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
		// Test conversion
		ByteArrayConverter::from_byte_array(vars[DB_Type::CONST],
											ByteArrayConverter::to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE
	}

	auto post_varsequence_update(const VarSequence &var) { m_server.post(m_host, "UPDATE", var); }
	auto post_varsequence_replace(const VariableSequences &var) { m_server.post(m_host, "PUT", var); }

private:
	std::string_view m_host;

	Server<
#ifndef SERVER_NOT_AVAILABLE
		Pairer, Getter, Poster
#else
		PairerDebug, GetterDebug, PosterDebug
#endif
		>
		m_server;

#ifndef SPS_NOT_AVAILABLE
	SPSConnection m_sps;
#endif
};