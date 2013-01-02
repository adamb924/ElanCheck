#include "mainwindow.h"

#include <QtGui>
#include <QDomDocument>
#include <QtDebug>
#include <QtMultimedia>

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    // make sure some pointers are null
    mAudioOutput = 0;
    mTierGroup = 0;

    setupUi();
    setupMenu();
    updateStyleSheet();

    if( argc == 2 )
    {
        mEafFile.readEaf(QString(argv[1]));
        setUiElementsFromEafFile();
    }
}

MainWindow::~MainWindow()
{
    if( mAudioOutput != 0 )
    {
        mAudioOutput->stop();
        delete mAudioOutput;
    }
}

void MainWindow::setupAudioOutput()
{
    QAudioFormat format = mEafFile.sound()->getAudioFormat();
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
        return;
    }

    mAudioOutput = new QAudioOutput(format,this);
}

void MainWindow::setupMenu()
{
    QMenu *file = new QMenu(tr("File"));
    file->addAction(tr("Open"),this,SLOT(open()),QKeySequence("Ctrl+O"));
    file->addAction(tr("Save"),this,SLOT(save()),QKeySequence("Ctrl+S"));
    file->addSeparator();
    file->addAction(tr("Exit"),this,SLOT(close()),QKeySequence("Ctrl+Q"));

    QMenu *goTo = new QMenu(tr("Go to"));
    goTo->addAction(tr("First"),this,SLOT(first()),QKeySequence("Ctrl+F"));
    goTo->addAction(tr("Last"),this,SLOT(last()),QKeySequence("Ctrl+L"));
    goTo->addAction(tr("Go to..."),this,SLOT(goTo()),QKeySequence("Ctrl+G"));

    QMenu *options = new QMenu(tr("Options"));


    mFlag = new QAction(tr("Flag"),options);
    mFlag->setShortcut(QKeySequence("F1"));
    mFlag->setCheckable(true);
    connect(mFlag,SIGNAL(toggled(bool)),this,SLOT(flagAnnotation(bool)));
    connect(mFlag,SIGNAL(toggled(bool)),this,SLOT(updateStyleSheet()));
    options->addAction(mFlag);
    options->addSeparator();

    options->addAction(tr("Font..."),this,SLOT(changeFont()));
    options->addSeparator();


    // Flag submenu
    QMenu *flagSubmenu = new QMenu(tr("Flags"));
    mFlagGroup = new QActionGroup(flagSubmenu);

    mShowAll = new QAction(tr("Show all"),flagSubmenu);
    mShowAll->setCheckable(true);
    mShowAll->setData( Eaf::ShowAll );
    flagSubmenu->addAction(mShowAll);
    mFlagGroup->addAction(mShowAll);

    mShowFlagged = new QAction(tr("Show flagged"),flagSubmenu);
    mShowFlagged->setCheckable(true);
    mShowFlagged->setData( Eaf::ShowFlagged);
    flagSubmenu->addAction(mShowFlagged);
    mFlagGroup->addAction(mShowFlagged);

    connect(mFlagGroup,SIGNAL(selected(QAction*)),this,SLOT(selectFlagBehavior(QAction*)));

    mShowAll->setChecked(true);

    // Path submenu
    QMenu *pathSubmenu = new QMenu(tr("Path"));
    mPathGroup = new QActionGroup(pathSubmenu);

    QAction *aTryRelativeThenAbsolute = new QAction(tr("First try relative, then absolute"),pathSubmenu);
    aTryRelativeThenAbsolute->setCheckable(true);
    aTryRelativeThenAbsolute->setData( Eaf::TryRelativeThenAbsolute );
    pathSubmenu->addAction(aTryRelativeThenAbsolute);
    mPathGroup->addAction(aTryRelativeThenAbsolute);

    QAction *aTryAbsoluteThenRelative = new QAction(tr("First try absolute, then relative"),pathSubmenu);
    aTryAbsoluteThenRelative->setCheckable(true);
    aTryAbsoluteThenRelative->setData( Eaf::TryAbsoluteThenRelative );
    pathSubmenu->addAction(aTryAbsoluteThenRelative);
    mPathGroup->addAction(aTryAbsoluteThenRelative);

    QAction *aOnlyUseRelative = new QAction(tr("Only use relative path"),pathSubmenu);
    aOnlyUseRelative->setCheckable(true);
    aOnlyUseRelative->setData( Eaf::OnlyUseRelative );
    pathSubmenu->addAction(aOnlyUseRelative);
    mPathGroup->addAction(aOnlyUseRelative);

    QAction *aOnlyUseAbsolute = new QAction(tr("Only use absolute path"),pathSubmenu);
    aOnlyUseAbsolute->setCheckable(true);
    aOnlyUseAbsolute->setData( Eaf::OnlyUseAbsolute );
    pathSubmenu->addAction(aOnlyUseAbsolute);
    mPathGroup->addAction(aOnlyUseAbsolute);

    aTryRelativeThenAbsolute->setChecked(true);

    options->addMenu(flagSubmenu);
    options->addMenu(pathSubmenu);

    mTierMenu = new QMenu(tr("Tiers"));
    options->addMenu(mTierMenu);

    QMenuBar *menubar = menuBar();
    menubar->addMenu(file);
    menubar->addMenu(goTo);
    menubar->addMenu(options);
}

