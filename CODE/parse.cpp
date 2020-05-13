#include "parse.h"
#include "utils.h"
#include <QDebug>

void Parse::run() {
    root = program();
    if (head->getLex() != ENDFILE) {
        syntaxError("end earily in line:"+QString::number(head->getLine()));
    }
        emit parse_success(QSharedPointer<TreeNode>(root),"递归下降");
}
QVector<QString> Parse::msg = QVector<QString>();
Parse::Parse(const Token *root):head(root)
{

}

TreeNode *Parse::program() {
    TreeNode *ph = programHead();
    TreeNode *dp = declarePart();
    TreeNode *pb = programBody();

    root = newRootNode();
    if (nullptr != root) {
        root->lineno = 0;
        if (nullptr != ph) root->child[0] = ph;
        else syntaxError("need a program head");
        if (nullptr != dp) root->child[1] = dp;
        if (nullptr != pb) root->child[2] = pb;
        else syntaxError("nedd a program body");
    }
    match(DOT);

    return root;
}

void Parse::syntaxError(QString msg) {
    msg.push_back(msg);
    qDebug()<< msg;
}

bool Parse::match(LexType expected) {
    if (nullptr!=head&&head->getLex() == expected) {
        head = head->next;
        if(head!=nullptr){
        line0 = head->getLine();
        lineno=line0;

        return true;
        }
    } else {
        syntaxError("ERROR not match,except:"+lexName[expected]+" get:"+lexName[head->getLex()]+" in line "+head->getLine());
        return false;
    }
    return false;
}

TreeNode *Parse::programHead() {
    TreeNode *t = newPheadNode();
    if(!match(PROGRAM))
        goto e;
    if ((nullptr != t) && (head->getLex() == ID)) {
        t->lineno = head->getLine();
        strcpy(t->name[0], head->getSem().toStdString().c_str());
    if(!match(ID))
        goto e;
    }else{
        syntaxError(&"need a program name in line:" [ lineno]);
    }
    return t;
    e:
    syntaxError(&"PROGRAM error in line:" [ lineno]);
    delete t;
    t=nullptr;
    return t;
}

TreeNode *Parse::declarePart() {
    TreeNode *tp = newDecANode(TypeK);
    TreeNode *pp = tp;
    if (nullptr != tp) {
        tp->lineno = 0;//TODO lineno=lineno??
        TreeNode *tp1 = typeDec();
        if (nullptr != tp1)
            tp->child[0] = tp1;

        else {
            free(tp);
            tp = nullptr;
        }
    }
    TreeNode *varp = newDecANode(VarK);
    if (nullptr != varp) {
        varp->lineno = 0;//TODO lineno??
        TreeNode *tp2 = varDec();
        if (nullptr != tp2)
            varp->child[0] = tp2;
        else {
            free(varp);
            varp = nullptr;
        }
    }

    TreeNode *proc = procDec();
    if (nullptr == proc) {}
    if (nullptr == varp) { varp = proc; };
    if (nullptr == tp) { pp = tp = varp; };
    if (tp != varp) {
        tp->sibling = varp;
        tp = varp;
    }
    if (varp != proc) {
        varp->sibling = proc;
        varp = proc;
    };
    return pp;
}
TreeNode *Parse::typeDec() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case TYPE:
            t = typeDeclaration();
            break;
        case VAR:
        case PROCEDURE:
        case BEGIN:
            break;
        default :
            syntaxError("unexpected token:" + head->getSem()+" in line :"+QString::number(lineno));
            head = head->next;
            break;
    }
    return t;
}
TreeNode *Parse::typeDeclaration() {
    if(!match(TYPE))
        syntaxError("Warning :need a declaration");
    auto t = typeDecList();
    if (nullptr == t) {
        syntaxError("Warning :need a declaration");
    }
    return t;

}

TreeNode *Parse::typeDecList() {
    auto t = newDecNode();
    if (nullptr != t) {
        t->lineno = line0;
        typeId(t);
        if(!match(EQ))
            goto e;
        typeName(t);
        if(!match(SEMI))
            goto e;
        auto *more = typeDecMore();
        if (nullptr != more) {
            t->sibling = more;
        }
    }

    return t;

    e:
    delete t;
    t=nullptr;
    return t;
}

void Parse::typeId(TreeNode *pNode) {

    int tnum = pNode->idnum;
    if (head->getLex() == ID) {
        strcpy(pNode->name[tnum], head->getSem().toStdString().c_str());
        tnum += 1;
    }else{
        syntaxError("Type Define need a ID");
    }
    pNode->idnum = tnum;
    match(ID);

}

