#ifndef LEX_H
#define LEX_H

#include <QObject>
#include <QTextStream>
#include <QThread>
#include <utility>
#include "globals.h"
#include <QPair>
#include <QTextDocument>
#include <QVector>

extern QMap<LexType,QString> lexName;
extern QMap<QString,LexType> reservedWords;

class Lex : public QThread
{
    Q_OBJECT
public:
    static Lex * getInstance(QTextDocument *doc){
        static auto *lex=new Lex();
        lex->doc=doc;
        return lex;

    }
    const Token * getTokenList();


signals:
    void token_get(Token *token,int flag=0);
    void ErrOut(QString msg);

public slots:


private:
    QTextDocument *doc;
    QTextStream ins;
    double intBuff;
    QString idBuff;
    Token *current;
    Token *head;
    int line_number;
    int sleep_time;
private:
    bool ischar(char c);
    bool isnum(char c);
    bool issinglesep(char c);
    Token *getsinglesep(char c);
    Token *lookup(QString str);


    // QThread interface
protected:
    void run() override;
};

#endif // LEX_H
