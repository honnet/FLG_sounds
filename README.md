Welcome
=======

This project was a part of Xylophage, an art installation made for Burning Man 2013.

![Xylophage - Burning Man 2013](http://farm4.staticflickr.com/3675/9625015206_d0fff9cf0e_z.jpg)

One of the giant mushrooms can detect approaching humans, which makes it breathing faster (yes, a mushroom can breath too), like if our presence was (surprisingly?) a stressful event.
A sincere hug might calm it down and it expresses it with a whining sound...

This repository contains the code to control generated sounds, here is the architecture of the system:

    Sensors (touch/distance) --> teensy (micro controller) --> raspberry pi (tiny computer) --> sound


Electronic trick for the touch sensor interface
-----------------------------------------------

This sensor, also called felt sensor, is a home made FSR (Force Sensing Resistor) built with resistive fabric.
It acts like a variable resistor but it has a *very* tiny value (it can have `Rmax = 10ohms`).
If we want to avoid killing our poor solar charged batteries, it can be tricky to measure its value.
A few different techniques exist, we could for example charge a tiny capacitor and measure the charging time but we used a dirty hack that makes it simpler:

We know that digital GPIO current is limited to `I = 40mA` so if we try to give 5V across our tiny resistor the following rule tells us what the voltage will really be:

        R x I = V (R: tiny resistance, I: limited current, V: real voltage)

So if `I = 10ohms`, we get: `V = 10ohms x 40mA = 400mV` (and if we press the sensor `R = 0` then `V = 0`).

The electronic connections look like this:

         __________
        |          |
        |  GPIO -> | --\
        |          |    } --> FSR --> GND
        |   ADC <- | --/
        |          |
        |          |
        | (teensy) |
        |__________|


The measurement procedure is the following:
 - set the GPIO output to 5V
 - we measure the input voltage (ADC)
 - set the GPIO output to 0V to save power


Sounds
------

Sound design audio files for the FLG Xylophage project.

Directory structure:

    example mixes     - Contains mixdowns that show generally how the sounds will go together.
    normalized        - Normalized sound files to use in volume-adjustable contexts.
    volume_adjusted   - Sound files where the volume matches the level in the mixdown.
