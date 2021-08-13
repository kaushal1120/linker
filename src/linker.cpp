#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <iomanip>
#include <sstream>
#include <tokenizer.h>
using namespace std;

#define MEM_SIZE 512
#define MAX_LIST 16
#define NUM_EXPECTED 0
#define SYM_EXPECTED 1
#define ADDR_EXPECTED 2
#define SYM_TOO_LONG 3
#define TOO_MANY_DEF_IN_MODULE 4
#define TOO_MANY_USE_IN_MODULE 5
#define TOO_MANY_INSTR 6

struct symbol {
    int addr;
    long unsigned int module_no;
    bool redefined;
    bool used;
};

class Linker {
    private:
        map<string, symbol> symboltable;
        //To maintain order of insertion, we use a vector of symbols as well.
        vector<string> symbols;
        vector<int> module_baseaddrs;
        Tokenizer* tokenizer;
        bool tokenMissing;

        /**
        Read an integer token.
        **/
        int readInt(bool newModFlag){
            char* token = NULL;
            while(token == NULL && !tokenizer->isEOF())
                token = tokenizer->getToken();

            //EOF reached.
            if(token == NULL){
                if(newModFlag) //Expecting new module defcount. token = NULL -> No new module.
                    return -1;
                else{
                    tokenMissing=true;
                    throw (NUM_EXPECTED);
                }
            }

            char* token_temp = token;
            while(*token_temp != '\0'){
                if(!isdigit(*token_temp))
                    throw (NUM_EXPECTED);
                token_temp++;
            }

            int no = atoi(token);

            //If no >= 2^30 or exceeds int limit.
            if(no >= 1073741824 || no < 0)
                throw (NUM_EXPECTED);
            return no;
        }

        /**
        Read a symbol(string) token.
        **/
        string readSymbol(){
            char* token = NULL;
            while(token == NULL && !tokenizer->isEOF())
                token = tokenizer->getToken();

            //EOF reached.
            if(token == NULL){
                tokenMissing=true;
                throw (SYM_EXPECTED);
            }

            string tokenstring(token);

            if(!isalpha(tokenstring[0]))
                throw (SYM_EXPECTED);
            for(int i = 1; i < tokenstring.size(); i++)
                if(!isalnum(tokenstring[i]))
                    throw (SYM_EXPECTED);

            if(tokenstring.size()>MAX_LIST)
                throw (SYM_TOO_LONG);

            return tokenstring;
        }

        /**
        Read an instruction(character) token.
        **/
        char readIAER(){
            char* token = NULL;
            while(token == NULL && !tokenizer->isEOF())
                token = tokenizer->getToken();

            //EOF reached.
            if(token == NULL){
                tokenMissing=true;
                throw (ADDR_EXPECTED);
            }

            if(*token != 'A' && *token != 'E' && *token != 'I' && *token != 'R')
                throw (ADDR_EXPECTED);

            return *token;
        }

        /**
        Prints the symbol table.
        **/
        void printSymbolTable(){
            cout<<"Symbol Table"<<endl;
            for(string key : symbols){
                cout << key << "=" << symboltable[key].addr;
                if(symboltable[key].redefined) //If a symbol is defined multiple times
                    cout<<" Error: This variable is multiple times defined; first value used";
                cout<<endl;
            }
            cout<<endl;
        }

    public:
        Linker(Tokenizer* tokenizer_arg){
            tokenMissing = false;
            tokenizer = tokenizer_arg;
            module_baseaddrs.push_back(0);
        }

        /**
        Maps parse error codes with their respective error strings.
        **/
        void parseError(int errcode) {
            static string errstr[] = {
                "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
                "SYM_EXPECTED", // Symbol Expected
                "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R
                "SYM_TOO_LONG", // Symbol Name is too long
                "TOO_MANY_DEF_IN_MODULE", // > 16
                "TOO_MANY_USE_IN_MODULE", // > 16
                "TOO_MANY_INSTR" // total num_instr exceeds memory size (512)
            };
            cout << "Parse Error line " << tokenizer->getLineNo() << " offset " << (tokenMissing ? tokenizer->getOffset() : tokenizer->getPrevOffset()) << ": " << errstr[errcode] << endl;
        }

        /**
        1st pass of the 2-pass linker.
        **/
        void pass1(){
            int defcount = 0;
            int usecount = 0;
            int codecount = 0;
            string key = "";

            while(!tokenizer->isEOF()){
                defcount = readInt(1);
                if(defcount == -1)
                    break;
                if(defcount > MAX_LIST)
                    throw (TOO_MANY_DEF_IN_MODULE);
                //Read and store symbols in symbol table. If redefined, set redefined for symbol to true.
                for(int i = 0; i < defcount; i++){
                    key = readSymbol();
                    if(symboltable.find(key) == symboltable.end()){
                        symbols.push_back(key);
                        symbol newSymbol = {module_baseaddrs[module_baseaddrs.size()-1] + readInt(0), module_baseaddrs.size(), false, false};
                        symboltable[key] = newSymbol;
                    }
                    else{
                        symboltable[key].redefined = true;
                        readInt(0);
                    }
                }

                //Read uselist.
                usecount = readInt(0);
                if(usecount > MAX_LIST)
                    throw (TOO_MANY_USE_IN_MODULE);
                for(int i = 0; i < usecount; i++){
                    readSymbol();
                }

                //Read instruction list.
                codecount = readInt(0);
                if(module_baseaddrs[module_baseaddrs.size()-1] + codecount > MEM_SIZE)
                    throw (TOO_MANY_INSTR);
                for(int i = 0; i < codecount; i++){
                    readIAER();
                    readInt(0);
                }

                //Warning for all symbols in current module that are bigger than the module size.
                for(int i = symbols.size() - defcount; i < symbols.size(); i++){
                    if(symboltable[symbols[i]].addr >= module_baseaddrs[module_baseaddrs.size()-1] + codecount){
                        cout << "Warning: Module " << module_baseaddrs.size() << ": " << symbols[i] << " too big " << symboltable[symbols[i]].addr - module_baseaddrs[module_baseaddrs.size()-1] << " (max=" << codecount-1 << ") assume zero relative" << endl;
                        symboltable[symbols[i]].addr = module_baseaddrs[module_baseaddrs.size()-1];
                    }
                }

                module_baseaddrs.push_back(module_baseaddrs[module_baseaddrs.size()-1] + codecount);
            }

            printSymbolTable();
        }

