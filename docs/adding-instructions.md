# Adding Instructions

Adding instructions to the simulated instruction set is relatively simple, and the process has been detailed below.

## 1. Add to InstructionType Enumerator

First, you will want to add the instruction to the InstructionType enumerator in `include/InstructionType.h`. The functions in InstructionExecutors will correspond to the order of isntructions in this enumerator.

## 2. Add to InstructionExecutors.h

Next, you will want to add the function declaration to the `include/InstructionExecutors.h` file. Each function should have the general structure of 

```cpp
void exec_example(const DecodedInstruction &, Processor &);
```

## 3. Implement new function

You then will need to implement the function in its own file or collection within the `src/instructions` folder. The function will look like so:

```cpp
void exec_example(const DecodedInstruction &inst, Processor &processor)
{
   // ...
}
```

## 4. Add to executor list

Finally, add the function to the executor list in the `src/Interpreter` file. You will see a list that looks like this:

```cpp
Interpreter::ExecFunc Interpreter::dispatch[] =
{
        exec_add,
        exec_sub,
        exec_addi,
        // ...
};
```

And notice that the InstructionType enumerator has the same exact order as this array:

```cpp
enum class InstructionType
{
   ADD,
   SUB,
   ADDI,
   // ...
};
```

It is **critical that you maintain this order**! This order determines which function is called based on the determined instruction type from the raw instruction input. With this in mind, you will add your function to this list in the same location that you added the InstructionType to the InstructionType enumerator.

```cpp
// Interpreter.cpp
Interpreter::ExecFunc Interpreter::dispatch[] =
{
        exec_add,
        exec_sub,
        exec_addi,
        exec_example, // <- Position 3
        // ...
};

// InstructionType.h
enum class InstructionType
{
   ADD,
   SUB,
   ADDI,
   EXAMPLE // <- Position 3
   // ...
};
```

Finally, add the instruction if necessary to the interpret function in Interpreter:

```cpp
InstructionType Interpreter::interpret(uint32_t instruction)
{

   uint32_t opcode = instruction & 0x7F;
   uint32_t funct3 = (instruction >> 12) & 0x7;
   uint32_t funct7 = (instruction >> 25) & 0x7F;

   switch (opcode)
   {

   case 0x33: // R-Type
      if (funct3 == 0 && funct7 == 0x00)
         return InstructionType::ADD;

      if (funct3 == 0 && funct7 == 0x20)
         return InstructionType::SUB;

      break;

      // ...

   return InstructionType::UNKNOWN;
   }
}
```

## 5. Sign Extended Immediate

If your function requires a sign extended immediate, the interpreter file has a function to assist with this.