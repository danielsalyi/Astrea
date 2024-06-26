#ifndef PIN_HPP_
#define PIN_HPP_

#include <climits>
#include <cstring>

class Pin {
public:
	Pin(GPIO_TypeDef *port, uint16_t pin) : port(port), pin(pin) { };

	inline void high() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); }
	inline void low() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); }
	inline void pulse() { high(); for (int i = 0; i < 1000; i++) { }; low(); }

	inline volatile bool isSet() { return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET; }

protected:
	GPIO_TypeDef *port;
	uint16_t pin;
};

#endif /* PIN_HPP_ */