void Parse::typeName(TreeNode *pNode) {
    if (nullptr != pNode) {
        switch (head->getLex()) {
            case INTEGER_T:
            case CHAR_T:
                baseType(pNode);
                break;
            case ARRAY:
            case RECORD:
                structureType(pNode);
                break;
            case ID:
                pNode->kind.dec = IdK;
                strcpy(pNode->type_name, head->getSem().toStdString().c_str());
                match(ID);
                break;
            default :
                head = head->next;
                syntaxError("unexpected Type");
                break;
        }
    }

}

void Parse::baseType(TreeNode *pNode) {
    switch (head->getLex()) {
        case INTEGER_T:
            match(INTEGER_T);
            pNode->kind.dec = IntegerK;
            break;
        case CHAR_T:
            match(CHAR_T);
            pNode->kind.dec = CharK;
            break;
        default:
            head = head->next;
            syntaxError("unexpected BaseType");
            break;
    }

}

void Parse::structureType(TreeNode *pNode) {
    switch (head->getLex()) {
        case ARRAY:
            arrayType(pNode);
            break;
        case RECORD:
            pNode->kind.dec = RecordK;
            recType(pNode);
            break;
        default:
            head = head->next;
            syntaxError("unexpected StructureType");
            break;
    }
}

void Parse::arrayType(TreeNode *pNode) {
    match(ARRAY);
    match(LMIDPAREN);
    if (head->getLex() == INTC_VAL) {
        pNode->attr.ArrayAttr.low = head->getSem().toInt();
    }
    match(INTC_VAL);
    match(UNDERRANGE);
    if (head->getLex() == INTC_VAL) {
        pNode->attr.ArrayAttr.up = head->getSem().toInt();
    }
    match(INTC_VAL);
    match(RMIDPAREN);
    match(OF);
    baseType(pNode);
    pNode->attr.ArrayAttr.childtype = pNode->kind.dec;
    pNode->kind.dec = ArrayK;


}

void Parse::recType(TreeNode *pNode) {
    match(RECORD);
    TreeNode *p = nullptr;
    p = fieldDecList();
    if (nullptr != p) {
        pNode->child[0] = p;
    } else syntaxError("need a record body");
    match(END);


}

TreeNode *Parse::fieldDecList() {
    auto *t = newDecNode();
    TreeNode *p = nullptr;
    t->lineno = line0;
    switch (head->getLex()) {
        case INTEGER_T:
        case CHAR_T:
            baseType(t);
            idList(t);
            match(SEMI);
            p = fieldDecMore();
            break;
        case ARRAY:
            arrayType(t);
            idList(t);
            match(SEMI);
            p = fieldDecMore();
            break;
        default :
            syntaxError("unexpected Type . Only accept INTEGER CHAR ARRAY");
            break;
    }
    t->sibling = p;
    return NULL;
}
void Parse::idList(TreeNode *pNode) {
    if (head->getLex() == ID) {
        strcpy(pNode->name[pNode->idnum], head->getSem().toStdString().c_str());
        match(ID);
        pNode->idnum += 1;
    }else{
        syntaxError("IDList need an ID but "+lexName[head->getLex()]+" get");
    }
    idMore(pNode);

}
void Parse::idMore(TreeNode *pNode) {
    switch (head->getLex()) {
        case SEMI:
            break;
        case COMMA:
            match(COMMA);
            idList(pNode);
            break;
        default :
            syntaxError("unexpected token in idMore");
            break;
    }
}

TreeNode *Parse::fieldDecMore() {
    TreeNode *p = nullptr;
    switch (head->getLex()) {
        case END:
            break;
        case INTEGER_T:
        case CHAR_T:
        case ARRAY:
            p = fieldDecList();
            break;
        default :
            syntaxError("unexpected token "+lexName[head->getLex()]);
            break;
    }
    return p;
}

