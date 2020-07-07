#include <gtest/gtest.h>
#include "simbo/conditional_markov_process_impl.hpp"
#include "simbo/constant_process.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class MockConditionalMarkovProcess : public ConditionalMarkovProcessImpl<double, bool> {
public:
	MockConditionalMarkovProcess(seconds_t first_time, seconds_t last_time, double value_true, double value_false);
protected:
	markov_process_ptr get_markov_process(const bool& parameter) const override;
private:
	markov_process_ptr true_;
	markov_process_ptr false_;
};

MockConditionalMarkovProcess::MockConditionalMarkovProcess(seconds_t first_time, seconds_t last_time, double value_true, double value_false)
	: ConditionalMarkovProcessImpl<double, bool>(first_time, last_time),
	true_(new ConstantProcess<double>(value_true)),
	false_(new ConstantProcess<double>(value_false)) {

}

MockConditionalMarkovProcess::markov_process_ptr MockConditionalMarkovProcess::get_markov_process(const bool& parameter) const {
	if (parameter) {
		return true_;
	} else {
		return false_;
	}
}

TEST(ConditionalMarkovProcessImpl, constructor) {
	MockConditionalMarkovProcess process(-10, 10, 0.5, 1.5);
	ASSERT_EQ(-10, process.get_first_time());
	ASSERT_EQ(10, process.get_last_time());
}

TEST(ConditionalMarkovProcessImpl, constructor_throws) {
	ASSERT_THROW(MockConditionalMarkovProcess(1, 1, 0, 0), std::domain_error);
	ASSERT_THROW(MockConditionalMarkovProcess(2, 1, 0, 0), std::domain_error);
}

TEST(ConditionalMarkovProcessImpl, get_next_state_throws) {
	MockConditionalMarkovProcess process(-10, 10, 0.5, 1.5);
	MockRNG rng;
	ASSERT_THROW(process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(15, true), MockConditionalMarkovProcess::Conditions(5, true)), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(-14, true), MockConditionalMarkovProcess::Conditions(6, true)), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(6, true), MockConditionalMarkovProcess::Conditions(16, true)), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(5, false), MockConditionalMarkovProcess::Conditions(15, true)), std::domain_error);
}

TEST(ConditionalMarkovProcessImpl, get_next_state) {
	MockConditionalMarkovProcess process(-10, 10, 0.5, 1.5);
	MockRNG rng;
	ASSERT_EQ(0.5, process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(-5, true), MockConditionalMarkovProcess::Conditions(5, true)));
	ASSERT_EQ(1.5, process.get_next_state(rng, 0.5, MockConditionalMarkovProcess::Conditions(-5, true), MockConditionalMarkovProcess::Conditions(5, false)));
}
