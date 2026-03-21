#include "DefaultRegistry.h"

void DefaultRegistry::init_default_instructions()
{
   register_rv64i();
   register_loads();
   register_stores();
   register_branch_jump();
   register_rv64i_arithmetic();
   register_csr();
   register_float();
   register_mul();
   register_atomics();
}