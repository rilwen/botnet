#include "email_provider.hpp"
#include "exceptions.hpp"

namespace simbo {
	EmailProvider::EmailProvider(const std::string& name, double email_filtering_efficiency)
		: name_(name), email_filtering_efficiency_(email_filtering_efficiency) {
		if (!(email_filtering_efficiency >= 0 && email_filtering_efficiency <= 1)) {
			throw std::domain_error(boost::locale::translate("Email filtering efficiency not in [0, 1] range"));
		}
	}
}
