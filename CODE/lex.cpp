#include "lex.h"
#include "globals.h"
#include <QDebug>
#include <QFile>
#include <utility>
#include <string>

QMap<LexType, QString> lexName = {{PROGRAM, "PROGRAM"},
                                  {TYPE, "TYPE"},
                                  {VAR, "VAR"},
                                  {PROCEDURE, "PROCEDURE"},
                                  {BEGIN, "BEGIN"},
                                  {END, "END"},
                                  {ARRAY, "ARRAY"},
                                  {OF, "OF"},
                                  {RECORD, "RECORD"},
                                  {IF, "IF"},
                                  {THEN, "THEN"},
                                  {ELSE, "ELSE"},
                                  {FI, "FI"},
                                  {WHILE, "WHILE"},
                                  {DO, "DO"},
                                  {ENDWH, "ENDWH"},
                                  {READ, "READ"},
                                  {WRITE, "WRITE"},
                                  {RETURN, "RETURN"},
                                  {INTEGER_T, "INTEGER"},
                                  {CHAR_T, "CHAR"},
                                  {INTEGER_T, "INTEGER_T"},
                                  {CHAR_T, "CHAR_T"},
                                  {ID, "ID"},
                                  {INTC_VAL, "INTC_VAL"},
                                  {CHARC_VAL, "CHAR_VAL"},
                                  {ASSIGN, ":="},
                                  {EQ, "="},
                                  {LT, "<"},
                                  {PLUS, "+"},
                                  {MINUS, "-"},
                                  {TIMES, "*"},
                                  {DIVIDE, "/"},
                                  {LPAREN, "("},
                                  {RPAREN, ")"},
                                  {DOT, "."},
                                  {COLON, ":"},
                                  {SEMI, ";"},
                                  {COMMA, ","},
                                  {LMIDPAREN, "["},
                                  {RMIDPAREN, "]"},
                                  {UNDERRANGE, ".."},
                                  {ENDFILE, "EOF"},
                                  {ERROR, "ERROR"}};
QMap<QString, LexType> reservedWords = {
    {"program", PROGRAM}, {"type", TYPE},
    {"var", VAR},         {"procedure", PROCEDURE},
    {"begin", BEGIN},     {"end", END},
    {"array", ARRAY},     {"of", OF},
    {"record", RECORD},   {"if", IF},
    {"then", THEN},       {"else", ELSE},
    {"fi", FI},           {"char", CHAR_T},
    {"while", WHILE},     {"do", DO},
    {"endwh", ENDWH},     {"read", READ},
    {"write", WRITE},     {"return", RETURN},
    {"integer", INTEGER_T},
};
QMap<char, LexType> map = {{'+', PLUS},   {'-', MINUS},     {'*', TIMES},
                             {'/', DIVIDE}, {'(', LPAREN},    {')', RPAREN},
                             {';', SEMI},   {'[', LMIDPAREN}, {']', RMIDPAREN},
                             {'=', EQ},     {'<', LT},        {',', COMMA},
                             {EOF,ENDFILE}};


const Token *Lex::getTokenList() {
    if(ERROR==head->getLex()){
  Token *tmp = head;
  head = head->next;
  delete tmp;
    };
  return head;
}


bool Lex::ischar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lex::isnum(char c) { return (c >= (0 + '0') && c <= (9 + '0')); }

bool Lex::issinglesep(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' ||
         c == ';' || c == '[' || c == ']' || c == '=' || c == '<' || c == ',' ||c==EOF;
}

Token *Lex::getsinglesep(char c) {
  Token *tmp = new Token(line_number, map[c], lexName[map[c]]);

  return tmp;
}

Token *Lex::lookup(QString str) {
  Token *tmp;
  if (reservedWords.find(str) != reservedWords.end()) {
    tmp = new Token(line_number, reservedWords[str], str);
  } else {
    tmp = new Token(line_number, ID, str);
  }
  return tmp;
}

void Lex::run()
{
    auto str=new QString(doc->toPlainText());
    ins.setString(str);
    ins.seek(0);
    line_number=1;


    head = new Token(0, ERROR, "");
    current = head;
    char lookhead;
    while (!ins.atEnd()) {
      ins >> lookhead;

      if (ischar(lookhead)) {
        idBuff.append(lookhead);

        ins >> lookhead;
        while (ischar(lookhead) || isnum(lookhead)) {
          idBuff.append(lookhead);

          if (ins.atEnd())
            break;
          ins >> lookhead;
        }
        Token *tmp = lookup(idBuff);
        emit token_get(tmp);
        current->next = tmp;
        current = tmp;
        ins.seek(ins.pos() - 1);
        idBuff.clear();
      } else if (isnum(lookhead)) {
        idBuff.append(lookhead);

        ins >> lookhead;

        while (isdigit(lookhead)) {
          idBuff.append(lookhead);

          if (ins.atEnd())
            break;
          ins >> lookhead;
        }
        Token *tmp = new Token(line_number, INTC_VAL, idBuff);
        emit token_get(tmp);
        current->next = tmp;
        current = tmp;
        ins.seek(ins.pos() - 1);
        idBuff.clear();
      } else if (issinglesep(lookhead)) {
        Token *tmp = getsinglesep(lookhead);
        emit token_get(tmp);
        current->next = tmp;
        current = tmp;
      } else {
        switch (lookhead) {

        case '\n': {
          ++line_number;
          continue;
        }
        case ':': {
            idBuff.append(lookhead);

          char next;
          ins >> next;
            idBuff.append(next);
          if (next != '=') {
            ins.seek(ins.pos() - 1);
          } else {
            Token *tmp = new Token(line_number, ASSIGN, lexName[ASSIGN]);
            emit token_get(tmp);
            current->next = tmp;
            current = tmp;
          }
          idBuff.clear();


          break;
        }
        case '{': {
            idBuff.append(lookhead);
          while (lookhead != '}') {
            ins >> lookhead;

            idBuff.append(lookhead);
          }
          idBuff.clear();
          break;
        }
        case '.': {
            idBuff.append(lookhead);
            char next;
            ins >> next;
            idBuff.append(lookhead);
            Token *tmp;
          if (next == '.') {
            tmp = new Token(line_number, UNDERRANGE, lexName[UNDERRANGE]);
          } else {
                if (ins.atEnd())
                    ;
                else
                    ins.seek(ins.pos()-1); //没有到最后才能回退，否则最后一个会死循环
                tmp = new Token(line_number, DOT, lexName[DOT]);

          }
          idBuff.clear();
          emit token_get(tmp);
          current->next = tmp;
          current = tmp;
          break;
        }
        case '\'': {
            idBuff.append(lookhead);
          char next;
          ins >> next;


          if (isnum(next) || ischar(next)) {
            idBuff.append(next);
            ins >> next;
            idBuff.append(lookhead);
            if (next == '\'') {
              Token *tmp = new Token(line_number, CHARC_VAL, idBuff);
              current->next = tmp;
              current = tmp;
              emit token_get(tmp);
            } else {
              ins.seek(ins.pos() - 2);
            }
          } else {
            ins.seek(ins.pos() - 1);
          }
          idBuff.clear();
          break;
        }
        default: {

            if(' '!=lookhead&&32!=lookhead&&9!=lookhead){
                char tmpch[2];tmpch[0]=lookhead;tmpch[1]='\0';
                Token * tmp = new Token(line_number, tmpch);
                emit token_get(tmp,1);
            }
        }
        }
      }
    }
        Token *tmp = getsinglesep(EOF);
        current->next = tmp;
        current = tmp;

}
