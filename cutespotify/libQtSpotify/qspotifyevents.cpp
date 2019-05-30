#include "qspotifyevents.h"

const QEvent::Type NotifyMainThreadEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User));
const QEvent::Type ConnectionErrorEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 1));
const QEvent::Type MetaDataEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 2));
const QEvent::Type StreamingStartedEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 3));
const QEvent::Type EndOfTrackEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 4));
const QEvent::Type StopEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 5));
const QEvent::Type ResumeEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 6));
const QEvent::Type SuspendEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 7));
const QEvent::Type AudioStopEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 8));
const QEvent::Type ResetBufferEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 9));
const QEvent::Type TrackProgressEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 10));
const QEvent::Type SendImageRequestEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 11));
const QEvent::Type ReceiveImageRequestEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 12));
const QEvent::Type PlayTokenLostEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 13));
const QEvent::Type LoggedInEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 14));
const QEvent::Type LoggedOutEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 15));
const QEvent::Type OfflineErrorEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 16));
const QEvent::Type ScrobbleLoginErrorEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 17));
const QEvent::Type ConnectionStateUpdateEventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 18));
