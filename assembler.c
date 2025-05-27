#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 500 // Max chars in line
#define LABLE_MAX 50	// Max Labels chars
#define LABLE_NUM 2000 // Max Lables
#define MAX_LINES 10000 // Max lines


typedef struct Lable {
	char LableName[LABLE_MAX];
	int position;
}Lable;

typedef struct { // creating an object of type Instruction
	char opcode_str[10];     //
	int opcode;              //
	int rd, rs, rt;          //
	int immediate;           //
	int has_label;           //
	int is_bigimm;           //
	int line_number;         //
} Instruction;

typedef struct {
	char OpCodeName[10];
	int code;
} OpcodeEntry;

OpcodeEntry opcode_table[] = {
	{"add", 0}, {"sub", 1}, {"mul", 2},
	{"and", 3}, {"or", 4}, {"xor", 5},
	{"sll", 6}, {"sra", 7}, {"srl", 8},
	{"beq", 9}, {"bne", 10}, {"blt", 11},
	{"bgt", 12}, {"ble", 13}, {"bge", 14},
	{"jal", 15}, {"lw", 16}, {"sw", 17},
	{"reti", 18}, {"in", 19}, {"out", 20},
	{"halt", 21}
};
int opcode_table_size = sizeof(opcode_table) / sizeof(OpcodeEntry);

typedef struct {
	char RegisterEntryName[10];
	int number;
} RegisterEntry;

RegisterEntry register_table[] = {
	{"$zero", 0}, { "$imm", 1 }, { "$v0", 2 },
	{ "$a0", 3 }, { "$a1", 4 }, { "$a2", 5 }, { "$a3", 6 },
	{ "$t0", 7 }, { "$t1", 8 }, { "$t2", 9 },
	{ "$s0", 10 }, { "$s1", 11 }, { "$s2", 12 },
	{ "$gp", 13 }, { "$sp", 14 }, { "$ra", 15 }
};
int register_table_size = sizeof(register_table) / sizeof(RegisterEntry);


int main(int argc, char* argv[])
{
	// setting variables for files
	FILE* inputFile = NULL;	//pointers to File
	FILE* outputFile = NULL;	//pointers to File
	Lable struct_lb[LABLE_NUM]; //label struct

	int last_line = 0, label_num = 0; // Lines & labels counting

	// First run - label Reading
	//file1 = fopen(argv[2], "r");
	inputFile = fopen("fib.txt", "r");
	if (inputFile == NULL) {
		exit(1);
	}
	label_num = Getlabels(inputFile, struct_lb);
	fclose(inputFile);
	// End of first run

	// second run - "Memory reading"
	Instruction instrucion_lb[MAX_LINES];
	inputFile = fopen("fib.txt", "r");
	outputFile = fopen("memin.txt", "w");
	if (inputFile == NULL)
		exit(1);
	if (outputFile == NULL)
		exit(1);
	last_line = second_run(inputFile, instrucion_lb, struct_lb, outputFile, label_num);
	fclose(inputFile);
	fclose(outputFile);
	// End of second run
}

int Getlabels(FILE* file, Lable lb[])		// returns the number of labels, and puts the labels in the array
{
	int CharCounter;
	int labelCounter = 0;
	int FoundCharsCounter = 0;
	int counter = 0;
	int isLabel = 0;
	char line[MAX_LINE];	// the current line from the file
	char t = NULL;
	char foundLabels[LABLE_MAX];
	char* tempLine = NULL;

	while (fgets(line, sizeof(line), file))	// while we don't reach the end of the file, find labels
	{
		isLabel = 0;
		if (line[0] == "\0" || line[0] == "\n" || line[0] == "#" || strstr(line, ".word") != NULL) //If line is blank, remark, .word --> continue
			continue;

		if (strstr(line, ":") != NULL) //If label found
		{
			if (strchr(line, "#") != NULL) //Checking if the line is a remark and not a label
				if ((strchr(line, ":")) > (strchr(line, "#")))	//line is a remark and not a label, continue
					continue;

			//we found a label
			CharCounter = 0;
			FoundCharsCounter = 0;
			do {
				t = line[CharCounter];	// Getting the current char
				if (t != ':' && t != '\t' && t != ' ') {	// not tab, not space, not :
					foundLabels[FoundCharsCounter] = t;	// adds the found character to foundLabels
					FoundCharsCounter++;
					CharCounter++;
				}

			} while (t != ':');	// puts the label in foundLabels
			foundLabels[FoundCharsCounter] = '\0';	// adds null at the end of the label


			do {
				CharCounter++; // Check if the line is label line only
			} while ((line[CharCounter] == ' ') || (line[CharCounter] == '\t'));	// runs on the line to see if there is more after the label 

			if ((line[CharCounter] == '\n') || (line[CharCounter] == '#'))	// if the code ends after the label then it is a label
				isLabel = 1;

			tempLine = foundLabels;
			strcpy(lb[labelCounter].LableName, tempLine); //inserting the found label into the labels array

			if (isLabel == 1) { // the line is only label
				lb[labelCounter].position = counter;
				counter--;
			}
			else {	// the line is label line and assembly prompt
				lb[labelCounter].position = counter;
			}
			labelCounter++;	// label added to array	
		}
		
		CharCounter = 0; // Check if the line is only spaces
		while (line[CharCounter] == ' ' || line[CharCounter] == '\t') {
			CharCounter++;
		}
		if (line[CharCounter] == '\n') {
			continue;
		}
		if (estimate_if_bigimm(line)) {
			counter += 2;
		}
		else {
			counter++;
		}
	}
	return(labelCounter); //Return number of labels
}

