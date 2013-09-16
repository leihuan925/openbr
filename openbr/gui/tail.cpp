#include "tail.h"

using namespace br;

/**** TAIL ****/
/*** PUBLIC ***/
Tail::Tail(QWidget *parent)
    : QSlider(parent)
{
    setOrientation(Qt::Horizontal);
    setVisible(false);
    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(setIndex(int)));
}

/*** PUBLIC SLOTS ***/
void Tail::setIndex(int index)
{
    index = std::min(std::max(minimum(), index), maximum());
    QSlider::setValue(index);
    emit newTargetFile((index >= 0) && (index < targetFiles.size()) ? targetFiles[index] : File());
    emit newQueryFile((index >= 0) && (index < queryFiles.size()) ? queryFiles[index] : File());
    emit newScore((index >= 0) && (index < scores.size()) ? scores[index] : std::numeric_limits<float>::quiet_NaN());
}

void Tail::setTargetGallery(const File &gallery)
{
    targetGallery = gallery;
    compare();
}

void Tail::setQueryGallery(const File &gallery)
{
    queryGallery = gallery;
    compare();
}

/*** PROTECTED ***/
void Tail::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
    event->accept();
    if      (event->key() == Qt::Key_Up)    first();
    else if (event->key() == Qt::Key_Left)  previous();
    else if (event->key() == Qt::Key_Right) next();
    else if (event->key() == Qt::Key_Down)  last();
}

void Tail::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
    event->accept();
    if (event->delta() < 0) next();
    else                    previous();
}

/*** PRIVATE ***/
void Tail::compare()
{
    targetFiles.clear();
    queryFiles.clear();
    scores.clear();

    if (targetGallery.isNull() || queryGallery.isNull()) {
        if (!targetGallery.isNull()) targetFiles = TemplateList::fromGallery(targetGallery).files();
        if (!queryGallery.isNull()) queryFiles = TemplateList::fromGallery(queryGallery).files();
        setMaximum(std::max(targetFiles.size(), queryFiles.size()) - 1);
    } else {
        Compare(targetGallery.flat(), queryGallery.flat(), "buffer.tail[atMost=5000,threshold=0.8]");
        QStringList lines = QString(Globals->buffer).split('\n');
        lines.takeFirst(); // Remove header

        foreach (const QString &line, lines) {
            const QStringList words = Object::parse(line);
            if (words.size() != 3) qFatal("Invalid tail file.");
            bool ok;
            float score = words[0].toFloat(&ok); assert(ok);
            targetFiles.append(words[1]);
            queryFiles.append(words[2]);
            scores.append(score);
        }
        setMaximum(scores.size()-1);
    }

    setVisible(maximum() > 1);
    setIndex(0);
}

void Tail::first()
{
    setIndex(scores.size()-1);
}

void Tail::previous()
{
    setIndex(value()+1);
}

void Tail::next()
{
    setIndex(value()-1);
}

void Tail::last()
{
    setIndex(0);
}

#include "moc_tail.cpp"