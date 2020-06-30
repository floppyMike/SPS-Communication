/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
- create reasonable installation
- documentation
*/

//#define SPS_NOT_AVAILABLE
//#define SERVER_NOT_AVAILABLE

#include "Includes.h"
#include "Interpeter.h"
#include "Logging.h"
#include "ServerInterface.h"
#include "ProgramParameters.h"
#include "ByteArray.h"
#include "VariableSequence.h"

#ifndef SPS_NOT_AVAILABLE
#include "SPS.h"
#include "SPSIO.h"
#endif

Logger g_log;

class RunTime
{
public:
	RunTime(asio::io_context *io, std::string_view host)
		: m_host(host)
		, m_server(io)
	{
	}

	auto pair_up(std::string_view sps_host, std::string_view sps_port)
	{
#ifndef SPS_NOT_AVAILABLE
		m_sps.connect(sps_host, sps_port);
#endif // SPS_NOT_AVAILABLE
		m_server.pair_up(m_host);
	}

	auto request_varsequence()
	{
		auto json = m_server.get(m_host);

		Interpreter interpret;
		auto		root_data = json.var("data");

		if (auto s = root_data.safe_var("settings"); s.has_value())
			interpret.prepare_seqs(s.value());
		else
			interpret.prepare_seqs();

		interpret.prepare_vars("interpret.txt");

		if (auto s = root_data.safe_var("data"); s.has_value())
			interpret.fill_vars(s.value());

		return interpret.give_seqs();
	}

	auto init_variables(VariableSequences &vars)
	{
#ifndef SPS_NOT_AVAILABLE
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::MUTABLE], m_sps.in(vars[DB_Type::MUTABLE].db(), vars[DB_Type::MUTABLE].total_byte_size()));
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::CONST], m_sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
		ByteArrayConverter::from_byte_array(vars[DB_Type::MUTABLE],
											ByteArrayConverter::to_byte_array(vars[DB_Type::MUTABLE]));
		ByteArrayConverter::from_byte_array(vars[DB_Type::CONST],
											ByteArrayConverter::to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE
	}

	auto update_sps(VariableSequences &vars)
	{
		auto data_write = ByteArrayConverter::to_byte_array(vars[DB_Type::MUTABLE]);
		g_log.write(Logger::Catagory::INFO) << "Bytes to be written:\n" << data_write;

#ifndef SPS_NOT_AVAILABLE
		m_sps.out(vars[DB_Type::MUTABLE].db(), data_write);
		ByteArrayConverter::from_byte_array(
			vars[DB_Type::CONST], m_sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
		ByteArrayConverter::from_byte_array(vars[DB_Type::CONST],
											ByteArrayConverter::to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE
	}

	auto post_varsequence(const VariableSequences &var) { m_server.post(m_host, var); }

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

auto main(int argc, char **argv) -> int
{
	// Handle command parameters
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
				  << " SPS_IP SPS_Port Host\n"
					 "\"SPS_IP\": IP address of the SPS.\n"
					 "\"SPS_Port\": Port on which the SPS sits. This is mostly 102.\n"
					 "\"Host\": Server name of machine where ProjectSpyder is running.\n";
		//"\"-p\": Looks for file specified and uses its authentication code for the server.\n";
		//"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	// Speed up io
	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	// network io sync
	asio::io_context io;

	RunTime rt(&io, argv[3]);

	// Pairing
	for (char err_c = 1; true;) try
		{
			rt.pair_up(argv[1], argv[2]);
			break;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during pairing: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}

	// Setup
	for (char err_c = 1; true;) try
		{
			auto vars = rt.request_varsequence();

			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of mutable variables:\n"
												<< vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of constant variables:\n"
												<< vars[DB_Type::CONST];

			rt.init_variables(vars);
			rt.post_varsequence(vars);

			break;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during setup: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}

	// Update
	for (char err_c = 1; true;) try
		{
			auto vars = rt.request_varsequence();

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read (values will be overwritten):\n"
												<< vars[DB_Type::CONST];

			rt.update_sps(vars);
			rt.post_varsequence(vars);

			err_c = 1;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during updating SPS: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}
}
