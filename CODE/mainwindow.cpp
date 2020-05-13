#include "ll1.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include <QtWidgets>
#include <QFileDialog>
#include <QGraphicsWidget>
#include <QGroupBox>
#include <QtWidgets/QWidgetAction>
#include <QtGui/QSyntaxHighlighter>
#include <QGraphicsView>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSaveFile>
#include <QSettings>
#include <QColorDialog>
#include <string>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    DoneLex=false;
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    init_ui();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == pview) {
        if (event->type() == QEvent::Wheel) {
            auto e = static_cast<QWheelEvent *>(event);
            if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
                auto delt = e->angleDelta();

                double m_zoom_delta = 0;
                if (delt.y() > 0) {
                    m_zoom_delta = 0.1;
                } else {
                    m_zoom_delta = -0.1;
                }
                pview->scale(1 + m_zoom_delta, 1 + m_zoom_delta);
            }
            return QObject::eventFilter(watched, event);

        } else {
            return QObject::eventFilter(watched, event);
        }
    } else
        return QObject::eventFilter(watched, event);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init_ui() {

    auto leftsplitter = new QSplitter(Qt::Horizontal);

    textEdit = new CodeEditor(leftsplitter);
    textEdit->setMaximumSize(500,1080);

    rightsplitter = new QSplitter(Qt::Vertical, leftsplitter);
    listwidget_token = new QPlainTextEdit(rightsplitter);
    listwidget_token->setMaximumSize(500,1080);
    listwidget_token->setReadOnly(1);

    auto topsplitter = new QSplitter(Qt::Vertical,leftsplitter);


    tab_center_widget = new QTabWidget(topsplitter);
    parseScene = new ParseScene();
    pview = new QGraphicsView(tab_center_widget);
    pview->installEventFilter(this);
    pview->setScene(parseScene);
    pview->setDragMode(QGraphicsView::ScrollHandDrag);
    tab_center_widget->addTab(pview, QString(tr("语法树")));

    errout = new  QPlainTextEdit(topsplitter);
    errout->setMaximumSize(1000,200);
    errout->setReadOnly(1);


    this->setCentralWidget(leftsplitter);

    QMenu *fileMenu = menuBar()->addMenu(tr("文件"));
    QToolBar *fileToolBar = addToolBar(tr("文件"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("新建"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("新建文件"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("打开..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("打开文件"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("保存"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("保存文件"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as", QIcon(":/images/saveas.png"));
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("另存为..."), this, &MainWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("文件另存为"));
    fileToolBar->addAction(saveAsAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit", QIcon(":/images/exit.png"));
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("退出"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("退出应用"));

    QMenu *editMenu = menuBar()->addMenu(tr("编辑"));
    QToolBar *editToolBar = addToolBar(tr("编辑"));
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("剪切"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("剪切文本"));
    connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);


    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("复制"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("复制文本"));
    connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("粘贴"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("粘贴文本"));
    connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);


    pUndoAction = new QAction( QIcon( ":/images/undo.png" ), tr( "撤销" ), this );
    editToolBar->addAction( pUndoAction );
    connect( pUndoAction, SIGNAL( triggered() ), textEdit, SLOT( undo() ) );

    pRedoAction = new QAction( QIcon( ":/images/redo.png" ), tr( "恢复" ), this );
    editToolBar->addAction( pRedoAction );
    connect( pRedoAction, SIGNAL( triggered() ), textEdit, SLOT( redo() ) );

    menuBar()->addSeparator();

    QMenu *buildMenu = menuBar()->addMenu(tr("词法分析"));
    QToolBar *buildToolBar = addToolBar(tr("词法分析"));

    const QIcon buildIcon = QIcon::fromTheme("build", QIcon(":/images/build.png"));
    QAction *buildAct = buildMenu->addAction(buildIcon, tr("词法分析"));
    connect(buildAct, &QAction::triggered, [this]() {
        listwidget_token->clear();
        lex = Lex::getInstance(textEdit->document());
        connect(lex, &Lex::token_get, this, &MainWindow::token_get, Qt::ConnectionType::UniqueConnection);
        lex->start();
    });
    buildAct->setStatusTip(tr("对词法进行分析，生成Token序列"));
    buildToolBar->addAction(buildAct);

    QMenu *runMenu = menuBar()->addMenu(tr("语法分析"));
    QToolBar *runToolBar = addToolBar(tr("语法分析"));

    const QIcon runll1Icon = QIcon::fromTheme("run", QIcon(":/images/run.png"));
    QAction *runll1Act = runMenu->addAction(runll1Icon, tr("LL1"), this,&MainWindow::ll1_parse);
    runll1Act->setStatusTip(tr("对语法进行分析，生成语法树"));
    runToolBar->addAction(runll1Act);
    ins = new QTextStream();


    QMenu *helpMenu = menuBar()->addMenu(tr("说明"));
    const QIcon aboutIcon = QIcon::fromTheme("about", QIcon(":/images/about.png"));
    QAction *aboutUsAct = helpMenu->addAction(aboutIcon, tr("关于"), this, &MainWindow::about);
    aboutUsAct->setStatusTip(tr("关于次项目"));

    const QIcon aboutSNLIcon = QIcon::fromTheme("about", QIcon(":/images/aboutsnl.png"));
    QAction *aboutSNLAct = helpMenu->addAction(aboutSNLIcon, tr("SNL"), this, &MainWindow::aboutSNL);
    aboutSNLAct->setStatusTip(tr("关于SNL"));


    QToolBar * pToolBar = addToolBar(tr("字体") );

    pLabel1 = new QLabel(tr( "字体:" ) );
    pFontBox = new QFontComboBox();
    pFontBox->setFontFilters( QFontComboBox::ScalableFonts );
    pToolBar->addWidget( pLabel1 );
    pToolBar->addWidget( pFontBox );
    connect( pFontBox, SIGNAL(activated(QString)), this, SLOT(slotFont( QString)));

    pLabel2 = new QLabel( tr( "字号:" ) );
    pSizeBox = new QComboBox();
    pToolBar->addWidget( pLabel2 );
    pToolBar->addWidget( pSizeBox );
    connect( pSizeBox, SIGNAL(activated(QString)), this, SLOT(slotSize( QString)));

    QFontDatabase db;

    foreach( int nSize, db.standardSizes() )
    pSizeBox->addItem( QString::number( nSize ) );
    pToolBar->addSeparator();

    pBoldBtn = new QToolButton();
    pBoldBtn->setIcon( QIcon( ":/images/bold.png") );
    pBoldBtn->setCheckable( true );
    pToolBar->addWidget( pBoldBtn );
    connect( pBoldBtn, SIGNAL(clicked() ), this, SLOT( slotBold()));

    pItalicBtn = new QToolButton();
    pItalicBtn->setIcon( QIcon( ":/images/italic.png" ) );
    pItalicBtn->setCheckable( true );
    pToolBar->addWidget( pItalicBtn );
    connect( pItalicBtn, SIGNAL(clicked()), this, SLOT( slotItalic()));

    pUnderBtn = new QToolButton();
    pUnderBtn->setIcon( QIcon( ":/images/underline.png" ) );
    pUnderBtn->setCheckable( true );
    pToolBar->addWidget( pUnderBtn );
    connect( pUnderBtn, SIGNAL(clicked()), this, SLOT( slotUnder()));

    pToolBar->addSeparator();
    pColorBtn = new QToolButton();
    pColorBtn->setIcon( QIcon( ":/images/color.png" ) );
    pToolBar->addWidget( pColorBtn );
    connect( pColorBtn, SIGNAL(clicked()), this, SLOT( slotColor()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
}

void MainWindow::token_get(Token *token,int flag) {
    try {
        if (flag==0)
        {
            if (token->getSem().length()>5)
            {
                QString str("Line:%1\tSem:%2\tLex:%3");
                QString item = str.arg(token->getLine()).arg(token->getSem()).arg(token->getLexName());
                listwidget_token->appendPlainText(item);
            }
            else
            {
                QString str("Line:%1\tSem:%2\t\tLex:%3");
                QString item = str.arg(token->getLine()).arg(token->getSem()).arg(token->getLexName());
                listwidget_token->appendPlainText(item);
            }
            DoneLex=true;
        }
        else
        {
            QTextCharFormat fmt;
            QColor Redcolor (255,0,0);
            QColor Blackcolor (0,0,0);
            fmt.setForeground(Redcolor);
            listwidget_token->mergeCurrentCharFormat(fmt);
            QString str("Line:%1\t存在错误:\t\t%2");
            QString item = str.arg(token->getLine()).arg(token->getSem());
            listwidget_token->appendPlainText(item);
            fmt.setForeground(Blackcolor);
            listwidget_token->mergeCurrentCharFormat(fmt);
        }
    } catch (QException e) {
        QMessageBox::warning(this, tr("警告"), tr("发生错误，请重新尝试！"));
    }

}
void MainWindow::ErrOut()
{
    errout->clear();
    QThread t;
    t.msleep(50);
    QVector<QString>  tmp = LL1_parse::msg;
    for (auto i = tmp.begin();i<tmp.end();i++)
    {
        errout->appendPlainText((*i));
    }
    errout->appendPlainText("词法分析完成！");
}
void MainWindow::ll1_parse() {
    if (!DoneLex)
    {
        QMessageBox::warning(this, tr("警告"), tr("请先进行词法分析"));
        return;
    }
    try {
        auto ll1 = LL1_parse::getInstance(lex->getTokenList());
        connect(ll1, &LL1_parse::parse_success, parseScene, &ParseScene::show_parsetree,
                Qt::ConnectionType::UniqueConnection);
        ll1->start();
        ErrOut();
        DoneLex=false;
    } catch (QException e) {
        QMessageBox::warning(this, tr("警告"), tr("发生错误，请重新尝试！"));
    }

}
void MainWindow::slotFont( QString f )
{
    QTextCharFormat fmt;
    fmt.setFontFamily( f );
    mergeFormat( fmt );
    }

void MainWindow::slotSize( QString num )
{
    QTextCharFormat fmt;
    fmt.setFontPointSize( num.toFloat() );
    mergeFormat( fmt );
}

void MainWindow::slotBold()

{
    QTextCharFormat fmt;
    fmt.setFontWeight( pBoldBtn->isChecked() ? QFont::Bold : QFont::Normal );
    mergeFormat( fmt );
}
void MainWindow::slotItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic( pItalicBtn->isChecked() );
    mergeFormat( fmt );
}
void MainWindow::slotUnder()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline( pUnderBtn->isChecked() );
    mergeFormat( fmt );
}
void MainWindow::slotColor()

{
    QColor color = QColorDialog::getColor( Qt::red, this );
    if ( color.isValid() )
    {
        QTextCharFormat fmt;
        fmt.setForeground( color );
        mergeFormat( fmt );
    }
}
void MainWindow::slotCurrentFormatChanged( const QTextCharFormat & fmt )
{
    pFontBox->setCurrentIndex( pFontBox->findText( fmt.fontFamily() ) );
    pSizeBox->setCurrentIndex( pSizeBox->findText( QString::number( fmt.fontPointSize() ) ) );
    pBoldBtn->setChecked( fmt.font().bold() );
    pItalicBtn->setChecked( fmt.fontItalic() );
    pUnderBtn->setChecked( fmt.fontUnderline() );
}

void MainWindow::mergeFormat( QTextCharFormat fmt )

{
    QTextCursor cursor = textEdit->textCursor();
    if ( !cursor.hasSelection() )
          cursor.select( QTextCursor::WordUnderCursor );
    cursor.mergeCharFormat( fmt );
    textEdit->mergeCurrentCharFormat( fmt );
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}
void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("关于此项目"),
            tr("编译原理作业\n"
               "由徐迎,张洪瑞,崔家华完成"
               ));
}
void MainWindow::aboutSNL()
{
   QMessageBox::about(this, tr("关于SNL"),
            tr("SNL是一个简化的类PASCAL语言，它保留了PASCAL语言的很多特性，因此其语义信息也非常丰富。"
               "在SNL语言中，标识符可以表示变量名，类型名，记录的域名，过程名等；SNL语言的程序包含显式"
               "的声明部分，所有标识符必须先声明再使用；在同一层过程里声明的标识符不允许重名，即标识符不"
               "允许重复声明；标识符的作用域和PASCAL语言的嵌套作用域规定相同。上述是与标识符有关的语义"
               "约定，要求SNL语言的程序员在编程的时候加以遵守，对于违反这些约定的程序，编译器会在语义分"
               "析时，通过构造标识符的符号表和相关信息表进行语义检查，标识出现的错误，以便程序员对程序进"
               "行修改。\n"
               "SNL语言规定，过程声明可以嵌套，即一个过程里还可以声明其他的过程；过程的参数有两类：一类"
               "是值参数，即值引用型；一类是变量参数，即地址引用型参数；过程允许递归调用；一个过程中出现"
               "的变量只能有三种形式，一是形参，二是局部量，三是全程量（即在该过程的外层声明的静态变量）；"
               "过程调用时，参数的个数和类型必须相符；这些规定与PASCAL语言完全相同，同样要在语义分析时进行"
               "检查。\n"
              " SNL语言的基类型只有整型和字符型，但是用户可以自己定义复杂类型，如数组类型和记录类型等，"
               "对于这些构造的类型也有一定的限制：构造的数组类型的基类型只能是整型和字符型；记录类型的域标"
               "识符不能声明为记录类型，即记录类型不允许嵌套定义。另外，SNL语言提供了标准的算术运算加减乘除"
               "，算术表达式的运算分量和运算结果只能是整型；SNL语言没有提供布尔类型，当然也没有提供布尔运算，"
               "只能通过整型值的关系运算表示条件，而且这种关系运算只能出现在条件语句和循环语句的条件表达式中。"
               "上面几条是与类型相关的语义信息，编译器也将在语义分析的时候，通过查找符号表，对标识符的类型和"
               "表达式的类型信息进行语义检查，标记出现的类型错误。"

               ));
}
void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("准备就绪"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}
bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("警告"),
                               tr("文件修改过\n"
                                  "是否保存?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}
void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("打开失败")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("加载成功"), 2000);
}
bool MainWindow::saveFile(const QString &fileName)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << textEdit->toPlainText();
        if (!file.commit()) {
            errorMessage = tr("保存失败")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("保存失败")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("保存成功"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}
#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        if (textEdit->document()->isModified())
            save();
    }
}
#endif
