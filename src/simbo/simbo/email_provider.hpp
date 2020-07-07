#pragma once
#include <string>

namespace simbo {

	/// Dostawca konta pocztowego.
	class EmailProvider {
	public:
		virtual ~EmailProvider() {}

		/** \brief Konstruktor
		\param name Nazwa.
		\param email_filtering_efficiency Ułamek zainfekowanych emaili które zostaną odfiltrowane przez dostawcę poczty
		\throw std::domain_error Jeżeli email_filtering_efficiency poza przedziałem [0, 1].
		*/
		EmailProvider(const std::string& name, double email_filtering_efficiency);

		/// Zwróć ułamek zainfekowanych emaili które zostaną odfiltrowane przez dostawcę poczty.
		double get_email_filtering_efficiency() const {
			return email_filtering_efficiency_;
		}

		const std::string& get_name() const {
			return name_;
		}

	private:
		/// Nazwa dostawcy.
		std::string name_;

		double email_filtering_efficiency_;
	};
}