        /**
        2nd pass of the 2-pass linker.
        **/
        void pass2(){
            cout<<"Memory Map"<<endl;
            int module_no = 0;
            int defcount = 0;
            int usecount = 0;
            int codecount = 0;
            vector<string> uselist;
            vector<bool> useflags;
            stringstream errorMsg;

            while(!tokenizer->isEOF()){
                defcount = readInt(1);
                if(defcount == -1)
                    break;
                for(int i = 0; i < defcount; i++){
                    readSymbol(); readInt(0);
                }

                usecount = readInt(0);
                for(int i = 0; i < usecount; i++){
                    uselist.push_back(readSymbol());
                    useflags.push_back(false);
                }

                codecount = readInt(0);
                for(int i = 0; i < codecount; i++){
                    char type = readIAER();
                    int instr = readInt(0);

                    if(type == 'I'){
                        if(instr >= 10000){ //If an illegal immediate value (I) is encountered (i.e. >= 10000)
                            instr = 9999;
                            errorMsg << " Error: Illegal immediate value; treated as 9999";
                        }
                    }
                    else{
                        if(instr/1000 >= 10){ //If an illegal opcode is encountered (i.e. op >= 10)
                            errorMsg << " Error: Illegal opcode; treated as 9999";
                            instr = 9999;
                        }
                        else{
                            if(type == 'R'){
                                if(instr%1000 >= codecount){ //If a relative address exceeds the size of the module
                                    instr -= instr%1000;
                                    errorMsg << " Error: Relative address exceeds module size; zero used";
                                }
                                instr += module_baseaddrs[module_no];
                            }
                            else if(type == 'E'){
                                if(instr%1000 >= usecount){ //If an external address is too large to reference an entry in the use list
                                    errorMsg << " Error: External address exceeds length of uselist; treated as immediate";
                                }
                                else if(symboltable.find(uselist[instr%1000]) == symboltable.end()){ //If a symbol is used in an E-instruction but not defined anywhere
                                    errorMsg << " Error: " << uselist[instr%1000] << " is not defined; zero used";
                                    useflags[instr%1000] = true;
                                    instr -= instr%1000;
                                }
                                else{
                                    useflags[instr%1000] = true;
                                    symboltable[uselist[instr%1000]].used = true;
                                    instr = (instr/1000)*1000 + symboltable[uselist[instr%1000]].addr;
                                }
                            }
                            else{ //Instr type 'A'
                                if(instr%1000 >= 512){ //If an absolute address exceeds the size of the machine
                                    instr -= instr%1000;
                                    errorMsg << " Error: Absolute address exceeds machine size; zero used";
                                }
                            }
                        }
                    }

                    cout << setw(3) << setfill('0') << module_baseaddrs[module_no] + i << ": " << setw(4) << setfill('0') << instr << errorMsg.str() << endl;
                    errorMsg.str(std::string());
                }

                //If a symbol appears in a use list but is not actually used in the module, print a warning message and continue.
                for(int i = 0; i < uselist.size(); i++){
                    if(!useflags[i])
                        cout<<"Warning: Module " << module_no + 1 << ": " << uselist[i] << " appeared in the uselist but was not actually used"<<endl;
                }

                useflags.clear();
                uselist.clear();
                module_no++;
            }

            cout<<endl;
            //If a symbol is defined but not used, print a warning message and continue.
            for(string key : symbols){
                if(!symboltable[key].used)
                    cout<<"Warning: Module " << symboltable[key].module_no << ": " << key << " was defined but never used" << endl;
            }
            cout<<endl;
        }

        void setTokenizer(Tokenizer* tokenizer_arg){
            tokenizer = tokenizer_arg;
        }
};

int main(int argc, char *argv[]){
    if(argc < 2){
        cout<<"Expected argument after options"<<endl;
        return 0;
    }

    Tokenizer* tokenizer = new Tokenizer(argv[1]);
    Linker linker(tokenizer);

    try {
        linker.pass1();
        delete tokenizer;

        tokenizer = new Tokenizer(argv[1]);
        linker.setTokenizer(tokenizer);
        linker.pass2();
    } catch(int errcode){
        linker.parseError(errcode);
        return 0;
    }
    delete tokenizer;
    return 1;
}