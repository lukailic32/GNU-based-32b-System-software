#ifndef _REGEX_HPP__
#define _REGEX_HPP__

#include <string>
#include <regex>

using namespace std;

/*regexes for input and output files*/
regex regex_input_file_extension("^.*.s$");
regex regex_ouput_file_extension("^.*.o$");

// one or more occurence of white_character
regex regex_empty_line("^\\s*$");
regex regex_additionalSpace(" {2,}|\\t");
regex regex_punctuationSpace(" ?, ?");

regex regex_for_label("^([a-zA-Z][a-zA-Z0-9_]*):$");
regex regex_for_global("^\\.global ([a-zA-Z][a-zA-Z0-9_]*(, [a-zA-Z][a-zA-Z0-9_]*)*)$");
regex regex_for_extern("^\\.extern [a-zA-Z][a-zA-Z0-9_]*(, [a-zA-Z][a-zA-Z0-9_]*)*$");
regex regex_for_skip_hex_literal("^\\.skip (0x[0-9A-Fa-f]+)$");
regex regex_for_skip_dec_literal("^\\.skip ([0-9]+)$");
//regex regex_for_skip_dec_negative_literal("^\\.skip -[0-9]+$");
regex regex_for_section("^\\.section ([a-zA-Z][a-zA-Z0-9_]*)$");
regex regex_for_end("^(\\.end)|(\\.END)$");

string regex_word_1 = "0x[0-9A-Fa-f]+";
string regex_word_2 = "[0-9]+";
string regex_word_3 = "-[0-9]+";
string regex_word_4 = "[a-zA-Z][a-zA-Z0-9_]*";
string regex_word = regex_word_1 + "|" + regex_word_2 + "|" + regex_word_3 + "|" + regex_word_4;

regex regex_for_word("^\\.word ((" + regex_word + ")(, (" + regex_word + "))*)$");
regex regex_for_word_single("^[a-zA-Z][a-zA-Z0-9_]*|[0-9]+|-[0-9]+|0x[0-9A-Fa-f]+$");


regex regex_for_instruction_with_no_operands("^(halt|int|ret|iret)$");
regex regex_for_instruction_with_one_operand("^(push|pop|not) %(r[0-9]|r1[0-5]|sp|pc)$");
regex regex_for_one_operand_jump_instruction("^(call|jmp) (" + regex_word + ")$");
regex regex_for_branch_instruction("^(beq|bne|bgt) %(r[0-9]|r1[0-5]|sp|pc), %(r[0-9]|r1[0-5]|sp|pc), (.*)$");
regex regex_for_ld_instruction("^ld ([^,]+), %(r[0-9]|r1[0-5]|sp|pc)$"); //proveri
regex regex_for_st_instruction("^st %(r[0-9]|r1[0-5]|sp|pc), (.*)$");
regex regex_for_csrrd_instruction("^csrrd %(status|handler|cause), %(r[0-9]|r1[0-5]|sp|pc)$");
regex regex_for_csrwr_instruction("^csrwr %(r[0-9]|r1[0-5]|sp|pc), %(status|handler|cause)$");
//proveri $ na kraju
regex regex_for_other_instructions_with_two_operands("(xchg|add|sub|mul|div|cmp|and|or|xor|shl|shr) %(r[0-9]|r1[0-5]|sp|pc), %(r[0-9]|r1[0-5]|sp|pc)$");
 

string single_word = "[a-zA-Z][a-zA-Z0-9_]*|[0-9]*|-[0-9]*|0x[0-9A-Fa-f]+";
string jmp_absolute =  "[a-zA-Z][a-zA-Z0-9_]*|[0-9]*|0x[0-9A-Fa-f]+";
string jmp_memdir = "[0-9]*|0x[0-9A-Fa-f]+|[a-zA-Z][a-zA-Z0-9_]*";

regex regex_for_absolute_addr_ldr_str("^\\$(" + single_word + ")$");
regex regex_for_memdir_addr_ldr_str("^([a-zA-Z][a-zA-Z0-9_]*|[0-9]*|0x[0-9A-Fa-f]+)$");
regex regex_for_regdir_addr_ldr_str("^%(r[0-9]|r1[0-5]|pc|sp)$");
regex regex_for_regind_addr_ldr_str("^\\[%(r[0-9]|r1[0-5]|pc|sp)\\]$");
regex regex_for_regind_with_disp_addr_ldr_str("^\\[%(r[0-9]|r1[0-5]|pc|sp) \\+ ([a-zA-Z][a-zA-Z0-9_]*|[0-9]*|-[0-9]*|0x[0-9A-Fa-f]+)\\]$");

regex regex_for_absolute_addr_jmp_call_branch("^([a-zA-Z][a-zA-Z0-9_]*|[0-9]*|0x[0-9A-Fa-f]+)$");



#endif