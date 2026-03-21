#pragma once

class DefaultRegistry
{
public:
   static void register_rv64i();
   static void register_loads();
   static void register_stores();
   static void register_branch_jump();
   static void register_rv64i_arithmetic();
   static void register_csr();
   static void register_mul();
   static void register_atomics();
   static void register_float();
   static void init_default_instructions();
};