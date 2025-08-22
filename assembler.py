#!/usr/bin/env python3

import sys
import re

if len(sys.argv) < 2:
	print(f"Filename not specified")
	exit(1)
if len(sys.argv) > 2:
	print(f"Unexpected argument: \"{sys.argv[2]}\"")
	exit(1)

filename = sys.argv[1]

with open(filename, 'r') as file:
	lines = file.readlines()

INSTRUCTION_SIZE = 4
MACRO_KEYWORD = '.macro'
DELIMTER_SPLIT = '(\s+|,)'
COMMENT_SYMBOL = ';'
OPCODES = {
	"load": 0,
	"store": 1,
	"add": 2,
	"sub": 3,
	"and": 4,
	"or": 5,
	"xor": 6,
	"mov": 7,
	"beq": 8,
	"bne": 9,
	"brn": 10,
}
REGS = {
    f"r{i}": i for i in range(8)
}
REGS["eq"] = 8
REGS["pc"] = 9
	

def resolve_macros(lines: list[str]):
	macro_map = {}
	for i in range(len(lines)):

		# Remove comments in the same pass for "efficiency"
		idx = lines[i].find(COMMENT_SYMBOL)
		if idx > -1:
			lines[i] = lines[i][:idx]

		lines[i] = lines[i].strip()
		if not lines[i]:
			continue
		
		parts = lines[i].split(maxsplit=2)
		if parts and parts[0] == MACRO_KEYWORD:
			macro_map[parts[1]] = parts[-1]

		cleaned = []
		for token in re.split(DELIMTER_SPLIT, lines[i]):
			if token and token[0] == "#":
				cleaned.append(f"#{macro_map.get(token[1:], token[1:])}")
			else:
				cleaned.append(macro_map.get(token, token))

		lines[i] = "".join(cleaned)

def strip_imm_if_able(imm: str) -> str:
	if imm and imm[0] == '#':
		return imm[1:]
	return imm

def gather_loops_and_instrs(lines: list[str]) -> tuple[dict, list[str], list[int]]:
	loops = {}
	instrs = []
	addrs = []
	pc = 0
	
	# Pass 1: collect labels
	for line in lines:
		if m := re.match(r'\s*@(\w+)', line):
			loops[m.group(1)] = pc
			continue
		
		tokens = line.split()
		if tokens and tokens[0] in OPCODES:
			instrs.append(line)
			addrs.append(pc)
			pc += 4
	
	return loops, instrs, addrs

def resolve_loops(loops: dict, instrs: list[str], addrs: list[int]):
	for i in range(len(instrs)):
		parts = re.split(DELIMTER_SPLIT, instrs[i])
		for j in range(len(parts)):
			if parts[j] in loops:
				parts[j] = f"pc, #-{addrs[i] - loops[parts[j]]}"
		instrs[i] = "".join(parts)
		print(instrs[i])
	
		
resolve_macros(lines)
loops, instrs, addrs = gather_loops_and_instrs(lines)
resolve_loops(loops, instrs, addrs)
print()

def assemble_instruction(instr: str, addr: int) -> int:
	parts = [p.strip() for p in instr.replace(",", " ").split()]
	if not instr:
		print(f"Encountered empty instruction during assembly stage at {addr}")

	opcode = OPCODES.get(parts[0], f"Invalid opcode in assembly stage at {addr}: \"{parts[0]}\"")
	dest = REGS.get(parts[1], f"Invalid register at {addr}: \"{parts[1]}\"")
	src1 = REGS.get(parts[2], f"Invalid register at {addr}: \"{parts[2]}\"")
	imm_flag = 0
	
	# If it's mov or a second source operand isn't provided, set it to 0
	if not opcode == OPCODES["mov"] and len(parts) > 3:
		if parts[3][0] == "#":
			candidiate = parts[3][1:]
			src2 = int(candidiate) if is_number(candidiate) else f"Invalid immediate at {addr}: \"{candidiate}\""
			imm_flag = 1
		else:
			src2 = REGS.get(parts[3], f"Invalid register at {addr}: \"{parts[3]}\"")
	else:
		src2 = 0

	error = False
	for operand in (opcode, dest, src1, src2):
		if isinstance(operand, str):
			print(operand)
			error = True
	if error:
		return
	
	return (opcode << 25) | (imm_flag << 24) | (dest << 20) | (src1 << 16) | (src2 & 0xFFFF)
		

def is_number(candidate: str) -> bool:
	try:
		int(candidate)
		return True
	except:
		return False

def assemble_all_instructions(instrs: list[str], addrs: list[int]) -> list[int]:
	result = []
	for instr, addr in zip(instrs, addrs):
		result.append(assemble_instruction(instr, addr))

assemble_all_instructions(instrs, addrs)


	





	

	

				