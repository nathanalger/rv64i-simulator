#include "DefaultRegistry.h"

void DefaultRegistry::init_default_instructions()
{
   register_rv64i();
   register_loads();
   register_stores();
   register_branch_jump();
}