void MainWindow::setupUi()
{
    mFilename = new QLabel(tr("No file selected"));
    mPosition = new QLabel(tr("--"));
    mNext = new QPushButton(tr("Next"));
    mPrevious = new QPushButton(tr("Previous"));
    mPlay = new QPushButton(tr("Play"));

    mAnnotation = new QTextEdit("");
    mAnnotation->setAcceptRichText(false);

    connect(mNext,SIGNAL(clicked()),this,SLOT(nextAnnotation()));
    connect(mPrevious,SIGNAL(clicked()),this,SLOT(previousAnnotation()));
    connect(mPlay,SIGNAL(clicked()),this,SLOT(play()));

    QHBoxLayout* infolayout = new QHBoxLayout;
    infolayout->addWidget(mFilename);
    infolayout->addStretch(1);
    infolayout->addWidget(mPosition);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(mPrevious);
    hlayout->addStretch(1);
    hlayout->addWidget(mPlay);
    hlayout->addStretch(1);
    hlayout->addWidget(mNext);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(infolayout);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(mAnnotation);

    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(vlayout);
    this->setCentralWidget(centralWidget);

    // not great
    mFontFamily = "Times New Roman";
    mFontPointSize = 28;

    // initially disable these
    mNext->setEnabled(false);
    mPrevious->setEnabled(false);
    mPlay->setEnabled(false);
    mAnnotation->setEnabled(false);
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open"),QString(),"*.eaf");
    if( filename.isEmpty() )
        return;

    Eaf::PathBehavior pathBehavior = (Eaf::PathBehavior)mPathGroup->checkedAction()->data().toInt();

    if( !mEafFile.readEaf(filename, pathBehavior) )
        QMessageBox::critical(this,tr("Error reading file"),tr("There was an error reading %1, so the file has not been opened.").arg(filename));
    else
        setUiElementsFromEafFile();
}

void MainWindow::setUiElementsFromEafFile()
{
    if( mEafFile.hasFlags() )
        mShowFlagged->setChecked(true);
    else
        mShowAll->setChecked(true);

    setupAudioOutput();

    addTierActions();

    QFileInfo info(mEafFile.filename());
    mFilename->setText( info.baseName() );

    setUiElementsForTier();
}

void MainWindow::setUiElementsForTier()
{
    if( mEafFile.annotations()->count() > 0 )
    {
        mPlay->setEnabled(true);
        mAnnotation->setEnabled(true);
        setAnnotation( mEafFile.getFirstAnnotation(getFlagBehavior()) );
    }
    else
    {
        mPlay->setEnabled(false);
        mAnnotation->setEnabled(false);
    }
}

void MainWindow::save()
{
    if( mEafFile.filename().isEmpty() )
        return;

    saveCurrentAnnotation();

    mEafFile.sendDataToDOM(mCurrentTierId);

    QString xml = mEafFile.document()->toString(4);
    QFile outfile(mEafFile.filename());
    if (!outfile.open(QIODevice::WriteOnly))
        return;
    outfile.write(xml.toUtf8());
    outfile.close();

    mEafFile.setFileChanged(false);
}

Eaf::FlagBehavior MainWindow::getFlagBehavior() const
{
    return (Eaf::FlagBehavior)mFlagGroup->checkedAction()->data().toInt();
}


void MainWindow::nextAnnotation()
{
    saveCurrentAnnotation();
    setAnnotation( mEafFile.getNextAnnotation(mCurrentAnnotation, getFlagBehavior() ) );
}

void MainWindow::previousAnnotation()
{
    saveCurrentAnnotation();
    setAnnotation( mEafFile.getPreviousAnnotation(mCurrentAnnotation, getFlagBehavior() ) );
}

void MainWindow::setAnnotation(int i)
{
    if( i == -1 || mEafFile.annotations()->count() == 0 )
        return;

    if( mAudioOutput != 0 )
        mAudioOutput->stop();

    mCurrentAnnotation = i;

    Eaf::FlagBehavior flagBehavior = getFlagBehavior();

    if( mEafFile.isFirstAnnotation(mCurrentAnnotation, flagBehavior ) )
        mPrevious->setEnabled(false);
    else
        mPrevious->setEnabled(true);

    if( mEafFile.isLastAnnotation(mCurrentAnnotation, flagBehavior ) )
        mNext->setEnabled(false);
    else
        mNext->setEnabled(true);

    mFlag->setChecked(mEafFile.annotations()->at(mCurrentAnnotation).isFlagged() );

    mAnnotation->setText( mEafFile.annotations()->at(mCurrentAnnotation).mValue );
    if( flagBehavior == Eaf::ShowAll )
        mPosition->setText( tr("%1/%2").arg(mCurrentAnnotation+1).arg(mEafFile.annotations()->count()));
    else if( flagBehavior == Eaf::ShowFlagged )
        mPosition->setText( tr("%1/%2 (%3/%4)").arg(mCurrentAnnotation+1).arg(mEafFile.annotations()->count()).arg(mEafFile.flagPosition(mCurrentAnnotation)+1).arg(mEafFile.nFlags()) );
}

