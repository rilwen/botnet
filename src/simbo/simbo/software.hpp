#pragma once
#include <cassert>
#include <memory>
#include "exceptions.hpp"
#include "software_vendor.hpp"

namespace simbo {
	/// Abstrakcyjna klasa reprezentująca instalowane oprogramowanie.
	template <class Spec> class Software {
	public:
		typedef Spec spec_t;
		typedef SoftwareVendor<Spec> vendor_t;

		/// Specyfikacja i numer edycji.
		typedef std::pair<Spec, int> full_spec_t;

		/** \brief Konstruktor.
		\param vendor Dostawca oprogramowania.
		\param release Wersja edycji.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		Software(Spec spec, std::shared_ptr<vendor_t> vendor, int release)
			: spec_(spec), vendor_(vendor), release_(release) {
			if (!vendor) {
				throw std::invalid_argument((boost::locale::format("Null vendor for {1}") % spec).str());
			}
			if (!vendor->supports(spec)) {
				throw std::invalid_argument((boost::locale::format("Vendor {1} does not support {2}") % vendor->get_name() % spec).str());
			}
		}

		/// Wirtualny destruktor.
		virtual ~Software() {}

		/// Zwróć specyfikację oprogramowania.
		const Spec& get_spec() const {
			return spec_;
		}

		/// Zwróć numer edycji.
		int get_release_number() const {
			return release_;
		}

		/** Uaktualnij oprogramowanie do najnowszej edycji.
		\param datetime Punkt czasowy aktualizacji.
		\return Czy edycja została uaktualniona.
		\throw std::domain_error Jeżeli datetime wykracza poza zakres danych.
		*/
		bool update_release_number(DateTime datetime) {
			const int latest_release = vendor_->get_latest_release(get_spec(), datetime);
			assert(latest_release >= release_);
			if (latest_release > release_) {
				release_ = latest_release;
				return true;
			} else {
				return false;
			}			
		}
	private:
		Spec spec_;
		std::shared_ptr<vendor_t> vendor_;
		int release_;
	};
}