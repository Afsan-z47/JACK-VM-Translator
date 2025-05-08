#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdlib.h>



//Function prototypes
int code_writer(char* command, char* segment, char* tmp_index, FILE* out, int line_count, char* static_memory);
int write_arithmetic(char* command, FILE* out, int line_count);
int write_push_pop(char* command, char* segment, char* tmp_index, FILE* out, char* static_memory);
int translate(char* comp_in, char* comp_out, int index, char* static_memory);
int write_goto(char* command, char* segment, FILE* out);
void write_label(char* command, char* segment, FILE* out);
int write_if_goto(char* command, char* segment, FILE* out);
int write_function(char* command, char* segment, char* tmp_index, FILE* out);
int write_call(char* command, char* segment, char* tmp_index, FILE* out, int line_count);
int write_return(char* comman, FILE* out);
int parse(char* file_location, FILE* out);
void set_static_memory(char* file_location, char* static_memory);
int line_count = 0;


//Function definations


int writeInit(FILE* out){

	fprintf(out , "@256\nD=A\n@SP\nM=D\n");
	int line_number = 4;
	fprintf(out , "//calling Sys.init\n");
	line_number += write_call("call", "Sys.init", "0", out, line_count);
	return line_number;
}

int write_arithmetic(char* command, FILE* out, int line_count){
	int line_number = 0;
	int udontno1 = line_count + 12;
	int udontno2 = line_count + 15;
	if( !strcmp(command, "add")){
		fprintf(out, "@SP\nA=M-1\nD=M\nA=A-1\nM=D+M\n@SP\nM=M-1\n");
		line_number = 7;
	}
	//Possible subtraction error D-M||M-D
	if( !strcmp(command, "sub")){
		fprintf(out, "@SP\nA=M-1\nD=M\nA=A-1\nM=M-D\n@SP\nM=M-1\n");
		line_number = 7;
	}
	if( !strcmp(command, "neg")){
		fprintf(out, "@SP\nA=M-1\nM=-M\n");
		line_number = 3;
	}
	if( !strcmp(command, "eq")){
		fprintf(out, "@SP\nAM=M-1\nD=M\nA=A-1\nMD=M-D\n@%d\nD;JNE\n@SP\nA=M-1\nM=-1\n@%d\n0;JMP\n@SP\nA=M-1\nM=0\n", udontno1, udontno2);
		line_number = 15;
	}
	if( !strcmp(command, "gt")){
		fprintf(out, "@SP\nAM=M-1\nD=M\nA=A-1\nMD=M-D\n@%d\nD;JGT\n@SP\nA=M-1\nM=0\n@%d\n0;JMP\n@SP\nA=M-1\nM=-1\n", udontno1, udontno2);
		line_number = 15;
	}
	if( !strcmp(command, "lt")){
		fprintf(out, "@SP\nAM=M-1\nD=M\nA=A-1\nMD=M-D\n@%d\nD;JGE\n@SP\nA=M-1\nM=-1\n@%d\n0;JMP\n@SP\nA=M-1\nM=0\n", udontno1, udontno2);
		line_number = 15;
	}	
	if( !strcmp(command, "and")){
		fprintf(out, "@SP\nA=M-1\nD=M\nA=A-1\nM=D&M\n@SP\nM=M-1\n");
		line_number = 7;
	}
	if( !strcmp(command, "or")){
		fprintf(out, "@SP\nA=M-1\nD=M\nA=A-1\nM=D|M\n@SP\nM=M-1\n");
		line_number = 7;
	}
	if( !strcmp(command, "not")){
		fprintf(out, "@SP\nA=M-1\nM=!M\n");
		line_number = 3;
	}
	return line_number;
}

