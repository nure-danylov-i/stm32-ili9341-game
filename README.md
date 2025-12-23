# STM32-based electronic game with ILI9341 display module

[Українська](./README_ua.md)

![photo](./extra/images/photo_general.jpg)

## About

This device was created as part of a thesis titled "An interactive game for STM32 microcontrollers using a graphical display".

Dev board used is STM32 NUCLEO-F303RE.

The game is a classic space shooter where you fight enemies and get score points.

![gameplay](./extra/images/photo_gameplay.jpg)

Features:
- Smooth 25 FPS gameplay
- Touch or joystick-based controls
- Sound effects (wavetable synthesis)
- Game difficulty increases over time
- 3 different types of eneimes
- Highscores (stored in RAM)
- Blue button on the board pauses the game

## Schematic

Original:

![schematic](./extra/images/circuit_image.png)

In the schematic above, the buzzer is directly driven by the DAC output. This isn't a great idea, so instead use this schematic with an amplifier and an external power source:

![schematic with an amplifier](./extra/images/circuit_amp_image.png)


