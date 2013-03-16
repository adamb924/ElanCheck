#ifndef WHITESPACEHIGHLIGHTER_H
#define WHITESPACEHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class WhitespaceHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit WhitespaceHighlighter(QObject * parent = 0);

    void highlightBlock(const QString & text);
};

#endif // WHITESPACEHIGHLIGHTER_H
