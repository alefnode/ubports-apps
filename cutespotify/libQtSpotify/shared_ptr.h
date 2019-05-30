#ifndef SHARED_PTR_H
#define SHARED_PTR_H

#include <memory>

#include "qspotifytrack.h"

uint qHash(const std::shared_ptr<QSpotifyTrack> &v);

#endif // SHARED_PTR_H
