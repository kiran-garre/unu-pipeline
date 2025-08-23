#!/usr/bin/env python3

import re
import struct
import argparse

# =====================
#		CONSTANTS
# ===================== 

RED_ERROR = "\033[1m\033[91merror:\033[0m"
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


# =============================
#		ARGUMENT PARSING
# =============================

parser = argparse.ArgumentParser()
parser.add_argument("input", help="Input source filename")
parser.add_argument("-o", help="Output binary filename", default="a.u", metavar="<file>")

args = parser.parse_args()

filename = args.input
output_filename = args.o

try:
	with open(filename, 'r') as file:
		lines = file.readlines()
except:
	print(f"{RED_ERROR} file \"{filename}\" not found")
	exit(1)

# ===========================
#		SYNTAX PARSING
# ===========================

def resolve_macros(lines: list[str]):
	"""
	Performs simple macro substitution in a single pass.
	Note: This function modifies the `lines` argument 
	"""
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
	"""
	Gathers all loops and instructions with valid opcodes

	Returns:
		A tuple where the first element is a mapping from loop labels to 
		relative addresses, the second element is a list of lines starting with
		a valid opcode, and the third element is the line numbers corresponding 
		to the gathered instructions.
	"""
	loops = {}
	instrs = []
	line_numbers = []
	pc = 0
	
	# Pass 1: collect labels
	for (i, line) in enumerate(lines):
		if m := re.match(r'\s*@(\w+)', line):
			loops[m.group(1)] = pc
			continue
		
		tokens = line.split()
		if tokens and tokens[0] in OPCODES:
			instrs.append(line)
			line_numbers.append(i + 1)
			pc += 4
	
	return loops, instrs, line_numbers

def resolve_loops(loops: dict, instrs: list[str]):
	"""
	Replaces loop labels with their relative PC offsets in gathered instructions
	Treats loops as a basic text replacement; the label becomes PC - offset,
	where offset = the address of the instruction - the address of the label.
	"""

	for i in range(len(instrs)):
		parts = re.split(DELIMTER_SPLIT, instrs[i])
		for j in range(len(parts)):
			if parts[j] in loops:
				parts[j] = f"pc, #{-(i * 4 - loops[parts[j]])}"
		instrs[i] = "".join(parts)
	

# ================================
#		INSTRUCTION ASSEMBLY
# ================================

class AssemblyError(Exception):
    """Custom exception for assembler errors."""
    pass

def get_reg(token: str):
	"""
	Converts a token to a register, or raises an exception if not possible
	"""
	if not token in REGS:
		raise AssemblyError(f"invalid register: \"{token}\"")
	return REGS[token]
	
def get_imm(token: str):
	"""
	Converts a token to an immediate, or raises an exception if not possible
	"""
	if not token.startswith("#"):
		raise AssemblyError(f"expected immediate: \"{token}\"")
	value = token[1:]
	if not is_number(value):
		raise AssemblyError(f"invalid immediate: \"{value}\"")
	return int(value)

def get_opcode(token: str):
	"""
	Converts a token to an opcode, or raises an exception if not possible
	"""
	if token not in OPCODES:
		raise AssemblyError(f"invalid opcode in assembly stage: \"{token}\"")
	return OPCODES[token]

def assemble_instruction(instr: str) -> int:
	"""
	Given a candidate instruction string, parses it to a little-endian, 32-bit
	integer representing the binary form of the instruction.
	"""
	parts = instr.replace(",", " ").split()
	if len(instr) < 3:
		raise AssemblyError(f"not enough arguments")
	
	# Set default values
	dest, src1, src2 = 0, 0, 0

	opcode = get_opcode(parts[0])
	dest = get_reg(parts[1])

	# Handle mov as a special case
	if opcode == OPCODES["mov"] or opcode == OPCODES["brn"]:
		if len(parts) != 3:
			raise AssemblyError(f"invalid number of arguments for {parts[0]}")
		elif parts[2][0] == "#":
			src2 = get_imm(parts[2])
			imm_flag = 1
		else:
			src2 = get_reg(parts[2])
			imm_flag = 0

	else:
		if len(parts) not in (3, 4):
			# Only print error if our opcode is valid
			raise AssemblyError(f"invalid number of arguments for {parts[0]}")
		elif len(parts) == 4:
			src1 = get_reg(parts[2])
			if parts[3][0] == "#":
				src2 = get_imm(parts[3])
				imm_flag = 1
			else:
				src2 = get_reg(parts[3])
				imm_flag = 0
		else:
			# If there's no third operand, it counts as 0, which is an 
			# immediate
			imm_flag = 1
	
	result = (opcode & 0x7F) << 25
	result |= (imm_flag & 0x1) << 24
	result |= (dest & 0xF) << 20
	result |= (src1 & 0xF) << 16
	result |= (src2 & 0xFFFF)
	return struct.pack(">I", result)
		

def is_number(candidate: str) -> bool:
	"""
	Hacky-ish way for checking if a string is either a positive or negative
	integer
	"""
	try:
		int(candidate)
		return True
	except:
		return False

def assemble_all_instructions(instrs: list[str], line_numbers: list[int]) -> list[bytes]:
	"""
	Assembles a list of candidate instruction strings if possible and returns a 
	list of their binary forms. On errors, error messages are printed and
	None is returned.
	"""
	result = []
	errors = []
	for instr, line_number in zip(instrs, line_numbers):
		try:
			result.append(assemble_instruction(instr))
		except AssemblyError as e:
			errors.append(f"{RED_ERROR} {filename}:{line_number}: {str(e)}")
			result.append(None)
	
	if errors:
		for e in errors:
			print(e)
		return None
	return result


# ================
#		MAIN
# ================

resolve_macros(lines)
loops, instrs, line_numbers = gather_loops_and_instrs(lines)
resolve_loops(loops, instrs)

result = assemble_all_instructions(instrs, line_numbers)
if not result:
	exit(1)

with open(output_filename, "wb+") as output:
	output.write(b"".join(result))
	