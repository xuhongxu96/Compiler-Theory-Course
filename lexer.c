#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

//#define _DEBUG_

enum Token {
    INT, FLOAT, ID, SEMI, COMMA, ASSIGNOP, RELOP, PLUS, MINUS, STAR, DIV, AND, OR,
    DOT, NOT, TYPE, LP, RP, LB, RB, LC, RC, STRUCT, RETURN, IF, ELSE, WHILE
};

char *TokenStr[] = {
    "INT", "FLOAT", "ID", "SEMI", "COMMA", "ASSIGNOP", "RELOP", "PLUS",
    "MINUS", "STAR", "DIV", "AND", "OR", "DOT", "NOT", "TYPE", "LP", "RP",
    "LB", "RB", "LC", "RC", "STRUCT", "RETURN", "IF", "ELSE", "WHILE"
};

union TokenVal {
    int intVal;
    float floatVal;
    char strVal[32];
    char desp[10];
};

struct TokenInfo {
    enum Token type;
    union TokenVal val;
};

enum LexerState {
    CREATED, RUNNING, FINISHED
};

enum ErrorType {
    NO_ERROR, NOT_A_DIGIT, NOT_A_HEX_DIGIT, EOF_ERR, INVALID_HEX_NUMBER,
    BEGIN_WITH_ZERO, INVALID_FLOAT, INVALID_EXPONENT, UNEXPECTED_TOKEN,
    INVALID_NUMBER
};

struct Lexer {
    FILE *fp;
    enum LexerState state;
    char ch;
    fpos_t last_pos;
};

void error(struct Lexer *lexer, enum ErrorType err);

char readChar(struct Lexer *lexer) {
    if (lexer->state == FINISHED || feof(lexer->fp)) {
        lexer->state = FINISHED;
        error(lexer, EOF_ERR);
        return 0;
    }
    if (EOF == (lexer->ch = fgetc(lexer->fp))) {
        lexer->state = FINISHED;
        return 0;
    }
    return lexer->ch;
}

struct Lexer *createLexer(FILE *fp) {
    struct Lexer *lexer = (struct Lexer *)malloc(sizeof(struct Lexer));
    rewind(fp);
    lexer->fp = fp;
    lexer->state = CREATED;
    lexer->last_pos = 0;
    return lexer;
}

bool isHexDigit(char ch) {
    if (isdigit(ch)) {
        return true;
    }
    ch = toupper(ch);
    if (ch <= 'F' && ch >= 'A') {
        return true;
    }
    return false;
}

int hex2int(char ch, int *n) {
    if (isdigit(ch)) {
        *n = ch - '0';
    } else if (isHexDigit(ch)) {
        ch = toupper(ch);
        *n = (ch - 'A' + 10);
    } else {
        return NOT_A_HEX_DIGIT;
    }
    return 0;
}

int char2int(char ch, int *n) {
    if (isdigit(ch)) {
        *n = ch - '0';
    } else {
        return NOT_A_DIGIT;
    }
    return 0;
}

#ifdef _DEBUG_

fpos_t getPos(struct Lexer *lexer) {
    fpos_t p;
    fgetpos(lexer->fp, &p);
    return p;
}

char getPosChar(struct Lexer *lexer, fpos_t p) {
    fpos_t temp;
    char ch;
    fgetpos(lexer->fp, &temp);
    fsetpos(lexer->fp, &p);
    ch = readChar(lexer);
    fsetpos(lexer->fp, &temp);
    return ch;
}

void outputPos(struct Lexer *lexer) {
    printf("!!! pos: %lld, ch: %c\n", getPos(lexer), getPosChar(lexer, getPos(lexer)));
}

#endif

void savePos(struct Lexer *lexer) {
    fgetpos(lexer->fp, &lexer->last_pos);
}

void restorePos(struct Lexer *lexer) {
    fsetpos(lexer->fp, &lexer->last_pos);
}

void spitChar(struct Lexer *lexer) {
    fseek(lexer->fp, -1, SEEK_CUR);
#ifdef _DEBUG_
    readChar(lexer);
    fseek(lexer->fp, -1, SEEK_CUR);
#endif
}

// Basic Eating Function

