#include "sb_coms/protocol/AudioPacketCodec.hpp"

#include <QDataStream>
#include <QIODevice>
#include <QString>

#include <cstring>

namespace sb_coms::protocol {

namespace {
constexpr std::uint32_t Magic = 0x53424331; // "SBC1"
constexpr std::uint8_t Version = 1;
} // namespace

QByteArray AudioPacketCodec::encode(const AudioPacket& packet)
{
    QByteArray datagram;
    QDataStream stream(&datagram, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << Magic;
    stream << Version;
    stream << static_cast<std::uint8_t>(packet.type);
    stream << packet.streamId;
    stream << packet.sequence;
    stream << packet.timestampMs;
    stream << QString::fromStdString(packet.channel);
    stream << QString::fromStdString(packet.senderName);
    stream << static_cast<std::uint32_t>(packet.participants.size());
    for (const auto& participant : packet.participants) {
        stream << QString::fromStdString(participant);
    }
    stream << static_cast<std::uint32_t>(packet.activeSpeakers.size());
    for (const auto& activeSpeaker : packet.activeSpeakers) {
        stream << QString::fromStdString(activeSpeaker);
    }
    stream << static_cast<std::uint32_t>(packet.payload.size());

    if (!packet.payload.empty()) {
        datagram.append(reinterpret_cast<const char*>(packet.payload.data()), static_cast<qsizetype>(packet.payload.size()));
    }

    return datagram;
}

bool AudioPacketCodec::decode(const QByteArray& datagram, AudioPacket& packet)
{
    constexpr qsizetype FixedHeaderBytes = 4 + 1 + 1 + 4 + 4 + 8;
    if (datagram.size() < FixedHeaderBytes) {
        return false;
    }

    QDataStream stream(datagram);
    stream.setByteOrder(QDataStream::BigEndian);

    std::uint32_t magic = 0;
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::uint32_t payloadSize = 0;
    QString channel;

    stream >> magic;
    stream >> version;
    stream >> type;
    stream >> packet.streamId;
    stream >> packet.sequence;
    stream >> packet.timestampMs;
    stream >> channel;

    if (magic != Magic || version != Version) {
        return false;
    }

    if (type != static_cast<std::uint8_t>(PacketType::Hello)
        && type != static_cast<std::uint8_t>(PacketType::Audio)
        && type != static_cast<std::uint8_t>(PacketType::Presence)
        && type != static_cast<std::uint8_t>(PacketType::Heartbeat)
        && type != static_cast<std::uint8_t>(PacketType::Leave)) {
        return false;
    }

    QString senderName;
    std::uint32_t participantCount = 0;
    std::uint32_t activeSpeakerCount = 0;
    stream >> senderName;
    stream >> participantCount;

    if (participantCount > 256) {
        return false;
    }

    packet.participants.clear();
    packet.participants.reserve(participantCount);
    for (std::uint32_t index = 0; index < participantCount; ++index) {
        QString participant;
        stream >> participant;
        packet.participants.push_back(participant.toStdString());
    }

    stream >> activeSpeakerCount;
    if (activeSpeakerCount > 256) {
        return false;
    }

    packet.activeSpeakers.clear();
    packet.activeSpeakers.reserve(activeSpeakerCount);
    for (std::uint32_t index = 0; index < activeSpeakerCount; ++index) {
        QString activeSpeaker;
        stream >> activeSpeaker;
        packet.activeSpeakers.push_back(activeSpeaker.toStdString());
    }

    stream >> payloadSize;

    const qsizetype payloadOffset = stream.device()->pos();
    if (payloadOffset < FixedHeaderBytes || datagram.size() != payloadOffset + static_cast<qsizetype>(payloadSize)) {
        return false;
    }

    packet.type = static_cast<PacketType>(type);
    packet.channel = channel.toStdString();
    packet.senderName = senderName.toStdString();
    packet.payload.resize(payloadSize);

    if (payloadSize > 0) {
        std::memcpy(packet.payload.data(), datagram.constData() + payloadOffset, payloadSize);
    }

    return true;
}

} // namespace sb_coms::protocol
