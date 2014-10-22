#ifndef HELPS_H
#define HELPS_H

#include <QObject>

class Helps : public QObject
{
    Q_OBJECT

private:
    bool m_closed;

public:
    Helps();

    bool getClosed() const;

public slots:
    void closed();
};

#endif // HELPS_H
