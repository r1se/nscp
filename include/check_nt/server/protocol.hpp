#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ssl/context.hpp>

#include <socket/socket_helpers.hpp>
#include <socket/connection.hpp>
#include <socket/server.hpp>

#include "handler.hpp"
#include "parser.hpp"

namespace check_nt {

	static const int socket_bufer_size = 8096;
	static const bool debug_trace = true;
	struct read_protocol : public boost::noncopyable {

		typedef std::vector<char> outbound_buffer_type;
		typedef boost::array<char, socket_bufer_size>::iterator iterator_type;

		enum state {
			none,
			connected,
			got_request,
			done
		};

		socket_helpers::connection_info info_;
		check_nt::server::handler *handler_;
		state current_state_;

		std::vector<char> data_;
		check_nt::server::parser parser_;

		read_protocol(socket_helpers::connection_info info, check_nt::server::handler *handler) 
			: info_(info)
			, handler_(handler)
			, current_state_(none)
		{}

		inline void set_state(state new_state) {
			current_state_ = new_state;
		}

		bool on_accept(boost::asio::ip::tcp::socket& socket) {
			std::list<std::string> errors;
			std::string s = socket.remote_endpoint().address().to_string();
			if (info_.allowed_hosts.is_allowed(socket.remote_endpoint().address(), errors)) {
				log_debug(__FILE__, __LINE__, "Accepting connection from: " + s);
				return true;
			} else {
				BOOST_FOREACH(const std::string &e, errors) {
					log_error(__FILE__, __LINE__, e);
				}
				log_error(__FILE__, __LINE__, "Rejected connection from: " + s);
				return false;
			}
		}

		bool on_connect() {
			set_state(connected);
			return true;
		}

		bool wants_data() {
			return current_state_ == connected;
		}
		bool has_data() {
			return current_state_ == got_request;
		}

		bool on_read(char *begin, char *end) {
			while (begin != end) {
				bool result;
				iterator_type old_begin = begin;
				boost::tie(result, begin) = parser_.digest(begin, end);
				if (begin == old_begin) {
					log_error(__FILE__, __LINE__, "Digester failed to parse chunk, giving up.");
					return false;
				}
				if (result) {
					check_nt::packet response;
					try {
						check_nt::packet request = parser_.parse();
						response = handler_->handle(request);
					} catch (const std::exception &e) {
						response = handler_->create_error(_T("Exception processing request: ") + utf8::to_unicode(e.what()));
					} catch (...) {
						response = handler_->create_error(_T("Exception processing request"));
					}

					data_ = response.get_buffer();
					set_state(got_request);
					return true;
				}
			}
			return true;
		}
		void on_write() {
			set_state(done);
		}

		std::vector<char> get_outbound() const {
			return data_;
		}

		socket_helpers::connection_info get_info() const {
			return info_;
		}

		void log_debug(std::string file, int line, std::string msg) const {
			handler_->log_debug("check_nt", file, line, msg);
		}
		void log_error(std::string file, int line, std::string msg) const {
			handler_->log_error("check_nt", file, line, msg);
		}
	};

	namespace server {
		typedef socket_helpers::server::server<read_protocol, socket_bufer_size> server;
	}
} // namespace check_nt
