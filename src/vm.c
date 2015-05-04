/***************************************************************************
 *   Copyright (C) 2015 by                                                 *
 *   - Carlos Eduardo Millani (carloseduardomillani@gmail.com)             *
 *   - Edson Borin (edson@ic.unicamp.br)                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
 * Description: This file implements the virtual machine. 
 */
#include "HAL.h"
#include "vm.h"
#include "syscall.h"
#include <inttypes.h>

typedef struct decoded_instruction
{
  uint8_t op;
  union {
    struct R {
      // op == 000000
      uint8_t rs, rt, rd, shamt, funct;
    } R;
    struct I {
      uint8_t rs, rt; 
      uint16_t immediate;
    } I;
    struct J {
      uint32_t address;
    } J;
  };
} decoded_instruction_t;


/*
 * Central processing unit. It continuously fetches and executes
 * instructions.
 */
void vm_cpu()
{
	/*DUVIDA: Utilizar setjmp e longjmp para tratamento de erros? Programa receberá sinais? Onde colocar setjmp se for usar?*/
	/* Register file. */
	uint32_t RF[32];
	uint32_t HI = 0, LO = 0;  
	uint32_t PC = 0;
	uint8_t halted = 0;


	while (!halted) 
	{

		uint32_t instr = fetch(PC);
		uint8_t op = (instr >> 26) & 0x3F;


		switch (op) 
		{
			uint8_t rs = (instr >> 21) & 0x1F;
			uint8_t rt = (instr >> 16) & 0x1F;
			uint8_t rd = (instr >> 11) & 0x1F;
			case 0x0: { // 000000 => Register encoding.
				//uint8_t rs = (instr >> 21)   && 0x1F;
				//uint8_t rt = (instr >> 16)   && 0x1F;
				//uint8_t rd = (instr >> 11)   && 0x1F;
				uint8_t shamt = (instr >> 6) & 0x1F;
				uint8_t funct = (instr >> 0) & 0x3F;
			  
				switch (funct) {
					case 0b100000: { // add		100000	ArithLog	$d = $s + $t
						RF[rd] = RF[rs] + RF[rt];
						break;
					}
					case 0b100001: { // addu	100001	ArithLog	$d = $s + $t ///////////Unsigned?
						RF[rd] = RF[rs] + RF[rt];
						break;
					}
					case 0b100100: { // and		100100	ArithLog	$d = $s & $t
						RF[rd] = RF[rs] & RF[rt];
						break;
					}
					case 0b011010: { // div		011010  DivMult		lo = $s / $t; hi = $s % $t
						LO = RF[rs] / RF[rt];
						HI = RF[rs] % RF[rt];
						break;
					}
					case 0b011011: { // divu	011011  DivMult		lo = $s / $t; hi = $s % $t
						LO = RF[rs] / RF[rt];
						HI = RF[rs] % RF[rt];  
						break;
					}
					case 0b011000: { // mult	011000  DivMult		hi:lo = $s * $t
						uint64_t mult = (uint64_t)RF[rs] * (uint64_t) RF[rt];//Melho desempenho que efetuar o calculo duas vezes mas ocupa mais memoria, possivel gordura
						HI = (mult >> 32) | 0xFFFFFFFF;
						LO = mult | 0xFFFFFFFF;
						break;
					}
					case 0b011001: { // multu	011001	DivMult		hi:lo = $s * $t
						uint64_t mult = (uint64_t)RF[rs] * (uint64_t) RF[rt];
						HI = (mult >> 32) | 0xFFFFFFFF;
						LO = mult | 0xFFFFFFFF;  
						break;
					}
					case 0b100111: { // nor		100111	ArithLog	$d = ~($s | $t)
						RF[rd] = ~(RF[rs] | RF[rt]);
						break;
					}
					case 0b100101: { // or		100101	ArithLog	$d = $s | $t
						RF[rd] = RF[rs] | RF[rt];
						break;
					}
					case 0b000000: { // sll		000000	Shift		$d = $t << a
						RF[rd] = RF[rt] << shamt;
						break;
					}
					case 0b000100: { // sllv	000100	ShiftV		$d = $t << $s
						RF[rd] = RF[rt] << RF[rs];
						break;
					}
					case 0b000011: { // sra		000011	Shift		$d = $t >> a /////////////Duvida
						RF[rd] = RF[rs] >> shamt;
						break;
					}
					case 0b000111: { // srav	000111	ShiftV		$d = $t >> $s
						RF[rd] = RF[rt] >> RF[rs];
						break;
					}
					case 0b000010: { // srl		000010	Shift		$d = $t >>> a ////////////Duvida
						RF[rd] = RF[rt] >> shamt;
						break;
					}
					case 0b000110: { // srlv	000110	ShiftV		$d = $t >>> $s
						RF[rd] = RF[rt] >> RF[rs];  
						break;
					}
					case 0b100010: { // sub		100010	ArithLog	$d = $s - $t
						RF[rd] = RF[rs] - RF[rt];  
						break;
					}
					case 0b100011: { // subu	100011	ArithLog	$d = $s - $t
						RF[rd] = RF[rs] - RF[rt];    
						break;
					}
					case 0b100110: { // xor		100110  ArithLog	$d = $s ^ $t
						RF[rd] = RF[rs] ^ RF[rt];    
						break;
					}
					case 0b101010: { // slt		101010	ArithLog	$d = ($s < $t)
						RF[rd] = (RF[rs] < RF[rt]);
						break;
					}
					case 0b101001: { // sltu	101001	ArithLog	$d = ($s < $t)
						RF[rd] = (RF[rs] < RF[rt]);
						break;
					}
					case 0b001001: { // jalr	001001	JumpR		$31 = pc; pc = $s
						RF[31] = PC;
						PC = RF[rs];
						break;
					}
					case 0b001000: { // jr		001000	JumpR		pc = $s 
						PC = RF[rs];
						break;
					}
					case 0b010000: { // mfhi	010000	MoveFrom	$d = hi
						RF[rd] = HI;
						break;
					}
					case 0b010010: { // mflo	010010	MoveFrom	$d = lo
						RF[rd] = LO;
						break;
					}
					case 0b010001: { // mthi	010001	MoveTo		hi = $s
						HI = RF[rs];
						break;
					}
					case 0b010011: { // mtlo	010011	MoveTo		lo = $s
						LO = RF[rs];
						break;
					}
					break;
				}      
				break; // case 0x0
			}
			
			//Immediate encoding
			uint16_t immediate = (instr >> 0) & 0xFFFF;
			case 0b001000: { //addi    001000  ArithLogI       $t = $s + SE(i)
				RF[rt] = RF[rs] + immediate;
				break;
			}
			case 0b001001: { //addiu   001001  ArithLogI       $t = $s + SE(i)
				RF[rt] = RF[rs] + immediate;
				break;
			}
			case 0b001100: { //andi    001100  ArithLogI       $t = $s & ZE(i)
			    RF[rt] = RF[rs] & immediate;
				break;
			}
			case 0b001101: { //ori     001101  ArithLogI       $t = $s | ZE(i)
				RF[rt] = RF[rs] | immediate;
				break;
			}
			case 0b001110: { //xori    001110  ArithLogI       $d = $s ^ ZE(i)
				RF[rd] = RF[rs] ^ immediate;
				break;
			}
			case 0b011001: { //lhi     011001  LoadI   HH ($t) = i /////////////// Duvida + o que eh SE e ZE sinal e sem sinal?
			  	RF[rt] = immediate;
				break;
			}
			case 0b011000: { //llo     011000  LoadI   LH ($t) = i
			  	RF[rt] = immediate;			  
				break;
			}
			case 0b001010: { //slti    001010  ArithLogI       $t = ($s < SE(i))
			  	RF[rt] = RF[rs] < immediate;
				break;
			}
			case 0b001011: { //sltiu   001011  ArithLogI       $t = ($s < SE(i)) //CORRIGIDO
				RF[rt] = RF[rs] < immediate;
				break;
			}
			case 0b000100: { //beq     000100  Branch  if ($s == $t) pc += i << 2
			  	PC = (RF[rs] == RF[rt])?PC+(immediate << 2):PC;
				break;
			}
			case 0b000111: { //bgtz    000111  BranchZ if ($s > 0) pc += i << 2
			  	PC = (RF[rs] > 0)?PC+(immediate << 2):PC;
				break;
			}
			case 0b000110: { //blez    000110  BranchZ if ($s <= 0) pc += i << 2
			  	PC = (RF[rs] <= 0)?PC+(immediate << 2):PC;
				break;
			}
			case 0b000101: { //bne     000101  Branch  if ($s != $t) pc += i << 2
			  	PC = (RF[rs] != RF[rt])?PC+(immediate << 2):PC;
				break;
			}
			case 0b100000: { //lb      100000  LoadStore       $t = SE (MEM [$s + i]:1)
			  	RF[rt] = (VM_memory[RF[rs] + immediate]& 0x7F)  | (uint32_t)(VM_memory[RF[rs] + immediate] & 0x80)<<24;
				break;
			}
			case 0b100100: { //lbu     100100  LoadStore       $t = ZE (MEM [$s + i]:1)
			  	RF[rt] = (VM_memory[RF[rs] + immediate]) & 0xFF;
				break;
			}
			case 0b100001: { //lh      100001  LoadStore       $t = SE (MEM [$s + i]:2)
			  	RF[rt] = (((VM_memory[RF[rs] + immediate]) | (VM_memory[RF[rs] + immediate + 1])) & 0x7FFF) | ((VM_memory[RF[rs] + immediate + 1]) & 0x8000) << 16;
				break;
			}
			case 0b100101: { //lhu     100101  LoadStore       $t = ZE (MEM [$s + i]:2)
				RF[rt] = (((VM_memory[RF[rs] + immediate]) | (VM_memory[RF[rs] + immediate + 1])) & 0xFFFF);
				break;
			}
			case 0b100011: { //lw      100011  LoadStore       $t = MEM [$s + i]:4
			  	RF[rt] = (VM_memory[RF[rs] + immediate]) | (VM_memory[RF[rs] + immediate + 1]) | (VM_memory[RF[rs] + immediate + 2])| (VM_memory[RF[rs] + immediate + 3]);
				break;
			}
			case 0b101000: { //sb      101000  LoadStore       MEM [$s + i]:1 = LB ($t)
			  	VM_memory[RF[rs] + immediate] = (uint8_t)(RF[rt] & 0xFF);
				break;
			}
			case 0b101001: { //sh      101001  LoadStore       MEM [$s + i]:2 = LH ($t)
			  	VM_memory[RF[rs] + immediate] = (uint8_t)((RF[rt] & 0xFF00) >> 8);
				VM_memory[RF[rs] + immediate + 1] = (uint8_t)(RF[rt] & 0xFF);
				break;
			}
			case 0b101011: { //sw      101011  LoadStore       MEM [$s + i]:4 = $t
			  	VM_memory[RF[rs] + immediate] = (uint8_t)((RF[rt] & 0xFF000000) >> 24);
				VM_memory[RF[rs] + immediate + 1] = (uint8_t)((RF[rt] & 0xFF0000) >> 16);
			  	VM_memory[RF[rs] + immediate + 2] = (uint8_t)((RF[rt] & 0xFF00) >> 8);
				VM_memory[RF[rs] + immediate + 3] = (uint8_t)(RF[rt] & 0xFF);
				break;
			}
			uint32_t address = (instr >> 0) & 0x3FFFFFF;
			//Jump encoding
			
			case 0b000010: { //j       000010  Jump    pc += i << 2
				PC += address << 2;
				break;
			}
			case 0b000011: { //jal     000011  Jump    $31 = pc; pc += i << 2
				RF[31] = PC;
				PC += address << 2;
				break;
			}
			case 0b011010: { //trap    011010  Trap    Dependent on operating system; different values for immed26 specify different operations. See the list of traps for information on what the different trap codes do.
				syscall((uint8_t)(address&0xFF));
				break;
			}
			
			uint8_t op = (instr >> 26) & 0x3F;
			break; //op
		}
		PC++;//Increments PC to fetch the next instruction
	}
}
uint32_t fetch(uint32_t PC)
{
	return (VM_memory[PC] >> 26) & 0x3F;
}