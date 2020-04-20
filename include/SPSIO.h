#pragma once

#include "Includes.h"
#include "Logging.h"

template<typename FuncPack, template<typename> class... Ex>
class SPSRequester : public Ex<SPSRequester<FuncPack, Ex...>>...
{
public:
	static constexpr int PDU_LIMIT = 222;

	SPSRequester(daveConnection* c)
		: m_con(c)
	{
		FuncPack::prep_request(c, &m_p);
	}

	virtual ~SPSRequester()
	{
		for (auto& i : m_result)
			daveFreeResults(&i);
	}

	void add_vars(int db, int len)
	{
		_handle_len_(len);
		FuncPack::add_var(&m_p, db, len);
	}

	void add_vars(int db, const std::vector<uint8_t>& arr)
	{
		_handle_len_(std::size(arr));
		FuncPack::add_var(&m_p, db, std::size(arr), const_cast<uint8_t*>(&arr.front()));
	}

	void send()
	{
		m_result.resize(m_result.size() + 1);

		m_curr_size = 0;
		if (auto res = FuncPack::request(m_con, &m_p, &m_result.back()); res != 0)
			throw std::runtime_error(daveStrerror(res));
	}

	auto results()
	{
		std::vector<uint8_t> arr;

		for (auto& i : m_result)
			for (auto* iter_res = i.results; iter_res != i.results + i.numResults; ++iter_res)
				arr.insert(arr.end(), iter_res->bytes, iter_res->bytes + iter_res->length);

		return arr;
	}

private:
	daveConnection* m_con;
	PDU m_p;

	int m_curr_size = 0;

	std::vector<daveResultSet> m_result;

	
	void _handle_len_(int len)
	{
		assert(len <= PDU_LIMIT && "Read request to SPS to large.");

		if (m_curr_size + len >= PDU_LIMIT)
			send();

		m_curr_size += len;
	}
};

class TSPSRead
{
public:
	TSPSRead() = delete;

	static void prep_request(daveConnection* c, PDU* p)
	{
		davePrepareReadRequest(c, p);
	}

	static void add_var(PDU* p, int db, int bytes)
	{
		daveAddVarToReadRequest(p, daveDB, db, 0, bytes);
	}

	static int request(daveConnection* c, PDU* p, daveResultSet* rs)
	{
		return daveExecReadRequest(c, p, rs);
	}
};

class TSPSWrite
{
public:
	TSPSWrite() = delete;

	static void prep_request(daveConnection* c, PDU* p)
	{
		davePrepareWriteRequest(c, p);
	}

	static void add_var(PDU* p, int db, int bytes, uint8_t* arr)
	{
		daveAddVarToWriteRequest(p, daveDB, db, 0, bytes, arr);
	}

	static int request(daveConnection* c, PDU* p, daveResultSet* rs)
	{
		return daveExecWriteRequest(c, p, rs);
	}
};
