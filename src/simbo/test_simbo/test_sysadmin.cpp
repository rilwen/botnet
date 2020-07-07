#include <gtest/gtest.h>
#include "simbo/host.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/sysadmin.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"

using namespace simbo;

TEST(Sysadmin, constructor) {
	Sysadmin luke(std::make_shared<PausingJumpMarkovProcess>(std::unique_ptr<JumpMarkovProcess>(new SimplePoissonProcess(1./(24 * 3600))), 0));
	ASSERT_TRUE(luke.get_hosts().empty());
}

TEST(Sysadmin, add_host) {
	Sysadmin peter(std::make_shared<PausingJumpMarkovProcess>(std::unique_ptr<JumpMarkovProcess>(new SimplePoissonProcess(1. / (24 * 3600))), 0));
	Sysadmin::host_ptr host = Sysadmin::host_ptr(new MockHost(1, Host::operating_system_ptr(new MockOS()), false, true));
	Host* host_ptr = host.get(); // Chcemy zachować dostęp do hosta.
	peter.add_host(std::move(host));
	ASSERT_EQ(nullptr, host);
	ASSERT_EQ(1u, peter.get_hosts().size());
	ASSERT_NE(nullptr, peter.get_hosts()[0]);
	ASSERT_EQ(&peter, &host_ptr->get_manager());
}
