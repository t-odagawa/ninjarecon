#include "NTBMSummary.hh"

#include <boost/log/trivial.hpp>

NTBMSummary::NTBMSummary() {
  NTBMSummary::Clear("C");
}

void NTBMSummary::Clear(Option_t *option) {
  spill_pot_ = -1.;
  
}
