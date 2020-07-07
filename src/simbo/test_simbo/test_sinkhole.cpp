#include <cassert>
#include <sstream>
#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/local_network.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/sinkhole.hpp"
#include "mock_botnet_config.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class SinkholeTest : public ::testing::Test {
protected:
	SinkholeTest()
		: rng(new MockRNG()),
		host(11, std::make_unique<MockOS>(), false, true),
		t_message(0.6),
		sinkhole(Sinkhole::EPOCH, true),
		incremental_sinkhole(Sinkhole::EPOCH, true)
	{
		LocalNetwork ln(1, LocalNetworkType::CORPORATE, 12);
		host.set_local_network(&ln);
		PublicConnectionStaticAddress public_connection(IpAddress::from_string("0.0.0.10"));
		ln.set_public_connection(&public_connection);
		Schedule schedule(std::vector<seconds_t>({ 0, 1, 2, 3 }));
		const auto start_date = schedule.get_start();
		Context ctx(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
		host.turn_on(ctx, 0);
		host.infect(ctx, 0.5);
		sinkhole.add_data_point(sinkhole.get_data_from_bot(&host, t_message));
		incremental_sinkhole.add_data_point(sinkhole.get_data_from_bot(&host, t_message));
		incremental_sinkhole.dump_full_bin_incremental(incremental_stream);
		assert(host.get_bot_state().get_value() == BotState::SETTING_UP);
		sinkhole.add_data_point(sinkhole.get_data_from_bot(&host, t_message + 24 * 3600));
		incremental_sinkhole.add_data_point(sinkhole.get_data_from_bot(&host, t_message + 24 * 3600));
		incremental_sinkhole.dump_full_bin_incremental(incremental_stream);
	}

	std::shared_ptr<MockRNG> rng;
	MockHost host;
	seconds_t t_message;
	// Instancje Sinkhole testowane bezpośrednio.
	Sinkhole sinkhole;
	Sinkhole incremental_sinkhole;
	std::stringstream incremental_stream;
};

TEST_F(SinkholeTest, get_number_data_points) {
	ASSERT_EQ(2, sinkhole.get_number_data_points());
	ASSERT_EQ(0, sinkhole.get_number_dumped_data_points());	
	ASSERT_EQ(0, incremental_sinkhole.get_number_data_points());
	ASSERT_EQ(2, incremental_sinkhole.get_number_dumped_data_points());

}

TEST_F(SinkholeTest, dump_full_csv) {
	std::stringstream ss;
	sinkhole.dump_full_csv(ss);
	ASSERT_EQ("time,bot_state,ip,host_id,local_network_type,is_fixed,country\n0.6,0,10,11,1,1,12\n86400.6,0,10,11,1,1,12\n", ss.str());
}

TEST_F(SinkholeTest, dump_reduced_csv) {
	std::stringstream ss;
	sinkhole.dump_reduced_csv(ss);
	ASSERT_EQ("date,hour,country,num_ip_addresses,num_messages,num_hosts\n1970-Jan-01,0,12,1,1,1\n1970-Jan-01,-1,12,1,1,1\n1970-Jan-02,0,12,1,1,1\n1970-Jan-02,-1,12,1,1,1\n", ss.str());
}

TEST_F(SinkholeTest, dump_full_bin) {
	std::stringstream ss;
	sinkhole.dump_full_bin(ss);
	ASSERT_EQ(ss.str(), incremental_stream.str());
	const int header_length = 128;
	const int row_length = 32;
	const int n_rows = 2;
	ASSERT_EQ(header_length + n_rows * row_length, ss.str().size());
	ASSERT_EQ("SIMBOBIN", ss.str().substr(0, 8));
	ASSERT_EQ(3, *reinterpret_cast<uint64_t*>(&ss.str()[8])); // wersja formatu
	ASSERT_EQ(row_length, *reinterpret_cast<uint64_t*>(&ss.str()[16]));
	ASSERT_EQ(n_rows, *reinterpret_cast<uint64_t*>(&ss.str()[24]));
	for (int i = 32; i < header_length; ++i) {
		ASSERT_EQ(0, ss.str()[i]) << i;
	}
	ASSERT_EQ(t_message, *reinterpret_cast<double*>(&ss.str()[header_length])); // timestamp
	ASSERT_EQ(10, *reinterpret_cast<int64_t*>(&ss.str()[header_length + 8])); // ip
	ASSERT_EQ(host.get_id(), *reinterpret_cast<int32_t*>(&ss.str()[header_length + 16])); // host_id
	ASSERT_EQ(0, *reinterpret_cast<int8_t*>(&ss.str()[header_length + 20])); // bot_state
	ASSERT_EQ(1, *reinterpret_cast<int8_t*>(&ss.str()[header_length + 21])); // local_network_type
	ASSERT_EQ(1, *reinterpret_cast<int8_t*>(&ss.str()[header_length + 22])); // is_fixed
	ASSERT_EQ(12, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + 23])); // country
	ASSERT_EQ(1, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + 24])); // payload_release
	for (int i = 25; i < 32; ++i) {
		ASSERT_EQ(0, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + i])); // wypełnienie zerami
	}
	ASSERT_EQ(t_message + 24 * 3600, *reinterpret_cast<double*>(&ss.str()[header_length + row_length])); // timestamp
	ASSERT_EQ(10, *reinterpret_cast<int64_t*>(&ss.str()[header_length + row_length + 8])); // ip
	ASSERT_EQ(host.get_id(), *reinterpret_cast<int32_t*>(&ss.str()[header_length + row_length + 16])); // host_id
	ASSERT_EQ(0, *reinterpret_cast<int8_t*>(&ss.str()[header_length + row_length + 20])); // bot_state
	ASSERT_EQ(1, *reinterpret_cast<int8_t*>(&ss.str()[header_length + row_length + 21])); // local_network_type
	ASSERT_EQ(1, *reinterpret_cast<int8_t*>(&ss.str()[header_length + row_length + 22])); // is_fixed
	ASSERT_EQ(12, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + row_length + 23])); // country
	ASSERT_EQ(1, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + row_length + 24])); // payload_release
	for (int i = 25; i < 32; ++i) {
		ASSERT_EQ(0, *reinterpret_cast<uint8_t*>(&ss.str()[header_length + row_length + i])); // wypełnienie zerami
	}
}