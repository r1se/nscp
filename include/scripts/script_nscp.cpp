#include <boost/foreach.hpp>

#include <utf8.hpp>
#include <nscapi/functions.hpp>
#include <nscapi/nscapi_core_helper.hpp>

#include <scripts/script_nscp.hpp>




std::list<std::string> scripts::nscp::settings_provider_impl::get_section(std::string section)
{
	return settings_.get_keys(section);
}

std::string scripts::nscp::settings_provider_impl::get_string(std::string path, std::string key, std::string value)
{
	return settings_.get_string(path, key, value);
}

void scripts::nscp::settings_provider_impl::set_string(std::string path, std::string key, std::string value)
{
	settings_.set_string(path, key, value);
}

bool scripts::nscp::settings_provider_impl::get_bool(std::string path, std::string key, bool value)
{
	return settings_.get_bool(path, key, value);
}

void scripts::nscp::settings_provider_impl::set_bool(std::string path, std::string key, bool value)
{
	settings_.set_bool(path, key, value);
}

int scripts::nscp::settings_provider_impl::get_int(std::string path, std::string key, int value)
{
	return settings_.get_int(path, key, value);
}

void scripts::nscp::settings_provider_impl::set_int(std::string path, std::string key, int value)
{
	settings_.set_int(path, key, value);
}

void scripts::nscp::settings_provider_impl::register_path(std::string path, std::string title, std::string description, bool advanced)
{
	settings_.register_path(path, title, description, advanced, false);
}

void scripts::nscp::settings_provider_impl::register_key(std::string path, std::string key, std::string type, std::string title, std::string description, std::string defaultValue)
{
	NSCAPI::settings_type iType = scripts::settings_provider::parse_type(type);
	settings_.register_key(path, key, iType, title, description, defaultValue, false, false);
}

void scripts::nscp::settings_provider_impl::save()
{
	settings_.save();
}

void scripts::nscp::nscp_runtime_impl::register_command(const std::string type, const std::string &command, const std::string &description) 
{
	if (type == tags::query_tag || type == tags::simple_query_tag) {
		nscapi::core_helper::core_proxy proxy(core_, plugin_id);
		proxy.register_command(command, description);
	}
}

bool scripts::nscp::core_provider_impl::submit_simple_message(const std::string channel, const std::string command, const NSCAPI::nagiosReturn code, const std::string & message, const std::string & perf, std::string & result)
{
	std::string request, response;
	nscapi::protobuf::functions::create_simple_submit_request(channel, command, code, message, perf, request);
	bool ret = core_->submit_message(channel, request, response) == NSCAPI::isSuccess;
	nscapi::protobuf::functions::parse_simple_submit_response(response, result);
	return ret;
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::simple_query(const std::string &command, const std::list<std::string> & argument, std::string & msg, std::string & perf)
{
	std::string request, response;
	nscapi::protobuf::functions::create_simple_query_request(command, argument, request);
	bool ret = core_->query(request, response) == NSCAPI::isSuccess;
	nscapi::protobuf::functions::parse_simple_query_response(response, msg, perf);
	return ret;
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::exec_simple_command(const std::string target, const std::string command, const std::list<std::string> &argument, std::list<std::string> & result)
{
	std::string request, response;
	nscapi::protobuf::functions::create_simple_exec_request(command, argument, request);
	bool ret = core_->exec_command(target, request, response) == NSCAPI::isSuccess;
	nscapi::protobuf::functions::parse_simple_exec_response(response, result);
	return ret;
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::exec_command(const std::string target, const std::string &request, std::string &response)
{
	return core_->exec_command(target, request, response);
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::query(const std::string &request, std::string &response)
{
	return core_->query(request, response);
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::submit(const std::string target, const std::string &request, std::string &response)
{
	return core_->submit_message(target, request, response);
}

NSCAPI::nagiosReturn scripts::nscp::core_provider_impl::reload(const std::string module)
{
	return core_->reload(module);
}

void scripts::nscp::core_provider_impl::log(NSCAPI::log_level::level level, const std::string file, int line, const std::string message)
{
	core_->log(level, file, line, message);
}

const std::string scripts::nscp::tags::simple_query_tag = "simple:query";
const std::string scripts::nscp::tags::simple_exec_tag = "simple:exec";
const std::string scripts::nscp::tags::simple_submit_tag = "simple:submit";
const std::string scripts::nscp::tags::query_tag = "query";
const std::string scripts::nscp::tags::exec_tag = "exec";
const std::string scripts::nscp::tags::submit_tag = "submit";

