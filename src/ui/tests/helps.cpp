#include "helps.h"


bool Helps::getClosed() const
{
    return m_closed;
}

Helps::Helps() :
    m_closed(false)
{
}

void Helps::closed()
{
    m_closed = true;
}