bool eatChar(struct Lexer *lexer, char ch) {
#ifdef _DEBUG_
    outputPos(lexer);
#endif
    readChar(lexer);
    if (lexer->ch == ch) {
        return true;
    }
    spitChar(lexer);
    return false;
}

bool eatDigit(struct Lexer *lexer) {
    readChar(lexer);
    if (isdigit(lexer->ch)) {
        return true;
    }
    spitChar(lexer);
    return false;
}

bool eatAlpha(struct Lexer *lexer) {
    readChar(lexer);
    if (isalpha(lexer->ch)) {
        return true;
    }
    spitChar(lexer);
    return false;
}

bool eatAlnum(struct Lexer *lexer) {
    readChar(lexer);
    if (isalnum(lexer->ch)) {
        return true;
    }
    spitChar(lexer);
    return false;
}

bool eatHexDigit(struct Lexer *lexer) {
    readChar(lexer);
    if (isHexDigit(lexer->ch)) {
        return true;
    }
    spitChar(lexer);
    return false;
}

bool eatSpace(struct Lexer *lexer) {
    readChar(lexer);
    if (isspace(lexer->ch)) {
        return true;
    }
    spitChar(lexer);
    return false;
}

int eatSpaces(struct Lexer *lexer) {
    int ct = 0;
    while (eatSpace(lexer)) {
        ++ct;
    }
    return ct;
}

void eatToSpace(struct Lexer *lexer) {
    readChar(lexer);
    do {
        readChar(lexer);
    } while (!isspace(lexer->ch));
}

void error(struct Lexer *lexer, enum ErrorType err) {
    printf(">>> ");
    fpos_t pos;
    fgetpos(lexer->fp, &pos);
    switch (err) {
        case NO_ERROR:
            printf("info: lexical analysis successful.");
            break;
        case NOT_A_DIGIT:
            printf("err: not a digit. pos: %lld.\n", pos);
            break;
        case NOT_A_HEX_DIGIT:
            printf("err: not a hex digit. pos: %lld\n", pos);
            break;
        case EOF_ERR:
            printf("err: end of file. pos: %lld\n", pos);
            break;
        case INVALID_HEX_NUMBER:
            printf("err: not a valid hex number. pos: %lld\n", pos);
            break;
        case BEGIN_WITH_ZERO:
            printf("err: decimal number begins with zero. pos: %lld\n", pos);
            break;
        case INVALID_FLOAT:
            printf("err: invalid float (missing integer or decimal part). pos: %lld\n", pos);
            break;
        case INVALID_EXPONENT:
            printf("err: invalid exponent (missing number after 'E'). pos: %lld\n", pos);
            break;
        case INVALID_NUMBER:
            printf("err: invalid number. pos: %lld\n", pos);
            break;
        case UNEXPECTED_TOKEN:
            printf("err: unexpected token. pos: %lld\n", pos);
            break;
    }
    printf(">>> err ch: %d - %c\n", lexer->ch, lexer->ch);
}


// Complex Eating Function

bool eatInt(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatInt\n");
#endif

    int res = 0;
    int base = 10;
    bool hasNumber = false;

    if (eatChar(lexer, '0')) {
        if (eatChar(lexer, 'x') || eatChar(lexer, 'X')) {
            base = 16;
        } else {
            base = 8;
        }
    }

    while (eatDigit(lexer) || (base == 16 && eatHexDigit(lexer))) {
        int n;
        int err = NO_ERROR;
        if (base == 16) {
            err = hex2int(lexer->ch, &n);
        } else {
            err = char2int(lexer->ch, &n);
        }
        if (base == 10 && n == 0 && !hasNumber) {
            err = BEGIN_WITH_ZERO;
        }
        hasNumber = true;
        if (err != NO_ERROR) {
            error(lexer, err);
            eatToSpace(lexer);
            return true;
        }
        res *= base;
        res += n;
    }

    if (hasNumber) {
        info->type = INT;
        info->val.intVal = res;
        return true;
    } else if (base == 8) {
        // 0
        res = 0;
        info->type = INT;
        info->val.intVal = res;
        return true;
    } else if (base == 16) {
        // found 0x but no other digits
        error(lexer, INVALID_HEX_NUMBER);
        eatToSpace(lexer);
        return true;
    }
    return false;
}

