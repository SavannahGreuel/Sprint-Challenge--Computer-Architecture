#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_LEN 6

#define SP 7

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, int argc, char *argv[]) // make ./ls8 examples/print8.ls8
{
  if (argc < 2)
  {
    printf("File does not exist.\n");
    exit(1);
  }

  char *file = argv[1];
  FILE *fp = fopen(file, "r");

  if (fp == NULL)
  {
    printf("File does not exist");
    exit(1);
  }
  else
  { 
    char file_line[1024];
    int address = 0;

    while (fgets(file_line, sizeof(file_line), fp) != NULL)
    {
      char *endptr;
      unsigned char val;
      val = strtol(file_line, &endptr, 2);

      // if empty line continue
      if (file_line == NULL)
      {
        continue;
      }

      cpu->ram[address] = val;
      address++;
    }
  }
  // close file
  fclose(fp);
}

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char index)
{
  return cpu->ram[index];
}

void cpu_ram_write(struct cpu *cpu, unsigned char index, unsigned char value)
{
  cpu->ram[index] = value;
}

// cpu push
void push_stack(struct cpu *cpu, unsigned char value)
{
  cpu->registers[SP]--;
  cpu_ram_write(cpu, cpu->registers[SP], value);
}

// cpu pop
unsigned char pop_stack(struct cpu *cpu)
{
  unsigned char ret = cpu->ram[cpu->registers[SP]];
  cpu->registers[SP]++;
  return ret;
}

//

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    // TODO
    cpu->registers[regA] *= cpu->registers[regB];
    break;

  // TODO: implement more ALU ops
  case ALU_ADD:
    cpu->registers[regA] += cpu->registers[regB];
    break;

  case ALU_CMP:
    if (cpu->registers[regA] == cpu->registers[regB])
    {
      cpu->FL = 1;
    }
    else if (cpu->registers[regA] > cpu->registers[regB])
    {
      cpu->FL = 0;
    }
    break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT INSTRUCTIONS
  // create ops
  unsigned char operandA;
  unsigned char operandB;

  while (running)
  {
    // TODO

    // 1. Get the value of the current INSTRUCTIONS (in address PC).
    unsigned char INSTRUCTIONS = cpu->ram[cpu->PC];

    // 2. Figure out how many operands this next INSTRUCTIONS requires
    unsigned int num_operands = INSTRUCTIONS >> 6;

    // 3. Get the appropriate value(s) of the operands following this INSTRUCTIONS
    if (num_operands == 2)
    {
      operandA = cpu_ram_read(cpu, (cpu->PC + 1) & 0xff);
      operandB = cpu_ram_read(cpu, (cpu->PC + 2) & 0xff);
    }
    else if (num_operands == 1)
    {
      operandA = cpu_ram_read(cpu, (cpu->PC + 1) & 0xff);
    }

    // 4. switch() over it to decide on a course of action.
    switch (INSTRUCTIONS)
    {
    case HLT:
      running = 0;
      break;

    case PRN:
      printf("%d\n", cpu->registers[operandA]);
      break;

    case LDI:
      cpu->registers[operandA] = operandB;
      break;

    case MUL:
      alu(cpu, ALU_MUL, operandA, operandB);
      break;

    case PUSH:
      push_stack(cpu, cpu->registers[operandA]);
      break;

    case POP:
      cpu->registers[operandA] = pop_stack(cpu);
      break;

    case ADD:
      alu(cpu, ALU_ADD, operandA, operandB);
      break;

    case CALL:
      push_stack(cpu, cpu->PC + 1);
      cpu->PC = cpu->registers[operandA] - 1;
      break;

    case RET:
      cpu->PC = pop_stack(cpu);
      break;

    case CMP:
      alu(cpu, ALU_CMP, operandA, operandB);
      break;

    case JMP:
      cpu->PC = cpu->registers[operandA];
      cpu->PC += 1;
      break;

    case JEQ:
      if (cpu->FL == 1)
      {
        cpu->PC = cpu->registers[operandA];
        cpu->PC -= 1;
      }
      break;

    case JNE:
      if (cpu->FL != 1)
      {
        cpu->PC = cpu->registers[operandA];
        cpu->PC -= 1;
      }
      break;
      
    default:
      break;
    }
    // 5. Do whatever the INSTRUCTIONS should do according to the spec.
    // 6. Move the PC to the next INSTRUCTIONS.
    cpu->PC += num_operands + 1;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registersisters
  // set all values to 0
  cpu->PC = 0;
  cpu->FL = 0;

  cpu->registers[SP] = ADDR_EMPTY_STACK;

  memset(cpu->registers, 0, sizeof(cpu->registers));
  memset(cpu->ram, 0, sizeof(cpu->ram));
}