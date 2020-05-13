#ifndef LL1_PARSE_H
#define LL1_PARSE_H

#include "globals.h"
#include <QObject>
#include <QSharedPointer>
#include <QStack>
#include <QThread>
#include <QVector>
#include <QString>
extern QSet<LexType> NTSet;
extern QSet<LexType> TTSet;

extern int lineno;
extern QMap<LexType, QString> lexName ;
class LL1_parse:public QThread
{
    Q_OBJECT
public:

    static LL1_parse *getInstance(const Token *head){
        auto instance= new LL1_parse(head);
        return instance;
    }
    TreeNode *get_parsetree_head();
    static QVector<QString>msg;
    void ErrOut(QString err);
protected:
    void run() override;

signals:
    void parse_success(QSharedPointer<TreeNode> p,QString title);

private:
    LL1_parse(const Token *root);
    void createLL1Table();
    void process(int id);
private:
    const Token *head;
    QMap<QPair<LexType,LexType>,int> table;
    QStack<LexType> symbol_stack;
    QStack<TreeNode **> syntaxtree_stack;
    QStack<TreeNode *> op_stack;
    QStack<TreeNode *> num_stack;
    TreeNode *root;

};

#endif // LL1_PARSE_H