bool eatFloat(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatFloat\n");
    outputPos(lexer);
#endif
    float res = 0.0f;
    int integer = 0;
    bool hasInteger = false;
    bool hasDecimal = false;
    bool hasPoint = false;

    float decimal = 0.0f;
    int d = 0;

    savePos(lexer);
    while (eatDigit(lexer)) {
        hasInteger = true;
        int n;
        int err;

        if ((err = char2int(lexer->ch, &n)) != NO_ERROR) {
            error(lexer, err);
            eatToSpace(lexer);
            return true;
        }

        integer *= 10;
        integer += n;
    }
    if (!eatChar(lexer, '.')) {
#ifdef _DEBUG_
    outputPos(lexer);
#endif
        // error(lexer, NO_POINT_IN_FLOAT);
        restorePos(lexer);
#ifdef _DEBUG_
    outputPos(lexer);
#endif
        // return false;
    } else {

        hasPoint = true;
        while (eatDigit(lexer)) {
            hasDecimal = true;
            int n;
            int err;
            if ((err = char2int(lexer->ch, &n)) != NO_ERROR) {
                error(lexer, err);
                eatToSpace(lexer);
                return true;
            }
            ++d;
            decimal *= 10;
            decimal += n;
        }
        for (int i = 0; i < d; ++i) {
            decimal /= 10;
        }

    }

    if (eatChar(lexer, 'E') || eatChar(lexer, 'e')) {
        // scientific notation
        int sign = 1;
        int exp = 0;
        bool hasExp = false;

        if (eatChar(lexer, '-')) {
            sign = -1;
        } else {
            eatChar(lexer, '+');
        }

        while (eatDigit(lexer)) {
            hasExp = true;
            int n;
            int err;

            if ((err = char2int(lexer->ch, &n)) != NO_ERROR) {
                error(lexer, err);
                eatToSpace(lexer);
                return true;
            }

            exp *= 10;
            exp += n;
        }

        if (!hasExp) {
            error(lexer, INVALID_EXPONENT);
            eatToSpace(lexer);
            return true;
        }

        if (exp == 0) {
            res = 1;
        }

        res = (integer + decimal);

        for (int i = 0; i < exp; ++i) {
            if (sign == 1) {
                res *= 10;
            } else {
                res /= 10;
            }
        }
    } else {

        if (!hasPoint) {
            return false;
        }
        if (hasPoint && (!hasInteger || !hasDecimal)) {
            error(lexer, INVALID_FLOAT);
            eatToSpace(lexer);
            return true;
        }
        res = integer + decimal;
    }

    info->type = FLOAT;
    info->val.floatVal = res;

    return true;
}

bool eatIdentifier(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatID\n");
#endif
    int i = 0;
    if (eatChar(lexer, '_') || eatAlpha(lexer)) {
        (info->val.strVal)[i++] = lexer->ch;
        while (eatChar(lexer, '_') || eatAlnum(lexer)) {
            (info->val.strVal)[i++] = lexer->ch;
        }
    } else {
        return false;
    }
    info->type = ID;
    (info->val.strVal)[i] = '\0';
    return true;
}

