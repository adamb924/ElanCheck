#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class QAudioOutput;

#include "eaf.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[], QWidget *parent = 0);
    ~MainWindow();


private:
    // GUI Setup
    void setupMenu();
    void setupAudioOutput();
    void setupUi();

    void addTierActions();

    // Data
    Eaf mEafFile;

    // Audio output
    QAudioOutput *mAudioOutput;

    // GUI interaction
    void closeEvent(QCloseEvent *event);
    bool maybeSave();

    void setAnnotation(int i);
    void saveCurrentAnnotation();

    void setUiElementsFromEafFile();
    void setUiElementsForTier();


    int mCurrentAnnotation;
    QString mFontFamily;
    int mFontPointSize;

    QString mCurrentTierId;

    // GUI Members
    QActionGroup *mTierGroup, *mPathGroup;
    QLabel *mFilename, *mPosition;
    QTextEdit* mAnnotation;
    QPushButton *mNext, *mPrevious, *mPlay;
    QMenu *mTierMenu;
    QAction *mOpen, *mSave, *mExit;

private slots:
    void open();
    void save();
    void play();
    void nextAnnotation();
    void previousAnnotation();
    void changeFont();
    void selectTier(QAction* action);
};

#endif // MAINWINDOW_H
