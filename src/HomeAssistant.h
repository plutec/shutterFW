/*
  HomeAssistant.h - A simple controller for HomeAssistant.
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/


#ifndef HomeAssistant_h
#define HomeAssistant_h
#include "PubSubClient.h"
//#include "config.h"


class HomeAssistant {
    private:
        PubSubClient* _mqtt;
        /*Client* _client;
        uint8_t buffer[MQTT_MAX_PACKET_SIZE];
        uint16_t nextMsgId;
        unsigned long lastOutActivity;
        unsigned long lastInActivity;
        bool pingOutstanding;
        MQTT_CALLBACK_SIGNATURE;
        uint32_t readPacket(uint8_t*);
        boolean readByte(uint8_t * result);
        boolean readByte(uint8_t * result, uint16_t * index);
        boolean write(uint8_t header, uint8_t* buf, uint16_t length);
        uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);
        // Build up the header ready to send
        // Returns the size of the header
        // Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
        //       (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the buffer
        size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length);
        IPAddress ip;
        const char* domain;
        uint16_t port;
        Stream* stream;
        int _state;*/
    public:
        HomeAssistant();
        HomeAssistant(PubSubClient* mqtt);
        void SendDiscovery();
        void SendStatus(uint8_t percent, bool relay1, bool relay2);
        /*PubSubClient(Client& client);
        PubSubClient(IPAddress, uint16_t, Client& client);
        PubSubClient(IPAddress, uint16_t, Client& client, Stream&);
        PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
        PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);
        PubSubClient(uint8_t *, uint16_t, Client& client);
        PubSubClient(uint8_t *, uint16_t, Client& client, Stream&);
        PubSubClient(uint8_t *, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
        PubSubClient(uint8_t *, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);
        PubSubClient(const char*, uint16_t, Client& client);
        PubSubClient(const char*, uint16_t, Client& client, Stream&);
        PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
        PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);

        PubSubClient& setServer(IPAddress ip, uint16_t port);
        PubSubClient& setServer(uint8_t * ip, uint16_t port);
        PubSubClient& setServer(const char * domain, uint16_t port);
        PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);
        PubSubClient& setClient(Client& client);
        PubSubClient& setStream(Stream& stream);

        boolean connect(const char* id);
        boolean connect(const char* id, const char* user, const char* pass);
        boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
        boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
        boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
        void disconnect();
        boolean publish(const char* topic, const char* payload);
        boolean publish(const char* topic, const char* payload, boolean retained);
        boolean publish(const char* topic, const uint8_t * payload, unsigned int plength);
        boolean publish(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);
        boolean publish_P(const char* topic, const char* payload, boolean retained);
        boolean publish_P(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);
        // Start to publish a message.
        // This API:
        //   beginPublish(...)
        //   one or more calls to write(...)
        //   endPublish()
        // Allows for arbitrarily large payloads to be sent without them having to be copied into
        // a new buffer and held in memory at one time
        // Returns 1 if the message was started successfully, 0 if there was an error
        boolean beginPublish(const char* topic, unsigned int plength, boolean retained);
        // Finish off this publish message (started with beginPublish)
        // Returns 1 if the packet was sent successfully, 0 if there was an error
        int endPublish();
        // Write a single byte of payload (only to be used with beginPublish/endPublish)
        virtual size_t write(uint8_t);
        // Write size bytes from buffer into the payload (only to be used with beginPublish/endPublish)
        // Returns the number of bytes written
        virtual size_t write(const uint8_t *buffer, size_t size);
        boolean subscribe(const char* topic);
        boolean subscribe(const char* topic, uint8_t qos);
        boolean unsubscribe(const char* topic);
        boolean loop();
        boolean connected();
        int state();*/
};

#endif // HomeAssistant_h
