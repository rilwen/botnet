#include <boost/version.hpp>
#include <Eigen/Core>

// Sprawdź w trakcie kompilacji czy dostępne są wymagane wersje zewnętrznych bibliotek.

static_assert(BOOST_VERSION >= 106700, "Wymagana wersja Boost >= 1.67.00");
static_assert(EIGEN_VERSION_AT_LEAST(3, 3, 4), "Wymagana wersja Eigen++ >= 3.3.4");