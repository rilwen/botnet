#pragma once
#include <string>
#include "exceptions.hpp"
#include "software_vendor_config.hpp"

namespace simbo {
	/** Dostawca oprogramowania.

	\tparam Spec Specyfikacja oprogramowania (np. typ i wersja systemu operacyjnego, albo nazwa antywirusa).
	*/
	template <class Spec> class SoftwareVendor {
	public:		
		typedef SoftwareVendorConfig<Spec> config_t;		

		/** \brief Konstruktor.
		\param name Nazwa dostawcy, np. "Miko-Soft".
		\param cfg Konfiguracja dostawcy.
		\throw std::invalid_argument Jeżeli konfiguracja jest niewłaściwa albo nazwa pusta.
		*/
		SoftwareVendor(const std::string& name, const config_t& cfg);

		/// Wirtualny destruktor.
		virtual ~SoftwareVendor() {}

		/** Zwróć najnowszą edycję (release) oprogramowania.
		\param spec Specyfikacja oprogramowania.
		\param datetime Aktualny czas.
		\throw std::invalid_argument Jeżeli program określony przez spec nie jest wspierany przez dostawcę.
		\throw std::domain_error Jeżeli datetime wykracza poza zakres dla którego mamy dane.
		\return Number edycji.
		*/
		virtual int get_latest_release(Spec spec, DateTime datetime) const;

		/// Zwróć nazwę dostawcy.
		const std::string& get_name() const {
			return name_;
		}

		/// Sprawdź czy dostawca wspiera to oprogramowanie.
		bool supports(const Spec& spec) const {
			return release_schedules_.count(spec);
		}
	private:
		std::string name_;

		typename config_t::release_map_t release_schedules_;
	};

	template <class Spec> SoftwareVendor<Spec>::SoftwareVendor(const std::string& name, const config_t& cfg)
		: name_(name), release_schedules_(cfg.release_schedules) {
		if (name.empty()) {
			throw std::invalid_argument(boost::locale::translate("Empty SoftwareVendor name"));
		}
		try {
			cfg.validate();
		} catch (std::exception& e) {
			throw std::invalid_argument((boost::locale::format("Invalid config for software vendor \"{1}\": {2}") % name % e.what()).str());
		}
	}

	template <class Spec> int SoftwareVendor<Spec>::get_latest_release(Spec spec, DateTime datetime) const {
		auto schedule_it = release_schedules_.find(spec);
		if (schedule_it != release_schedules_.end()) {
			return schedule_it->second.interpolate_and_extrapolate(datetime);
		} else {
			throw std::invalid_argument((boost::locale::format("Vendor \"{1}\" does not support software {2}") % name_ % spec).str());
		}
	}
}