TreeNode *Parse::typeDecMore() {
    TreeNode *p = nullptr;
    switch (head->getLex()) {
        case VAR:
        case PROCEDURE:
        case BEGIN:
            break;
        case ID:
            p = typeDecList();
            break;

        default:
            syntaxError("unexpected token when declare more Type");
            break;
    }
    return p;
}
TreeNode *Parse::varDec() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case PROCEDURE:
        case BEGIN:
            break;
        case VAR:
            t = varDeclaration();
            break;
        default:
            syntaxError("unexpected token is here! "+lexName[head->getLex()]+head->getLine());
            break;
    }
    return t;
}
TreeNode *Parse::varDeclaration() {
    match(VAR);
    TreeNode *t = varDecList();
    if (t == nullptr)
        syntaxError("a var declaration is expected!");
    return t;
}
TreeNode *Parse::varDecList() {
    TreeNode *t = newDecNode();
    TreeNode *p = nullptr;

    if (t != nullptr) {
        t->lineno = line0;
        typeName(t);
        varIdList(t);
        match(SEMI);
        p = varDecMore();
        t->sibling = p;
    }
    return t;
}
TreeNode *Parse::varDecMore() {
    TreeNode *t = nullptr;

    switch (head->getLex()) {
        case PROCEDURE:
        case BEGIN:
            break;
        case INTEGER_T:
        case CHAR_T:
        case ARRAY:
        case RECORD:
        case ID:
            t = varDecList();
            break;
        default:
            syntaxError("unexpected token"+head->getLexName()+" is here in line:"+QString::number(lineno));
            break;
    }
    return t;
}
void Parse::varIdList(TreeNode *t) {
    if (head->getLex() == ID) {
        strcpy(t->name[(t->idnum)], head->getSem().toStdString().c_str());
        match(ID);
        t->idnum = (t->idnum) + 1;
    } else {
        syntaxError("a varid is expected here! in line:"+QString::number(lineno));
    }
    varIdMore(t);
}
void Parse::varIdMore(TreeNode *t) {
    switch (head->getLex()) {
        case SEMI:
            break;
        case COMMA:
            match(COMMA);
            varIdList(t);
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+ "is here! in line:"+QString::number(lineno));
            break;
    }
}

TreeNode *Parse::programBody() {
    TreeNode *t = newStmlNode();
    match(BEGIN);
    if (t != nullptr) {
        t->lineno = 0;
        t->child[0] = stmList();
    }
    match(END);
    return t;
}

TreeNode *Parse::stmList() {
    TreeNode *t = stm();
    TreeNode *p = stmMore();
    if (t != nullptr)
        if (p != nullptr)
            t->sibling = p;
    return t;
}
TreeNode *Parse::stmMore() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case ELSE:
        case FI:
        case END:
        case ENDWH:
            break;
        case SEMI:
            match(SEMI);
            t = stmList();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+"is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}
TreeNode *Parse::stm() {

    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case IF:
            t = conditionalStm();
            break;
        case WHILE:
            t = loopStm();
            break;
        case READ:
            t = inputStm();
            break;
        case WRITE:
            t = outputStm();
            break;
        case RETURN:
            t = returnStm();
            break;
        case ID:
            temp_name = head->getSem();
            match(ID);
            t = assCall();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+"is here! in  line:"+QString::number(lineno));
            break;
    }
    return t;
}
TreeNode *Parse::assCall() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case ASSIGN:
        case LMIDPAREN:
        case DOT:
            t = assignmentRest();
            break;
        case LPAREN:
            t = callStmRest();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+"is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}

TreeNode *Parse::assignmentRest() {
    TreeNode *t = newStmtNode(AssignK);
    if (t != nullptr) {
        t->lineno = line0;


        TreeNode *child1 = newExpNode(VariK);
        if (child1 != nullptr) {
            child1->lineno = line0;
            strcpy(child1->name[0], temp_name.toStdString().c_str());
            (child1->idnum)++;
            variMore(child1);
            t->child[0] = child1;
        }


        match(ASSIGN);


        t->child[1] = mexp();

    }
    return t;
}

TreeNode *Parse::conditionalStm() {
    TreeNode *t = newStmtNode(IfK);
    match(IF);
    if (t != nullptr) {
        t->lineno = line0;
        t->child[0] = mexp();
    }
    match(THEN);
    if (t != nullptr) t->child[1] = stmList();
    if (head->getLex() == ELSE) {
        match(ELSE);
        if (t != nullptr)
            t->child[2] = stmList();
    }
    match(FI);
    return t;
}

TreeNode *Parse::loopStm() {
    TreeNode *t = newStmtNode(WhileK);
    match(WHILE);
    if (t != nullptr) {
        t->lineno = line0;
        t->child[0] = mexp();
        match(DO);
        t->child[1] = stmList();
        match(ENDWH);
    }
    return t;
}

