#include "mainwindow.h"
#include <QApplication>

int lineno=0;
int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.setWindowTitle("SNL分析器");
    mainWin.setWindowIcon(QIcon(":/images/app.png"));
    mainWin.show();

    qRegisterMetaType<QVector<QPair<int,int>>>("QVector<QPair<int,int>>");
    qRegisterMetaType<QSharedPointer<TreeNode>>("QSharedPointer<TreeNode>");
    return app.exec();
}
