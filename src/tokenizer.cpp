#include <iostream>
#include <fstream>
#include <string.h>
#include "tokenizer.h"
using namespace std;

/**
Open file. Initialize cursor with line number and offset for first token.
**/
Tokenizer::Tokenizer(char* filename){
    f_stream.open(filename);

    if(!f_stream){
        cout<<"Not a valid inputfile <" << filename << ">" <<endl;
        exit(0);
    }

    line_no=1;
    prevOffset=1;
    getline(f_stream, objmod_line);
    s_stream = strtok(const_cast<char*>(objmod_line.c_str()), " \t");
    if(s_stream != NULL)
        offset = s_stream - &objmod_line[0] + 1;
    else
        offset = objmod_line.length() + 1;
}

/**
Get next token.
**/
char* Tokenizer::getToken(){
    char* token = NULL;

    if(s_stream != NULL){
        token = s_stream;
        //cout << "Token: " << line_no << ":" << offset << " : " << s_stream << endl;
        s_stream = strtok(NULL, " \t");
    }
    else{
        if(getline(f_stream, objmod_line)){
            line_no++;
            s_stream = strtok(const_cast<char*>(objmod_line.c_str()), " \t");
        }
        else{
            //cout << "Final Spot in File : line=" << line_no << " offset=" << offset << endl;
            return 0;
        }
    }
    prevOffset=offset;
    if(s_stream != NULL)
        offset = s_stream - &objmod_line[0] + 1;
    else
        offset = objmod_line.length() + 1;
    return token;
}

/**
Checks if end of file (EOF) has been reached.
**/
bool Tokenizer::isEOF(){
    return f_stream.peek() == EOF && s_stream == NULL;
}

/**
Returns the current line number of the cursor.
**/
int Tokenizer::getLineNo(){
    return line_no;
}

/**
Returns the current offset.
**/
int Tokenizer::getOffset(){
    return offset;
}

/**
Returns the preceding offset instead of the current offset.
Used when there are no more tokens following the current offset.
**/
int Tokenizer::getPrevOffset(){
    return prevOffset;
}

/**
Close the file stream
**/
Tokenizer::~Tokenizer(){
    f_stream.close();
}

/*int main(int argc, char *argv[]){
    Tokenizer tokenizer(argv[1]);
    while(!tokenizer.isEOF()){tokenizer.getToken();}
    tokenizer.getToken();
    return 1;
}*/