TreeNode *Parse::inputStm() {
    TreeNode *t = newStmtNode(ReadK);
    match(READ);
    match(LPAREN);
    if ((t != nullptr) && (head->getLex() == ID)) {
        t->lineno = line0;
        strcpy(t->name[0], head->getSem().toStdString().c_str());
        (t->idnum)++;
    }
    match(ID);
    match(RPAREN);
    return t;
}

TreeNode *Parse::outputStm() {
    TreeNode *t = newStmtNode(WriteK);
    match(WRITE);
    match(LPAREN);
    if (t != nullptr) {
        t->lineno = line0;
        t->child[0] = mexp();
    }
    match(RPAREN);
    return t;
}

TreeNode *Parse::returnStm() {
    TreeNode *t = newStmtNode(ReturnK);
    match(RETURN);
    if (t != nullptr)
        t->lineno = line0;
    return t;
}

TreeNode *Parse::callStmRest() {
    TreeNode *t = newStmtNode(CallK);
    match(LPAREN);
    if (t != nullptr) {
        t->lineno = line0;


        TreeNode *child0 = newExpNode(VariK);
        if (child0 != nullptr) {
            child0->lineno = line0;
            strcpy(child0->name[0], temp_name.toStdString().c_str());
            (child0->idnum)++;
            t->child[0] = child0;
        }
        t->child[1] = actParamList();
    }
    match(RPAREN);
    return t;
}

TreeNode *Parse::actParamList() {
    TreeNode *t = nullptr;

    switch (head->getLex()) {
        case RPAREN:
            break;
        case ID:
        case INTC_VAL:
            t = mexp();
            if (t != nullptr)
                t->sibling = actParamMore();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}

TreeNode *Parse::actParamMore() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case RPAREN:
            break;
        case COMMA:
            match(COMMA);
            t = actParamList();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}

TreeNode *Parse::mexp() {

    TreeNode *t = simple_exp();


    if ((head->getLex() == LT) || (head->getLex() == EQ)) {

        TreeNode *p = newExpNode(OpK);


        if (p != nullptr) {
            p->lineno = line0;
            p->child[0] = t;
            p->attr.ExpAttr.op = head->getLex();


            t = p;
        }


        match(head->getLex());


        if (t != nullptr)
            t->child[1] = simple_exp();
    }


    return t;
}