void MainWindow::saveCurrentAnnotation()
{
    if( mEafFile.annotations()->count() == 0 )
        return;
    if( (*mEafFile.annotations())[mCurrentAnnotation].mValue != mAnnotation->toPlainText() )
    {
        (*mEafFile.annotations())[mCurrentAnnotation].mValue = mAnnotation->toPlainText();
        mEafFile.setFileChanged(true);
    }
    if( (*mEafFile.annotations())[mCurrentAnnotation].isFlagged() != mFlag->isChecked() )
    {
        (*mEafFile.annotations())[mCurrentAnnotation].setFlag(mFlag->isChecked());
        mEafFile.setFileChanged(true);
    }
}

void MainWindow::play()
{
    if( mAudioOutput != 0 )
        mAudioOutput->stop();
    if( mEafFile.annotations()->count() == 0 )
        return;

    QBuffer audio_buffer( &( (*mEafFile.annotations())[mCurrentAnnotation].mAudioData ) );
    audio_buffer.open(QIODevice::ReadOnly);

    if( (*mEafFile.annotations())[mCurrentAnnotation].mAudioData.length() < 1 )
    {
        qDebug() << "Zero-length audio";
        return;
    }

    mAudioOutput->start(&audio_buffer);

    QEventLoop loop;
    QObject::connect(mAudioOutput, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
    do {
        loop.exec();
    } while(mAudioOutput->state() == QAudio::ActiveState);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if( mAudioOutput != 0 )
        mAudioOutput->stop();

    saveCurrentAnnotation();

    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave()
{
    if( mEafFile.filename().length() == 0 )
        return true;
    if( !mEafFile.fileChanged() )
        return true;

    QMessageBox::StandardButton response;
    response = QMessageBox::question( this, tr("Save before closing?"), tr("Do you wish to save your work before closing?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel , QMessageBox::Yes );
    if( response == QMessageBox::Cancel )
        return false;
    if ( response == QMessageBox::Yes )
        save();
    return true;
}

void MainWindow::changeFont()
{
    QFont currentFont(mFontFamily,mFontPointSize);
    bool isOk;
    QFont newFont = QFontDialog::getFont(&isOk,currentFont,this);
    if( isOk )
    {
        mFontFamily = newFont.family();
        mFontPointSize = newFont.pointSize();
        mAnnotation->setStyleSheet(QString("font: %1pt \"%2\";").arg(mFontPointSize).arg(mFontFamily));
    }
}

void MainWindow::selectTier(QAction* action)
{
    saveCurrentAnnotation();
    mEafFile.sendDataToDOM(mCurrentTierId);
    mCurrentTierId = action->text();
    mEafFile.readAnnotations(action->text());
    setUiElementsForTier();
}

void MainWindow::selectFlagBehavior(QAction* action)
{
    setAnnotation( mEafFile.getFirstAnnotation(getFlagBehavior()) );
}

void MainWindow::addTierActions()
{
    mTierMenu->clear();

    if( mTierGroup != 0 )
        delete mTierGroup;

    mTierGroup = new QActionGroup(mTierMenu);
    connect(mTierGroup,SIGNAL(selected(QAction*)),this,SLOT(selectTier(QAction*)));

    for(int i=0; i<mEafFile.tiers()->count(); i++)
    {
        QAction *action = new QAction(mEafFile.tiers()->at(i),mTierMenu);
        action->setCheckable(true);
        mTierMenu->addAction(action);
        mTierGroup->addAction(action);
    }
    if( mEafFile.tiers()->count() > 0 )
    {
        mCurrentTierId = mTierGroup->actions().at(0)->text();
        mTierGroup->actions().at(0)->setChecked(true);
    }
}

void MainWindow::first()
{
    saveCurrentAnnotation();
    setAnnotation( mEafFile.getFirstAnnotation(getFlagBehavior()) );
}

void MainWindow::last()
{
    saveCurrentAnnotation();
    setAnnotation( mEafFile.getLastAnnotation(getFlagBehavior()) );
}

void MainWindow::goTo()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("Elan Check"),
                                 tr("Which?"), 1, 1, mEafFile.annotations()->count(), 1, &ok);
    if (ok)
    {
        saveCurrentAnnotation();
        setAnnotation( i-1 );
    }
}

void MainWindow::flagAnnotation(bool flag)
{
    (*mEafFile.annotations())[mCurrentAnnotation].setFlag(flag);
    mEafFile.setFileChanged(true);
}

void MainWindow::updateStyleSheet()
{
    QString styleSheet = QString("font: %1pt \"%2\";").arg(mFontPointSize).arg(mFontFamily);
    if( mAnnotation->isEnabled() )
        styleSheet +=  "background-color: " + (mFlag->isChecked() ? QString("#FFFF99") : QString("#FFFFFF")) + ";";
    mAnnotation->setStyleSheet(styleSheet);
}
