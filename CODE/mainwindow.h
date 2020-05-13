#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lex.h"
#include "draw.h"
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextStream>
#include <QThread>
#include <QGraphicsView>
#include "liner.h"
#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QFontComboBox>
#include <QToolButton>
#include <QPlainTextEdit>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    void init_ui();

private:
    Ui::MainWindow *ui;

    QSplitter *rightsplitter;
    QPlainTextEdit *editer_left;
    QTabWidget *tab_center_widget;
    QGraphicsView *pview;
    ParseScene *parseScene;
    QVBoxLayout *layout_right;
    QHBoxLayout *layout_left;
    QHBoxLayout *layout;
    QLabel *label_current_char;
    QLabel *label_current_line;
    QPlainTextEdit *listwidget_token;

    QString filename;
    QTextStream *ins;
    Lex *lex;

    QThread *worker_thread;

public slots:
    void token_get(Token *token,int flag=0);
    void ll1_parse();

    bool eventFilter(QObject *watched, QEvent *event) override;
public:
    void loadFile(const QString &fileName);
//修改字体设置相关函数
protected slots:
    void slotFont( QString f );
    void slotSize( QString num );
    void slotBold();
    void slotItalic();
    void slotUnder();
    void slotColor();
    void slotCurrentFormatChanged( const QTextCharFormat & fmt );
    void mergeFormat( QTextCharFormat fmt );
protected:
    void closeEvent(QCloseEvent *event) override;

//主页面相关函数
private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void aboutSNL();
    void documentWasModified();

//主页面相关函数
private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
public:
    CodeEditor *textEdit;
    QPlainTextEdit * errout;
    QPlainTextEdit * resultEdit;
    QString curFile;
    QLabel * pLabel1;
    QLabel * pLabel2;
    QFontComboBox * pFontBox;
    QComboBox * pSizeBox;
    QToolButton * pBoldBtn;
    QToolButton * pItalicBtn;
    QToolButton * pUnderBtn;
    QToolButton * pColorBtn;
    QAction * pRedoAction;
    QAction * pUndoAction;
public:
    void ErrOut();
    bool DoneLex;
#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif
};

#endif // MAINWINDOW_H
