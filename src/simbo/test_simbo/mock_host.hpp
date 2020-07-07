#pragma once
#include "simbo/host.hpp"

namespace simbo {
	class MockHost : public Host {
	public:
		MockHost(id_t id, operating_system_ptr&& os, bool always_on, bool is_fixed)
			: Host(id, std::move(os), always_on),
			local_network_(nullptr),
			is_fixed_(is_fixed)
		{}

		const_local_network_ptr get_local_network() const {
			return local_network_;
		}

		void set_local_network(const_local_network_ptr local_network) {
			local_network_ = local_network;
		}

		bool is_fixed() const override {
			return is_fixed_;
		}

		HostType get_type() const override {
			return HostType::DESKTOP;
		}
	private:
		const_local_network_ptr local_network_;

		bool is_fixed_;
	};
}
