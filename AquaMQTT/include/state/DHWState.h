#ifndef AQUAMQTT_DHWSTATE_H
#define AQUAMQTT_DHWSTATE_H

#include <Arduino.h>
#include <RingBuf.h>

#include <map>

#include "message/MessageConstants.h"

namespace aquamqtt
{

struct BufferStatistics
{
    uint64_t msgHandled;
    uint64_t msgUnhandled;
    uint64_t msgCRCFail;
    uint64_t droppedBytes;
    uint64_t msgSent;
};

/**
 * DHWState contains the original state originated by the original DHW controllers
 * FrameBuffers are writing complete frames to the DHWState storing the last known messages
 * In case a message has changed, it will trigger the mqtt task to publish updates to the broker.
 */
class DHWState
{
public:
    static DHWState& getInstance();

    virtual ~DHWState() = default;

    DHWState(const DHWState&) = delete;

private:
    DHWState();

public:
    DHWState& operator=(const DHWState&) = delete;

    message::ProtocolVersion getVersion() const;

    void setVersion(message::ProtocolVersion version);

    void setChecksumType(message::ProtocolChecksum checksum);

    void setListener(TaskHandle_t handle);

    void storeFrame(uint8_t frameId, uint8_t payloadLength, uint8_t* payload);

    void updateFrameBufferStatistics(message::FrameBufferChannel source, BufferStatistics statistics);

    BufferStatistics getFrameBufferStatistics(message::FrameBufferChannel source);

    size_t copyFrame(
            uint8_t                    frameId,
            uint8_t*                   buffer,
            message::ProtocolVersion&  version,
            message::ProtocolChecksum& type);

    void debugSaveFrameTiming(uint8_t fromFrameId, uint8_t toFrameId, unsigned long millis);

    std::map<uint8_t, std::map<uint8_t, unsigned long>> debugGetFrameTiming() const;

    void debugAddToDropBuffer(message::FrameBufferChannel source, int val);

    size_t debugTakeDropBuffer(message::FrameBufferChannel source, uint8_t* buffer);

private:
    TaskHandle_t mNotify;

    SemaphoreHandle_t mMutex;

    message::ProtocolVersion  mProtocolVersion;
    message::ProtocolChecksum mChecksumType;

    bool mHasHmiMessage;
    bool mHasMainMessage;
    bool mHasEnergyMessage;
    bool mHasErrorMessage;
    bool mHasExtraMessage;

    uint8_t mMessageHmi[message::HEATPUMP_MAX_FRAME_LENGTH];
    uint8_t mMessageMain[message::HEATPUMP_MAX_FRAME_LENGTH];
    uint8_t mMessageEnergy[message::HEATPUMP_MAX_FRAME_LENGTH];
    uint8_t mMessageError[message::HEATPUMP_MAX_FRAME_LENGTH];
    uint8_t mMessageExtra[message::HEATPUMP_MAX_FRAME_LENGTH];

    BufferStatistics mHmiStats;
    BufferStatistics mMainStats;
    BufferStatistics mListenerStats;

    std::map<uint8_t, std::map<uint8_t, unsigned long>> mFrameTiming;
    RingBuf<int, message::HEATPUMP_MAX_FRAME_LENGTH>    mDroppedBufferListener;
    RingBuf<int, message::HEATPUMP_MAX_FRAME_LENGTH>    mDroppedBufferHmi;
    RingBuf<int, message::HEATPUMP_MAX_FRAME_LENGTH>    mDroppedBufferController;
};

}  // namespace aquamqtt

#endif  // AQUAMQTT_DHWSTATE_H
