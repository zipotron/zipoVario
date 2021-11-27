# ZipoVario
A variometer for paraglading implemented with an Arduino nano and a athmosferic
sensor BMP-280.
It is using a very simple scheduler for handle four threads based in hardware
timer interupt, with diferent priorities and time asignament.
1º Thread reads the athmosferic presure and storing in an array.
2º Thread filters the signal in base of the array for calculate altitud and vertica speed.
3º Thread modulates the buzzer frequency in base of the vertical speed calculated by the other thread.
4º Thread modulates the power and time interval of the vibrator motor.

The variomenter emites sound with different gradient of frequency indicating if you go up
or down and the speed of the movement.

As an aditional componets, you need a buzzer, a vibration motor, a NPN transistor for
drive the motor and three resistors dor the buzzer (100ohm), transistor (10Kohm) and motor (47ohm),
and a diode for invert voltage protection.