// Simple heuristic to guess bigimm: label or out-of-range imm
int estimate_if_bigimm(const char* line) {
	char temp_line[MAX_LINE];
	strcpy(temp_line, line);
	char* token = strtok(temp_line, ", ");
	int count = 0;
	while (token) {
		count++;
		if (count == 5) {  // 5th operand = immediate
			if (is_label_immediate(token)) {
				//printf("%s", token);
				return 1;
			}
			int imm = atoi(token);
			if (imm < -128 || imm > 127) {
				//printf("%s", token);
				return 1;
			}
			return 0;
			
		}
		token = strtok(NULL, ", ");
	}
	return 0;
}

int is_label_immediate(const char* token) {	// if there is a letter in the word == the word is label 
	if (!token) return 0;
	int i = 0;
	while (token[i]) {
		if (isalpha(token[i])) {
			return 1;
		}
		i++;
	}
	return 0;
}

int second_run(FILE* inputFile, Instruction lines[], Lable struct_lb[], FILE* outputFile, int label_num) {
	char* line[MAX_LINE];
	int to_hexa;
	int linesCounter = 0;
	int word_line_number = 0;
	int word_line_input = 0;
	while (fgets(line, sizeof(line), inputFile)) {	// while we don't reach the end of the file, add line to lines[]
		if (line == "\0" || line == "\n" || line == "#" || strstr(line, ":") != NULL) { //If line is blank, remark,vis label
			continue;
		}
		if (strstr(line, ".word") != NULL) {
			int word1, word2;
			if (sscanf(line, " .word %d %d", &word1, &word2) == 2) {
				word_line_number = word1;
				word_line_input = word2;
			}
			continue;
		}
		linesCounter += line_to_hexa(line, struct_lb, outputFile, label_num);
	}
	
	if (linesCounter < word_line_number) { // add blanks until .word line
		while (linesCounter < word_line_number) {
			fprintf(outputFile, "00000000\n");
			linesCounter++;
		}
		if (word_line_number == linesCounter) {	//add .word line
			fprintf(outputFile, "%08X\n", word_line_input);
		}
	}
}

int lookup_word(const char* word) {
	if (word[0] == '$') {
		for (int i = 0; i < register_table_size; i++) {
			if (strcmp(register_table[i].RegisterEntryName, word) == 0) {
				return register_table[i].number;
			}
		}
		return -1;
	}
	else {
		for (int i = 0; i < opcode_table_size; i++) {
			if (strcmp(opcode_table[i].OpCodeName, word) == 0) {
				return opcode_table[i].code;
			}
		}
		return -1;
	}
}

char* remove_Spaces(const char* word) {

	int j = 0;
	for (int i = 0; i < LABLE_MAX && word[i] != '\0'; i++) {
		if (word[i] != ' ' && word[i] != '\t') {
			j++;
		}
	}
	char* result = malloc(j + 1);	// Allocate max possible size

	int k = 0;
	for (int i = 0; i < LABLE_MAX && word[i] != '\0'; i++) {
		if (word[i] != ' ' && word[i] != '\t') {
			result[k++] = word[i];
		}
	}
	result[k] = '\0';  // Null-terminate the string
	return result;
}

int lookup_label(const char* word, Lable struct_lb[], int label_num) {

	char* result = remove_Spaces(word);
	for (int i = 0; i < label_num; i++) {
		if (strcmp(struct_lb[i].LableName, result) == 0) {
			return struct_lb[i].position;
		}
	}
	return -999;
}

int line_to_hexa(char* line, Lable struct_lb[], FILE* outputFile, int label_num) {

	char* token;
	int word_index = 0;
	unsigned int encoded = 0;
	int values[5] = { 0 };
	int immediate = 0;
	int is_label = 0;
	int bigimm = 0;
	int counter = 0;

	// Remove comment from line
	char* comment = strchr(line, '#');
	if (comment) *comment = '\0';

	// Skip leading spaces
	while (*line == ' ' || *line == '\t') line++;

	// Tokenize and parse
	token = strtok(line, " ,\n");
	// check if line is empty
	if (token == NULL) {	
		return 0;
	}

	// Go over the line and write to the file in Hexa
	while (token != NULL && counter < 5) {
		if (word_index < 4) {
			values[word_index] = lookup_word(token);
		}
		else	// Handle label or immediate
		{
			int label_address = lookup_label(token, struct_lb, label_num);
			if (label_address != -999) {	// if label
				immediate = label_address;
				is_label = 1;
			}
			else {	// if number
				token = remove_Spaces(token);
				immediate = atoi(token);
				is_label = 0;
			}
		}
		word_index++;
		token = strtok(NULL, " ,\n");
		counter++;
	}

	// Determine if bigimm is needed
	if (is_label || immediate < -128 || immediate > 127)
		bigimm = 1;

	// Encode the instruction
	encoded |= (values[0] & 0xFF) << 24;       // opcode
	encoded |= (values[1] & 0xF) << 20;        // rd
	encoded |= (values[2] & 0xF) << 16;        // rs
	encoded |= (values[3] & 0xF) << 12;        // rt
	encoded |= 0 << 9;                         // reserved
	encoded |= bigimm << 8;                    // bigimm flag
	encoded |= (bigimm ? 0 : (immediate & 0xFF));  // imm8 (if not bigimm)

	fprintf(outputFile, "%08X\n", encoded);

	if (bigimm) {
		fprintf(outputFile, "%08X\n", immediate & 0xFFFFFFFF);  // emit second word
		return 2;  // tells second_run this line took 2 slots
	}
	return 1;
}