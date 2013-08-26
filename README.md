FLG_sounds
==========

Sound design audio files for the FLG Xylophage project.

Directory structure:
example mixes     - Contains mixdowns that show generally how the sounds will go together.
normalized        - Normalized sound files to use in volume-adjustable contexts.
volume_adjusted   - Sound files where the volume matches the level in the mixdown.


Electrical notes:
=================

The felt sensor is a home made FSR (force sensing resistor) built with resistive fabric.
It acts like a variable resistor but it has a *very* tiny value (it can have Rmax = 10ohms).
If we want to avoid killing our poor solar charged batteries, it can be tricky to measure its value.
A few different techniques exist, we could for example charge a tiny capacitor and measure the charging time but we used a dirty hack that makes it simpler:

We know that digital GPIO current is limited to 40mA so if we try to give 5V across our tiny resistor the following rule tell us what the voltage will really be:

Rt . Il = Vr (Rt: tiny resistance, Il: limited current, Vr: real voltage)

So if Il = 10ohms, we get: Vr = 10ohms x 40mA = 400mV (and if we press the sensor, R=0 then V=0).

The electronic connections look like this:

________
GPIO -> | --\
        |    } --> FSR --> GND
ADC  <- | --/
        |
atmega  |
________|


The measurement is the following:
 - set the GPIO output to 5V
 - we measure the input voltage (ADC)
 - set the GPIO output to 0V to save power

