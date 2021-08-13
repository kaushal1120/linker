#include <fstream>
#include <string.h>
using namespace std;

/**
 Tokenizer to separate file contents into tokens.
**/
class Tokenizer {
    private:
        ifstream f_stream;
        int line_no;
        int offset;
        int prevOffset;
        char* s_stream;
        string objmod_line;

    public:
        /**
        Open file. Initialize cursor with line number and offset for first token.
        **/
        Tokenizer(char* filename);

        /**
        Get next token.
        **/
        char* getToken();

        /**
        Checks if end of file (EOF) has been reached.
        **/
        bool isEOF();

        /**
        Returns the current line number of the cursor.
        **/
        int getLineNo();

        /**
        Returns the current offset.
        **/
        int getOffset();

        /**
        Returns the preceding offset instead of the current offset.
        Used when there are no more tokens following the current offset.
        **/
        int getPrevOffset();

        /**
        Close the file stream
        **/
        virtual ~Tokenizer();
};