#include "http_client.h"
#include <open62541.h>
#include "../macros.h"

namespace gateway
{

	HTTP_Client::HTTP_Client(
		const std::string & jsonConfig
	) :
		m_jsonConfig(jsonConfig),
		m_endpoint("null"),
		m_username(""),
		m_password(""),
		m_outputFile(), // output, std::ofstream::out | std::ofstream::binary | std::ofstream::app
		m_verbose(false)
	{
		// Get config strings as JSON objects
		json jsonCfg = json::parse(m_jsonConfig);

		// Fetch client configuration
		m_endpoint = jsonCfg["endpoint"].get<std::string>();
		m_username = jsonCfg["username"].get<std::string>();
		m_password = jsonCfg["password"].get<std::string>();
		m_outputFile = std::ofstream(jsonCfg["output"].get<std::string>(), std::ofstream::out | std::ofstream::binary | std::ofstream::app);
		m_verbose = jsonCfg["verbose"].get<bool>();

		LOG("HTTP_Client initialized successfully, endpoint: %s, output: %s\n", UA_DateTime_now(), m_endpoint.c_str(), jsonCfg["output"].get<std::string>().c_str());
	}

	HTTP_Client::~HTTP_Client()
	{
		m_outputFile.close();
	}

	json HTTP_Client::getJSON(const std::string & path)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);
		json result = "{}"_json;

		// Input stream for request output
		std::ostringstream sstream;
		curl_ios<std::ostringstream> cwriter(sstream);

		// Create the curl easy handle
		curl_easy ceasy(cwriter);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Accept: application/json");

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);
		if (m_username.empty() == false)
		{
			ceasy.add<CURLOPT_USERNAME>(m_username.c_str());
			ceasy.add<CURLOPT_PASSWORD>(m_password.c_str());
			ceasy.add<CURLOPT_HTTPAUTH>(CURLAUTH_BASIC | CURLAUTH_DIGEST);
		}

		try
		{
			// Excecute the request
			ceasy.perform();

			if (sstream.str().size() > 2)
			{
				// Get the result JSON
				result = json::parse(sstream.str());
			}
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Exception: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Exception: %s\n", UA_DateTime_now(), e.what());
		}

		return result;
	}

	void HTTP_Client::sendJSON(const std::string & path, HTTP_Request_t request, json & data)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);
		std::string data_str = data.dump();

		// Input stream for request output
		curl_ios<std::ostream> cwriter(m_outputFile);

		// Create the curl easy handle
		curl_easy ceasy(cwriter);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Content-Type: application/json");

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);
		if (m_username.empty() == false)
		{
			ceasy.add<CURLOPT_USERNAME>(m_username.c_str());
			ceasy.add<CURLOPT_PASSWORD>(m_password.c_str());
			ceasy.add<CURLOPT_HTTPAUTH>(CURLAUTH_BASIC | CURLAUTH_DIGEST);
		}
		ceasy.add<CURLOPT_CUSTOMREQUEST>((request == HTTP_POST) ? "POST" : "PUT");
		ceasy.add<CURLOPT_POSTFIELDS>(data_str.c_str());
		ceasy.add<CURLOPT_POSTFIELDSIZE>(-1L);

		try
		{
			// Excecute the request
			ceasy.perform();
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Exception: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Exception: %s\n", UA_DateTime_now(), e.what());
		}
	}

	void HTTP_Client::sendREQ(const std::string & path, HTTP_Request_t request)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);

		// Input stream for request output
		curl_ios<std::ostream> cwriter(m_outputFile);

		// Create the curl easy handle
		curl_easy ceasy(cwriter);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Accept: application/json");

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);
		if (m_username.empty() == false)
		{
			ceasy.add<CURLOPT_USERNAME>(m_username.c_str());
			ceasy.add<CURLOPT_PASSWORD>(m_password.c_str());
			ceasy.add<CURLOPT_HTTPAUTH>(CURLAUTH_BASIC | CURLAUTH_DIGEST);
		}
		ceasy.add<CURLOPT_CUSTOMREQUEST>((request == HTTP_DELETE) ? "DELETE" : "GET");

		try
		{
			// Excecute the request
			ceasy.perform();
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Exception: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Exception: %s\n", UA_DateTime_now(), e.what());
		}
	}

	bool HTTP_Client::isVerbose() const
	{
		return m_verbose;
	}

}