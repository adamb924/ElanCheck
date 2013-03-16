#include "whitespacehighlighter.h"

#include <QtDebug>

WhitespaceHighlighter::WhitespaceHighlighter(QObject * parent) : QSyntaxHighlighter(parent)
{
}

void WhitespaceHighlighter::highlightBlock(const QString & text)
{
    QTextCharFormat highlightedFormat;
    highlightedFormat.setBackground(QColor( Qt::yellow ).lighter(150));
    QString pattern = "\\s+";

    QRegExp expression(pattern);
    int index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, highlightedFormat);
        index = text.indexOf(expression, index + length);
    }
}