bool eatSemi(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatSemi\n");
#endif
    if (eatChar(lexer, ';')) {
        info->type = SEMI;
        info->val.desp[0] = ';';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatComma(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatComma\n");
#endif
    if (eatChar(lexer, ',')) {
        info->type = COMMA;
        info->val.desp[0] = ',';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatAssignOp(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatAssignOp\n");
#endif
    if (eatChar(lexer, '=')) {
        info->type = ASSIGNOP;
        info->val.desp[0] = '=';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatRelop(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatRelop\n");
#endif
    info->type = RELOP;
    if (eatChar(lexer, '<')) {
        if (eatChar(lexer, '=')) {
            // <=
            info->val.desp[0] = '<';
            info->val.desp[1] = '=';
            info->val.desp[2] = '\0';
        } else {
            // <
            info->val.desp[0] = '<';
            info->val.desp[1] = '\0';
        }
        return true;
    }
    if (eatChar(lexer, '>')) {
        if (eatChar(lexer, '=')) {
            // >=
            info->val.desp[0] = '>';
            info->val.desp[1] = '=';
            info->val.desp[2] = '\0';
        } else {
            // >
            info->val.desp[0] = '>';
            info->val.desp[1] = '\0';
        }
        return true;
    }
    if (eatChar(lexer, '!')) {
        if (eatChar(lexer, '=')) {
            // !=
            info->val.desp[0] = '!';
            info->val.desp[1] = '=';
            info->val.desp[2] = '\0';
            return true;
        } else {
            spitChar(lexer);
        }
    }
    if (eatChar(lexer, '=')) {
        if (eatChar(lexer, '=')) {
            // ==
            info->val.desp[0] = '=';
            info->val.desp[1] = '=';
            info->val.desp[2] = '\0';
            return true;
        } else {
            spitChar(lexer);
        }
    }
    return false;
}

bool eatPlus(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatPlus\n");
#endif
    if (eatChar(lexer, '+')) {
        info->type = PLUS;
        info->val.desp[0] = '+';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatMinus(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatMinus\n");
#endif
    if (eatChar(lexer, '-')) {
        info->type = MINUS;
        info->val.desp[0] = '-';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatStar(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatStar\n");
#endif
    if (eatChar(lexer, '*')) {
        info->type = STAR;
        info->val.desp[0] = '*';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatDiv(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatDiv\n");
#endif
    if (eatChar(lexer, '/')) {
        info->type = DIV;
        info->val.desp[0] = '/';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatAnd(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatAnd\n");
#endif
    if (eatChar(lexer, '&')) {
        if (eatChar(lexer, '&')) {
            info->type = AND;
            info->val.desp[0] = '&';
            info->val.desp[1] = '&';
            info->val.desp[2] = '\0';
            return true;
        } else {
            spitChar(lexer);
        }
    }
    return false;
}

bool eatOr(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatOr\n");
#endif
    if (eatChar(lexer, '|')) {
        if (eatChar(lexer, '|')) {
            info->type = OR;
            info->val.desp[0] = '|';
            info->val.desp[1] = '|';
            info->val.desp[2] = '\0';
            return true;
        } else {
            spitChar(lexer);
        }
    }
    return false;
}

bool eatDot(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatDot\n");
#endif
    if (eatChar(lexer, '.')) {
        info->type = DOT;
        info->val.desp[0] = '.';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatNot(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatNot\n");
#endif
    if (eatChar(lexer, '!')) {
        info->type = NOT;
        info->val.desp[0] = '!';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatLP(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatLP\n");
#endif
    if (eatChar(lexer, '(')) {
        info->type = LP;
        info->val.desp[0] = '(';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatRP(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatRP\n");
#endif
    if (eatChar(lexer, ')')) {
        info->type = RP;
        info->val.desp[0] = ')';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatLB(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatLB\n");
#endif
    if (eatChar(lexer, '[')) {
        info->type = LB;
        info->val.desp[0] = '[';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatRB(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatRB\n");
#endif
    if (eatChar(lexer, ']')) {
        info->type = RB;
        info->val.desp[0] = ']';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatLC(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatLC\n");
#endif
    if (eatChar(lexer, '{')) {
        info->type = LC;
        info->val.desp[0] = '{';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatRC(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatRC\n");
#endif
    if (eatChar(lexer, '}')) {
        info->type = RC;
        info->val.desp[0] = '}';
        info->val.desp[1] = '\0';
        return true;
    }
    return false;
}

bool eatType(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatType\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 'i')) {
        if (eatChar(lexer, 'n')) {
            if (eatChar(lexer, 't')) {
                info->type = TYPE;
                strcpy(info->val.desp, "int");
                return true;
            }
        }
    }
    restorePos(lexer);
    savePos(lexer);
    if (eatChar(lexer, 'f')) {
        if (eatChar(lexer, 'l')) {
            if (eatChar(lexer, 'o')) {
                if (eatChar(lexer, 'a')) {
                    if (eatChar(lexer, 't')) {
                        info->type = TYPE;
                        strcpy(info->val.desp, "float");
                        return true;
                    }
                }
            }
        }
    }
    restorePos(lexer);
    return false;
}

bool eatStruct(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatStruct\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 's')) {
        if (eatChar(lexer, 't')) {
            if (eatChar(lexer, 'r')) {
                if (eatChar(lexer, 'u')) {
                    if (eatChar(lexer, 'c')) {
                        if (eatChar(lexer, 't')) {
                            info->type = STRUCT;
                            strcpy(info->val.desp, "struct");
                            return true;
                        }
                    }
                }
            }
        }
    }
    restorePos(lexer);
    return false;
}

bool eatReturn(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatReturn\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 'r')) {
        if (eatChar(lexer, 'e')) {
            if (eatChar(lexer, 't')) {
                if (eatChar(lexer, 'u')) {
                    if (eatChar(lexer, 'r')) {
                        if (eatChar(lexer, 'n')) {
                            info->type = RETURN;
                            strcpy(info->val.desp, "return");
                            return true;
                        }
                    }
                }
            }
        }
    }
    restorePos(lexer);
    return false;
}

bool eatIf(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatIf\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 'i')) {
        if (eatChar(lexer, 'f')) {
            info->type = IF;
            strcpy(info->val.desp, "if");
            return true;
        }
    }
    restorePos(lexer);
    return false;
}

bool eatElse(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatElse\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 'e')) {
        if (eatChar(lexer, 'l')) {
            if (eatChar(lexer, 's')) {
                if (eatChar(lexer, 'e')) {
                    info->type = ELSE;
                    strcpy(info->val.desp, "else");
                    return true;
                }
            }
        }
    }
    restorePos(lexer);
    return false;
}

bool eatWhile(struct Lexer *lexer, struct TokenInfo *info) {
#ifdef _DEBUG_
    printf("eatWhile\n");
#endif
    savePos(lexer);
    if (eatChar(lexer, 'w')) {
        if (eatChar(lexer, 'h')) {
            if (eatChar(lexer, 'i')) {
                if (eatChar(lexer, 'l')) {
                    if (eatChar(lexer, 'e')) {
                        info->type = WHILE;
                        strcpy(info->val.desp, "while");
                        return true;
                    }
                }
            }
        }
    }
    restorePos(lexer);
    return false;
}

void outputToken(struct TokenInfo token) {
    if (token.type == INT) {
        printf("%s: %d", TokenStr[token.type], token.val.intVal);
    } else if (token.type == FLOAT) {
        printf("%s: %f", TokenStr[token.type], token.val.floatVal);
    } else if (token.type == ID) {
        printf("%s: %s", TokenStr[token.type], token.val.strVal);
    } else {
        printf("%s: %s", TokenStr[token.type], token.val.desp);
    }
    printf("\n");
}

void eatToken(struct Lexer *lexer) {
    struct TokenInfo info;
    while (lexer->state != FINISHED) {
        int n = eatSpaces(lexer);
        if (lexer->state == FINISHED) {
            break;
        }
#ifdef _DEBUG_
        printf("%d spaces eaten.\n", n);
        savePos(lexer);
        printf("pos: %lld, ch: %c\n", lexer->last_pos, lexer->ch);
#endif
        if (eatSemi(lexer, &info)) {
        } else if (eatComma(lexer, &info)) {
        } else if (eatRelop(lexer, &info)) {
        } else if (eatAssignOp(lexer, &info)) {
        } else if (eatPlus(lexer, &info)) {
        } else if (eatMinus(lexer, &info)) {
        } else if (eatStar(lexer, &info)) {
        } else if (eatDiv(lexer, &info)) {
        } else if (eatAnd(lexer, &info)) {
        } else if (eatOr(lexer, &info)) {
        } else if (eatDot(lexer, &info)) {
        } else if (eatNot(lexer, &info)) {
        } else if (eatType(lexer, &info)) {
        } else if (eatLP(lexer, &info)) {
        } else if (eatRP(lexer, &info)) {
        } else if (eatLB(lexer, &info)) {
        } else if (eatRB(lexer, &info)) {
        } else if (eatLC(lexer, &info)) {
        } else if (eatRC(lexer, &info)) {
        } else if (eatStruct(lexer, &info)) {
        } else if (eatReturn(lexer, &info)) {
        } else if (eatIf(lexer, &info)) {
        } else if (eatElse(lexer, &info)) {
        } else if (eatWhile(lexer, &info)) {
        } else if (eatFloat(lexer, &info)) {
        } else if (eatInt(lexer, &info)) {
        } else if (eatIdentifier(lexer, &info)) {
        } else {
            error(lexer, UNEXPECTED_TOKEN);
            readChar(lexer);
            continue;
        }
        outputToken(info);

    }
}