TreeNode *Parse::simple_exp() {

    TreeNode *t = term();


    while ((head->getLex() == PLUS) || (head->getLex() == MINUS)) {

        TreeNode *p = newExpNode(OpK);


        if (p != nullptr) {
            p->lineno = line0;
            p->child[0] = t;
            p->attr.ExpAttr.op = head->getLex();


            t = p;


            match(head->getLex());


            t->child[1] = term();
        }
    }

    return t;
}
TreeNode *Parse::procDec() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case BEGIN:
            break;
        case PROCEDURE:
            t = procDeclaration();
            break;
        default:
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}
void Parse::formList(TreeNode *t) {
    if (head->getLex() == ID) {
        strcpy(t->name[(t->idnum)], head->getSem().toStdString().c_str());
        t->idnum = (t->idnum) + 1;
        match(ID);
    }
    fidMore(t);
}
void Parse::fidMore(TreeNode *t) {
    switch (head->getLex()) {
        case SEMI:
        case RPAREN:
            break;
        case COMMA:
            match(COMMA);
            formList(t);
            break;
        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
}
TreeNode *Parse::procDeclaration() {
    TreeNode *t = newProcNode();
    match(PROCEDURE);
    if (t != nullptr) {
        t->lineno = line0;
        if (head->getLex() == ID) {
            strcpy(t->name[0], head->getSem().toStdString().c_str());
            (t->idnum)++;
            match(ID);
        }
        match(LPAREN);
        paramList(t);
        match(RPAREN);
        match(SEMI);
        t->child[1] = procDecPart();
        t->child[2] = procBody();
        t->sibling = procDec();
    }
    return t;
}
TreeNode *Parse::procBody() {
    TreeNode *t = programBody();
    if (t == nullptr)
        syntaxError("a program body is requested!");
    return t;
}

TreeNode *Parse::procDecPart() {
    TreeNode *t = declarePart();
    return t;
}

void Parse::paramList(TreeNode *t) {
    TreeNode *p = nullptr;

    switch (head->getLex()) {
        case RPAREN:
            break;
        case INTEGER_T:
        case CHAR_T:
        case ARRAY:
        case RECORD:
        case ID:
        case VAR:
            p = paramDecList();
            t->child[0] = p;
            break;
        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
}

TreeNode *Parse::paramDecList() {
    TreeNode *t = mparam();
    TreeNode *p = paramMore();
    if (p != nullptr) {
        t->sibling = p;
    }
    return t;
}

TreeNode *Parse::paramMore() {
    TreeNode *t = nullptr;
    switch (head->getLex()) {
        case RPAREN:
            break;
        case SEMI:
            match(SEMI);
            t = paramDecList();
            if (t == nullptr)
                syntaxError("a param declaration is request!");
            break;
        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}

TreeNode *Parse::mparam() {
    TreeNode *t = newDecNode();
    if (t != nullptr) {
        t->lineno = line0;
        switch (head->getLex()) {
            case INTEGER_T:
            case CHAR_T:
            case ARRAY:
            case RECORD:
            case ID:
                t->attr.ProcAttr.paramt = valparamType;
                typeName(t);
                formList(t);
                break;
            case VAR:
                match(VAR);
                t->attr.ProcAttr.paramt = varparamType;
                typeName(t);
                formList(t);
                break;
            default:
                head = head->next;

            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
                break;
        }
    }
    return t;
}

TreeNode *Parse::term() {

    TreeNode *t = factor();


    while ((head->getLex() == TIMES) || (head->getLex() == DIVIDE)) {

        treeNode *p = newExpNode(OpK);


        if (p != nullptr) {
            p->lineno = line0;
            p->child[0] = t;
            p->attr.ExpAttr.op = head->getLex();
            t = p;
        }


        match(head->getLex());


        p->child[1] = factor();

    }

    return t;
}
TreeNode *Parse::factor() {

    TreeNode *t = nullptr;

    switch (head->getLex()) {
        case INTC_VAL :
            t = newExpNode(ConstK);

            if ((t != nullptr) && (head->getLex() == INTC_VAL)) {
                t->lineno = line0;
                t->attr.ExpAttr.val = head->getSem().toInt();
            }
            match(INTC_VAL);
            break;
        case ID :
            t = variable();
            break;
        case LPAREN :
            match(LPAREN);
            t = mexp();
            match(RPAREN);
            break;

        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
    return t;
}
TreeNode *Parse::variable() {
    TreeNode *t = newExpNode(VariK);

    if ((t != nullptr) && (head->getLex() == ID)) {
        t->lineno = line0;
        strcpy(t->name[0], head->getSem().toStdString().c_str());
        (t->idnum)++;
    }

    match(ID);
    variMore(t);
    return t;
}
void Parse::variMore(TreeNode *t) {
    switch (head->getLex()
            ) {
        case ASSIGN:
        case TIMES:
        case EQ:
        case LT:
        case PLUS:
        case MINUS:
        case DIVIDE:
        case RPAREN:
        case RMIDPAREN:
        case SEMI:
        case COMMA:
        case THEN:
        case ELSE:
        case FI:
        case DO:
        case ENDWH:
        case END:
            break;
        case LMIDPAREN:
            match(LMIDPAREN);


            t->child[0] = mexp();

            t->attr.ExpAttr.varkind = ArrayMembV;


            t->child[0]->attr.ExpAttr.varkind = IdV;
            match(RMIDPAREN);
            break;
        case DOT:
            match(DOT);

            t->child[0] = fieldvar();

            t->attr.ExpAttr.varkind = FieldMembV;

            t->child[0]->attr.ExpAttr.varkind = IdV;
            break;
        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
}
TreeNode *Parse::fieldvar() {

    TreeNode *t = newExpNode(VariK);

    if ((t != nullptr) && (head->getLex() == ID)) {
        t->lineno = line0;
        strcpy(t->name[0], head->getSem().toStdString().c_str());
        (t->idnum)++;
    }
    match(ID);

    fieldvarMore(t);

    return t;
}

void Parse::fieldvarMore(TreeNode *t) {
    switch (head->getLex()) {
        case ASSIGN:
        case TIMES:
        case EQ:
        case LT:
        case PLUS:
        case MINUS:
        case DIVIDE:
        case RPAREN:
        case SEMI:
        case COMMA:
        case THEN:
        case ELSE:
        case FI:
        case DO:
        case ENDWH:
        case END:
            break;
        case LMIDPAREN:
            match(LMIDPAREN);
            t->child[0] = mexp();
            t->child[0]->attr.ExpAttr.varkind = ArrayMembV;
            match(RMIDPAREN);
            break;
        default:
            head = head->next;
            syntaxError("unexpected token "+head->getLexName()+" is here! in line:"+QString::number(lineno));
            break;
    }
}
