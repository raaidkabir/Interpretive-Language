#include <iostream>
#include <map>
#include <ctype.h>
#include "lex.h"
using namespace std;

static map<Token,string> token_map{
        {PROGRAM, "PROGRAM"},
        {WRITE,"WRITE"},
        {INT,"INT"},
        {END,"END"},
        {IF,"IF"},
        {FLOAT,"FLOAT"},
        {STRING,"STRING"},

        {REPEAT,"REPEAT"},
        {BEGIN,"BEGIN"},
        {IDENT,"IDENT"},

        {ICONST,"ICONST"},
        {RCONST,"RCONST"},
        {SCONST,"SCONST"},
        {PLUS,"PLUS"},
        {MINUS,"MINUS"},
        {MULT,"MULT"},
        {DIV, "DIV"},
        {REM, "REM"},
        {ASSOP, "ASSOP"},
        {LPAREN,"LPAREN"},
        {RPAREN, "RPAREN"},
        {COMMA, "COMMA"},
        {EQUAL, "EQUAL"},
        {GTHAN, "GTHAN"},
        {SEMICOL, "SEMICOL"},
        {ERR,"ERR"},
        {DONE, "DONE"}
};

static map<string, Token> keyword_map{
        {"PROGRAM", PROGRAM},
        {"WRITE", WRITE},
        {"INT", INT},
        {"END", END},
        {"IF", IF},
        {"FLOAT", FLOAT},
        {"STRING", STRING}
};

ostream& operator<<(ostream& out, const LexItem& tok) {
    Token selected_token = tok.GetToken();
    out << token_map[selected_token];
    if( selected_token == IDENT || selected_token == RCONST || selected_token == ICONST || selected_token == SCONST || selected_token == ERR ) {
        out << "(" << tok.GetLexeme() << ")";
    }
    return out;
}

LexItem id_or_kw(const string& lexeme, int linenum){
    Token selected_token = IDENT;
    auto it = keyword_map.find(lexeme);
    if(it != keyword_map.end()) {
        selected_token = it->second;
    }

    return LexItem(selected_token, lexeme, linenum);
}

LexItem getNextToken(istream& in, int& linenum){
    enum State { NEUTRAL, IN_IDENT, IN_STR, IN_COMMENT, IN_INT, IN_FLOAT, IN_EQ};
    State currState = NEUTRAL;
    string lexeme;
    char c;
    while(in.get(c)){
        if( c == '\n' ) {
            linenum++;
        }
        switch(currState){
            case NEUTRAL:
                if(isspace(c)){
                    continue;
                }
                lexeme = c;
                if(isalpha(c)){
                    currState = IN_IDENT;
                }else if(isdigit(c)){
                    currState = IN_INT;
                }else if(c=='"'){
                    currState = IN_STR;
                }else if(c=='.'){
                    currState = IN_FLOAT;
                }else if(c=='#'){
                    currState= IN_COMMENT;
                }else if(c=='='){
                    currState = IN_EQ;
                }else{
                    Token t;
                    switch (c) {
                        case '+': t = PLUS; break;
                        case '-': t=MINUS; break;
                        case '*': t=MULT; break;
                        case '/': t=DIV; break;
                        case '%': t=REM; break;
                        case '(': t=LPAREN; break;
                        case ')': t=RPAREN; break;
                        case ',': t=COMMA; break;
                        case '>': t=GTHAN; break;
                        case ';': t=SEMICOL; break;
                        default: t=ERR; break;
                    }
                    return LexItem(t,lexeme,linenum);
                }
                break;
            case IN_IDENT:
                if( isalpha(c) || isdigit(c) ) {
                    lexeme += c;
                }
                else {
                    in.putback(c);
                    return id_or_kw(lexeme, linenum);
                }
                break;
            case IN_STR:
                lexeme += c;
                if( c == '\n' ) {
                    return LexItem(ERR, lexeme, linenum );
                }
                if( c == '"' ) {
                    lexeme = lexeme.substr(1, lexeme.length()-2);
                    return LexItem(SCONST, lexeme, linenum );
                }
                break;
            case IN_INT:
                if( isdigit(c) ) {
                    lexeme += c;
                }
                else if( isalpha(c) ) {
                    lexeme += c;
                    return LexItem(ERR, lexeme, linenum);
                }
                else if(c=='.'){
                    lexeme +=c;
                    currState = IN_FLOAT;
                }else{
                    in.putback(c);
                    return LexItem(ICONST, lexeme, linenum);
                }
                break;
            case IN_FLOAT:
                if( isdigit(c) ) {
                    lexeme += c;
                }
                else if( isalpha(c) ) {
                    lexeme += c;
                    return LexItem(ERR, lexeme, linenum);
                }else{
                    in.putback(c);
                    return LexItem(ICONST, lexeme, linenum);
                }
                break;
            case IN_EQ:
                if(c=='='){
                    return LexItem(EQUAL,lexeme,linenum);
                }else{
                    return LexItem(ASSOP,lexeme,linenum);
                }
                break;
            case IN_COMMENT:
                if( c == '\n' ) {
                    currState = NEUTRAL;
                }
                break;
        }
    }
    if( in.eof() ) {
        return LexItem(DONE, "", linenum);
    }
    return LexItem(ERR, lexeme, linenum);
}