int write_function(char* command, char* segment, char* tmp_index, FILE* out){

	int line_number = 0;
	int nVars = 0;
	nVars = atoi(tmp_index);

	if(!strcmp(command, "function")){

		write_label("label", segment, out);
		if( nVars != 0){
			fprintf( out, "@SP\nM=M+1\nD=A\nA=M-1\n");
			line_number += 4;
		}
		for( int i =0; i<nVars; i++){
			fprintf(out, "M=D\nA=A+1\n");
			line_number += 2;
		}
	}
	return line_number;
}
int return_location(char* tmp_index, int line_count){
	int index = 42;
	int nArgs = 0;
	nArgs = atoi(tmp_index);
	for(int i =0;i<nArgs;i++){
		index +=1;
	}
	index +=  line_count;
	return index;
}

int write_call(char* command, char* segment, char* tmp_index, FILE* out, int line_count){
	int line_number = 0;
	int nArgs = 0;
	nArgs = atoi(tmp_index);
	if(!strcmp(command, "call")){
		int location = return_location(tmp_index, line_count);
		fprintf(out, "//location: %d\n", location);
		fprintf(out, "@%d\nD=A\n@SP\nM=M+1\nA=M-1\nM=D //saving RETURN\n", location); //save return
		fprintf(out, "@LCL\nD=M\n@SP\nM=M+1\nA=M-1\nM=D //saving LCL\n"); // save previous lcl
		fprintf(out, "@ARG\nD=M\n@SP\nM=M+1\nA=M-1\nM=D //saving ARG\n"); //save previous arg
		fprintf(out, "@THIS\nD=M\n@SP\nM=M+1\nA=M-1\nM=D //saving THIS\n"); //save previous this
		fprintf(out, "@THAT\nD=M\n@SP\nM=M+1\nA=M-1\nM=D //saving THAT\n"); //save previous that
		line_number += 30;
		fprintf( out, "//setting function ARG\n");
		fprintf( out, "@5\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
		line_number += 6;

		for(int i =0; i<nArgs; i++){
			fprintf(out, "M=M-1\n");
			line_number +=1;
		}
		fprintf( out, "//setting function LCL\n");
		fprintf( out, "@SP\nD=M\n@LCL\nM=D\n");
		line_number += 4;
		line_number += write_goto("goto", segment, out);
	}
	return line_number;
}

int write_return(char* command, FILE* out){
	int line_number = 0;
	if(!strcmp(command, "return")){
		//Set to previous return
		fprintf( out, "@5\nD=A\n@LCL\nA=M-D\nD=M\n@returnAddress\nM=D //set pre-return\n");
		line_number +=7;
		//Set return-value
		fprintf(out, "@SP //set return-value\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n");
		line_number += 6;
		//RESET SP
		fprintf(out , "//RESET SP\n");
		fprintf(out, "@ARG\nD=M\n@SP\nM=D+1\n");
		line_number +=4;
		//Set to previous ARG
		fprintf(out, "@LCL\nA=M-1\nD=M\n@THAT\nM=D //set pre-THAT\n");
		line_number += 5;
		//Set to previous THIS
		fprintf( out, "@2\nD=A\n@LCL\nA=M-D\nD=M\n@THIS\nM=D //set pre-THIS\n");
		line_number +=7;
		//Set to previous THAT
		fprintf( out, "@3\nD=A\n@LCL\nA=M-D\nD=M\n@ARG\nM=D //set pre-ARG\n");
		line_number +=7;
		//Set to previous LCL
		fprintf( out, "@4\nD=A\n@LCL\nA=M-D\nD=M\n@LCL\nM=D //set pre-LCL\n");
		line_number +=7;
		//return to previous function
		fprintf( out, "@returnAddress\nA=M\n0;JMP // return to pre-function\n");
		line_number +=3;
	}
	return line_number;

}

int write_goto(char* command, char* segment, FILE* out){
	int line_number = 0;

	if(!strcmp(command, "goto")){
		fprintf( out,"@%s\n0;JMP\n", segment );
		line_number = 2;
	}

	return line_number;
}

void write_label(char* command, char* segment, FILE* out){

	if(!strcmp(command, "label")){
		fprintf( out,"(%s)\n", segment);
	}

}

int write_if_goto(char* command, char* segment, FILE* out){
	int line_number = 0;

	if(!strcmp(command, "if-goto")){
		fprintf( out, "@SP\nAM=M-1\nD=M\n@%s\nD;JNE\n", segment);
		line_number = 5;
	}
	return line_number;
}


int write_push_pop(char* command, char* segment, char* tmp_index, FILE* out, char* static_memory){

	char tmp_segment[10];
	int line_number = 0;
	int is_translate = 0;
	int index = atoi(tmp_index);

	if( !strcmp(command, "push")){
		is_translate = translate(segment, tmp_segment, index, static_memory);
		if(is_translate == 1){
			fprintf(out, "@%s\nD=M\n@%d\nA=D+A\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", tmp_segment, index);
			line_number = 10;
		}
		//push temp
		if(is_translate == 2){
			fprintf(out, "@%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", tmp_segment);
			line_number = 7;
		}
		//push static
		if (is_translate == 3) {
			fprintf(out, "@%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", tmp_segment);
			line_number = 7;
		}
		//push pointer
		if(is_translate == 4){
			fprintf(out, "@%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", tmp_segment);
			line_number = 7;
		}
		//push constant
		if(is_translate == 0)
		{
			fprintf(out, "@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", index);
			line_number = 7;
		}
	}

	if( !strcmp(command, "pop")){
		is_translate = translate(segment, tmp_segment, index, static_memory);
		//	fprintf(out, "tmp: %s\nis_translate: $%d", tmp_segment,is_translate);
		if(is_translate == 1){
			fprintf(out, "@%s\nD=M\n@%d\nD=D+A\n@%s\nM=D\n@SP\nAM=M-1\nD=M\n@%s\nA=M\nM=D\n@%d\nD=A\n@%s\nM=M-D\n", tmp_segment,index, tmp_segment, tmp_segment, index, tmp_segment);
			line_number = 16;
		}
		//pop temp
		if(is_translate == 2){
			fprintf(out, "@SP\nAM=M-1\nD=M\n@%s\nM=D\n", tmp_segment);
			line_number = 5;
		}
		//pop static
		if(is_translate == 3){
			fprintf(out, "@SP\nAM=M-1\nD=M\n@%s\nM=D\n", tmp_segment);
			line_number = 5;
		}
		//pop pointer
		if(is_translate == 4){
			fprintf(out, "@SP\nAM=M-1\nD=M\n@%s\nM=D\n", tmp_segment);
			line_number = 5;
		}//pop constant
		if(is_translate == 0)
		{
			fprintf(out, "@SP\nA=M\nD=M\n@%d\nM=D\n@SP\nM=M-1\n",index);
			line_number = 6;
		}

	}
	return line_number;
}

int translate(char* comp_in, char* comp_out, int index, char* static_memory){

	if ( !strcmp(comp_in, "local" )){
		sprintf( comp_out, "LCL");
		return 1;
	}
	if ( !strcmp(comp_in, "argument" )){
		sprintf( comp_out, "ARG");
		return 1;
	}
	if ( !strcmp(comp_in, "this" )){
		sprintf( comp_out, "THIS");
		return 1;
	}
	if ( !strcmp(comp_in, "that" )){
		sprintf( comp_out, "THAT");
		return 1;
	}
	//	if ( !strcmp(comp_in, "constant" )){
	//		sprintf( comp_out, "");
	//	}

	if ( !strcmp(comp_in, "static" )){
		sprintf( comp_out, "%s.%d", static_memory,index);
		return 3;
	}
	if ( !strcmp(comp_in, "temp" )){
		int tmp_index = 5;
		tmp_index += index;
		printf("tmp_index %d\n", tmp_index);

		sprintf( comp_out, "%d", tmp_index);
		return 2;
	}
	if ( !strcmp(comp_in, "pointer" )){
		if(index == 0){
			sprintf( comp_out, "THIS");
		}
		else {
			sprintf( comp_out, "THAT");
		}
		return 4;
	}

	return 0;
}



int code_writer(char* command, char* segment, char* tmp_index, FILE* out, int line_count, char* static_memory){


	int line_number = 0;
	if( strcmp(command, "\0")){
		fprintf(out,"//%s %s %s\n",command, segment, tmp_index);
	}
	//BOOTSTRAP code
	if( line_count == 0){
		line_number += writeInit(out);
	}
	//Memory segment commands
	line_number += write_push_pop(command, segment, tmp_index, out, static_memory);
	//Arithmetic logical commmands
	line_number += write_arithmetic(command, out, line_count);
	//GOTO command
	line_number += write_goto(command, segment, out);
	//LABEL command
	write_label(command, segment, out);
	//IF_GOTO command
	line_number += write_if_goto(command, segment, out);
	//FUNCTION command
	line_number += write_function(command, segment, tmp_index, out);
	//CALL command
	line_number += write_call(command, segment, tmp_index, out, line_count);
	//RETURN command
	line_number += write_return(command, out);

	//line count
	line_count = (line_count + line_number);
	fprintf(out, "//line_count: %d\n", line_count);	
	//CLEAR INPUT
	sprintf(command, "%s", "");
	sprintf(segment, "%s", "");
	sprintf(tmp_index, "%s", "");

	return line_count;

}
void set_static_memory(char* file_location, char* static_memory){
	
	//finding end of file_name
	printf("file_location: %s\n", file_location);

	int pointer = strlen(file_location);
	int character_number = 0;

	for(;; pointer--){
		if(file_location[pointer] == '/'){
			pointer++;
			break;
		}
		character_number++;
	}
	//Excluding (.vm) by Subtracting 3
	int i = 0;
	for(;i<(character_number-4);i++){
		static_memory[i] = file_location[pointer+i];
	}
	static_memory[i] = '\0';
}
int parse(char* file_location, FILE* out){

	char source[1024];
	char command[16] = "\0";
	char segment[32] = "\0";
	char tmp_index[16]="\0";
	//int line_count = 0;	
	//int index = 0;
	//Command Types
	int c_arithmetic;
	int c_push;
	int c_label;
	int c_goto;
	int c_if;
	int c_function;
	int c_return;
	int c_call;

	//Static_memory
	char static_memory[32];
	sprintf(static_memory, NULL);
	set_static_memory(file_location, static_memory);
	printf("static_memory: %s\n", static_memory);


	FILE* file = fopen( file_location, "r");
	if ( file == NULL){
		printf("File cannot be opened\n");
		return line_count;
	}

	for(int i =0;; i++){	

		//Command Types
		c_arithmetic = 0;
		c_push = 0;
		c_label = 0;
		c_goto = 0;
		c_if = 0;
		c_function=0;
		c_return = 0;
		c_call = 0;


		if(fgets( source, 1024, file) == NULL){ break;}
		else if (source[0] == '/') {
			source[0] = '\0';
			continue;
		}

		sscanf(source, "%s", command);
		sscanf(source, "%*s %s", segment);
		sscanf(source, "%*s %*s %s", tmp_index);	

		line_count = code_writer(command, segment, tmp_index, out, line_count, static_memory);

		//fprintf(out, "%s", command);
		//fprintf(out, "%s", segment);
		//fprintf(out, "%d", index);
		//fprintf(out, "\n");
		//while (fscanf(source, "%s ", word) == 1)
		//	fprintf(out, "%s\n", source);

		//	fprintf(out, "%s", source);


	}
	return line_count;
	fclose(file);
	
}



//Main function

int main(int argc, char** argv){


	FILE* out;
	out = fopen("out.asm", "w");
	printf("%d number of argc passed\n", argc);
	//Add all vm files
	for(int i=1;i<argc; i++){
		parse(argv[i], out);
	}
	fclose(out);

}
