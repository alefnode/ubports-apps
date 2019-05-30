#include "qspotifyringbuffer.h"

#include <cstring>
#include <algorithm>

QSpotifyRingbuffer::QSpotifyRingbuffer() :
    m_readPos{}, m_writePos{}, m_size{}, m_isOpen{}
{
    m_data = new char[BUF_SIZE];
    memset(m_data, 0, BUF_SIZE);
}

QSpotifyRingbuffer::~QSpotifyRingbuffer()
{
    if(m_data)
        delete[] m_data;
}

void QSpotifyRingbuffer::close()
{
    memset(m_data, 0, BUF_SIZE);
    m_readPos = 0;
    m_writePos = 0;
    m_size = 0;
    m_isOpen = false;
}

void QSpotifyRingbuffer::open()
{
    memset(m_data, 0, BUF_SIZE);
    m_isOpen = true;
}

int QSpotifyRingbuffer::read(char *data, int numBytes)
{
    if(!m_data) return 0;

    numBytes = std::min(numBytes, m_size);
    if(numBytes > 0) {
        if(m_readPos + numBytes >= BUF_SIZE) {
            int firstBytes = BUF_SIZE - m_readPos;
            memcpy(&data[0], &m_data[m_readPos], firstBytes);
            memcpy(&data[firstBytes], &m_data[0], numBytes - firstBytes);
        } else {
            memcpy(&data[0], &m_data[m_readPos], numBytes);
        }
        m_readPos = (m_readPos + numBytes) % BUF_SIZE;
        m_size -= numBytes;
    }
    return numBytes;
}

int QSpotifyRingbuffer::write(const char *data, int numBytes)
{
    if(!m_data) return 0;

    numBytes = std::min(numBytes, BUF_SIZE - m_size);
    if(m_writePos + numBytes >= BUF_SIZE) {
        int firstBytes = BUF_SIZE - m_writePos;
        memcpy(&m_data[m_writePos], &data[0], firstBytes);
        memcpy(&m_data[0], &data[firstBytes], numBytes - firstBytes);
    } else {
        memcpy(&m_data[m_writePos], &data[0], numBytes);
    }
    m_writePos = (m_writePos + numBytes) % BUF_SIZE;
    m_size += numBytes;
    return numBytes;
}

int QSpotifyRingbuffer::freeBytes()
{
    return BUF_SIZE - m_size;
}
