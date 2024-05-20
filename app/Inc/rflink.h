#ifndef RFLINK_H_
#define RFLINK_H_

#include "main.h"
#include "sx1280.h"
#include "pin.hpp" //note to self, might add this to my pins.h, just for consistency.
#include <functional>

#include <stdint.h>
#include <map>

typedef struct {
	struct {
		uint16_t packetType : 3;
		uint16_t rfu : 3;
		uint16_t packetNumber : 10;
	} status;

	uint8_t payload[39];
} Packet;

typedef struct {
	uint8_t ch1Lsb;
	uint8_t ch2Lsb;
	struct {
		uint8_t ch1Msb : 4;
		uint8_t ch2Msb : 4;
	} ch12Msb;

	uint8_t ch3Lsb;
	uint8_t ch4Lsb;
	struct {
		uint8_t ch3Msb : 4;
		uint8_t ch4Msb : 4;
	} ch34Msb;

	uint8_t *begin() { return &ch1Lsb; }
	uint8_t *end() { return &ch4Lsb + 2; }
} ServoData;

typedef enum {
				INIT,
				START,
				SEND_OR_ENTER_RX,
	// Tx			// Rx
	TRANSMITTED,	WAITING_FOR_SYNC,
					SYNC_RECEIVED,
					WAITING_FOR_RECEIVE,
					RECEIVED,
				DONE,
				WAITING_FOR_NEXT_HOP
} LinkState;

typedef enum {
	NO_IRQ,
	MODULE1_IRQ,
	MODULE2_IRQ
} IrqSource;

class RfLink {
public:
	RfLink(TIM_HandleTypeDef *heartBeatTimer, bool transmitter) :
		heartBeatTimer(heartBeatTimer),
		transmitter(transmitter)
	{ };

	~RfLink() {
		delete rfModule;
	};

	void init();
	void processHeartBeat(TIM_HandleTypeDef *htim);
	void processIrqs(Pin pin);
	void runLoop(void);

	std::function<void(Packet &packet)> onTransmit;
	std::function<void(Packet &packet)> onTransmitTelemetry;
	std::function<void(Packet &packet)> onReceive;
	std::function<void(Packet &packet)> onReceiveTelemetry;

	SX1280 *rfModule;
	bool sender { true };
	void changeMode(void);
	void sendPacket(char *message);
	bool receivePacket(uint8_t* buffer, uint8_t* size, uint8_t maxSize);
	void enterRx(void);

private:
	TIM_HandleTypeDef *heartBeatTimer;

	bool transmitter;
	bool tracking { false };

	Pin rfTxEnable { Pin(RF_tx_GPIO_Port, RF_tx_Pin) };
	Pin rfRxEnable { Pin(RF_rx_GPIO_Port, RF_rx_Pin) };

	volatile LinkState state { INIT };
	uint16_t packetNumber;

	volatile bool heartBeatTimeout { false };
	volatile IrqSource lastIrqSource { NO_IRQ };
	uint32_t lostSync { 0 };
	uint32_t lostPacket { 0 };
	std::map<uint8_t, int> failuresPerChannel { };
	uint16_t timerWhenSynReceived { 746 }; // Measured

	const uint32_t trackingHopRate { 9999 }; // 10 ms
	const uint32_t acqusitionHopRate { 29999 }; // 30 ms
	const uint8_t lostSyncTreshold { 20 };
	const uint8_t downLinkFrequency { 4 };

	bool upLink(void);
	void setTracking(bool tracking);
	void adjustTimerToTrackTx(void);
	void registerLostPacket(void);
};

#endif /* RFLINK_H_ */