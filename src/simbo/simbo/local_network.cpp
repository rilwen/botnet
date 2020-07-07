#include "exceptions.hpp"
#include "host.hpp"
#include "local_network.hpp"
#include "log.hpp"

namespace simbo {
	LocalNetwork::LocalNetwork(id_t id, LocalNetworkType typ, int country) 
		: id_(id), type_(typ), public_connection_(nullptr), country_(country) {
		if (country <= 0) {
			throw std::domain_error((boost::locale::format("Invalid country code: {1}") % country).str());
		}
	}

	// host jest na pewno != nullptr dzięki Passkey.
	void LocalNetwork::add_host(const Host* host, seconds_t time) {
		if (host->get_local_network() != this) {
			throw std::invalid_argument(boost::locale::translate("Host not connected to this network"));
		}
		hosts_.insert(host);
		get_tracker()->info("{},LocalNetwork,{},AddHost,{}", time, get_id(), host->get_id());
	}

	// host jest na pewno != nullptr dzięki Passkey.
	void LocalNetwork::remove_host(const Host* host, seconds_t time) {
		if (host->get_local_network() != this) {
			throw std::invalid_argument(boost::locale::translate("Host not connected to this network"));
		}
		hosts_.erase(host);
		get_tracker()->info("{},LocalNetwork,{},DelHost,{}", time, get_id(), host->get_id());
	}
}

