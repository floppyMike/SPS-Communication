#pragma once

#include "Includes.h"
#include "Logging.h"

class ISPSRequest
{
public:
	ISPSRequest(daveConnection* c)
		: m_con(c)
	{
	}

	virtual ~ISPSRequest()
	{
		for (auto& i : m_result)
			daveFreeResults(&i);
	}

	auto results()
	{
		std::vector<uint8_t> arr;

		for (auto& i : m_result)
			for (auto* iter_res = i.results; iter_res != i.results + i.numResults; ++iter_res)
				arr.insert(arr.end(), iter_res->bytes, iter_res->bytes + iter_res->length);

		return arr;
	}

protected:
	daveConnection* m_con;
	PDU m_p;

	std::vector<daveResultSet> m_result;
};

class SPSReadRequest : ISPSRequest
{
public:
	static constexpr int PDU_READ_LIMIT = 222;

	SPSReadRequest(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareReadRequest(c, &m_p);
	}

	void add_vars(int db, int len)
	{
		assert(len <= PDU_READ_LIMIT && "Read request to SPS to large.");

		if (m_curr_size + len >= PDU_READ_LIMIT)
			send();

		m_curr_size += len;

		daveAddVarToReadRequest(&m_p, daveDB, db, 0, len);
	}

	void send()
	{
		m_result.resize(m_result.size() + 1);

		m_curr_size = 0;
		if (auto res = daveExecReadRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));
	}

	using ISPSRequest::results;

private:
	int m_curr_size = 0;
};

class SPSWriteRequest : ISPSRequest
{
public:
	static constexpr size_t PDU_WRITE_LIMIT = 222;

	SPSWriteRequest(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareWriteRequest(c, &m_p);
	}

	void add_vars(int db, const std::vector<char>& arr)
	{
		assert(arr.size() <= PDU_WRITE_LIMIT && "Read request to SPS to large.");

		if (m_curr_size + arr.size() >= PDU_WRITE_LIMIT)
			send();

		m_curr_size += arr.size();

		daveAddVarToWriteRequest(&m_p, daveDB, db, 0, arr.size(), const_cast<char*>(arr.data()));
	}

	void send()
	{
		m_result.resize(m_result.size() + 1);

		m_curr_size = 0;
		if (auto res = daveExecWriteRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));
	}

	using ISPSRequest::results;

private:
	int m_curr_size = 0;
};