# Lavarand using OV2640 camera module (STM32H743ZI2)
An STM32 HAL demo of a true/hardware random number generator with a camera as the entropy source.

![CMOS image sensor entropy in action](/Assets/Images/lavarand_in_action.jpg)

First of all, I'm sorry for the clickbait regarding the lava :volcano: Although I was inspired by the original Lavarand (AKA the Wall of Entropy)[^1], I decided to use dirt cheap tealights instead of the fancy lava lamps. To my surprise, a single tealight was sufficient to pass the NIST suite of tests. To my even bigger surprise, true random numbers can be generated just by staring blankly at the plain wall. In my case apparently the image sensor itself is a sufficient source of entropy for generating at least 256 random bits per image. I learned that by accident actually - the candle went out when I was gathering the data. Now my TRNG is pointed at the ceiling and it is doing fine. I should probably coin a new name for it - the Ceiling of Entropy? In fact the entropy is not harvested from the ceiling, which is far from a chaotic system such as lava lamps or double pendulums, but it comes from the sensor noise. And you can do it with any CMOS camera sensor - the accompanying noise is inherent in such semiconductor devices. My above-mentioned "being surprised" came from the privilege of being a rookie cryptographer - the relevant fallacy is described a couple of sections below.

In my example the OV2640 camera module is configured to output the image in the compressed JPEG format. The SHA256 is deployed as the entropy extractor.

> [!IMPORTANT]
> If the lossy compression is used, a part of the entropy introduced by the sensor is lost.

> [!IMPORTANT]
> Pointing the camera at a chaotic system increases the combined entropy.

> [!IMPORTANT]
> Hashing does not increase entropy because hashing is a deterministic process. Hashing can be used as an entropy extractor, not an entropy source.

You might ask why use a camera to generate random numbers when we have a (T)RNG peripheral present in the majority of STM32 microcontrollers. Because it's fun![^2] Because it's interesting![^3] And because you can learn a couple of things along the way. You probably know the drill by now :slightly_smiling_face: The main objective for this submission is to inspire you to play with the DCMI peripheral and a cryptographic library.

> [!TIP]
> Some STM32 uCs have the HASH peripheral. SHA256 can then be calculated without bothering the main core. On all others one can use a cryptographic library, such as [Mbed TLS](https://github.com/Mbed-TLS/mbedtls), to hash data.

[^1]: [Lavarand](https://en.wikipedia.org/wiki/Lavarand) (Wikipedia) and [The Lava Lamps That Help Keep The Internet Secure](https://www.youtube.com/watch?v=1cUUfMeOijg) (Tom Scott)
[^2]: ["Why Do We Do Physics? Because Physics Is Fun!"](https://www.annualreviews.org/content/journals/10.1146/annurev-nucl-101918-023359) (James D. Bjorken)
[^3]: [Prof. Andrzej Dragan - Między prawdą a niewiedzą | AI, Edukacja, Fizyka Teoretyczna | SGMK](https://www.youtube.com/watch?v=zW8_XEcUKD8) (SGMK_ Nicolaus Copernicus Superior School)

# Missing files?
Don't worry :slightly_smiling_face: Just hit Alt-K to generate /Drivers/CMCIS/, /Drivers/STM32H7xx_HAL_Driver/, /MBEDTLS/App/ and /Middlewares/Third_Party/mbedTLS/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Software tools
* [RealTerm: Serial/TCP Terminal](https://sourceforge.net/projects/realterm/) - check the Capture tab :hammer_and_wrench:
* [Tabby](https://tabby.sh/) - because I like its modern look :slightly_smiling_face:

![RealTerm capture tab](/Assets/Images/realterm_rng_capture.JPG)

# Exemplary hardware
* [USB to UART converter](https://www.waveshare.com/ch343-usb-uart-board.htm) (Waveshare) - just for the convenience of having two separate COM ports: one for debugging messages and the other one for streaming random bits to the computer for their statistical evaluation.

# Docs, tutorials, libraries and examples

## Some random :wink: places to start
* [Really Really Random](http://www.reallyreallyrandom.com/)
* ["So what can the NIST tests do? Not much, really."](https://crypto.stackexchange.com/questions/67509/nist-randomness-test-p-values)
* [peteroupc.github.io](https://peteroupc.github.io/)

## Hash
* [https://en.m.wikipedia.org/wiki/Hash_function](https://en.m.wikipedia.org/wiki/Hash_function) (Wikipedia)
* [Cryptographic hash function](https://en.wikipedia.org/wiki/Cryptographic_hash_function) (Wikipedia)
* [Security Part3 - STM32 Security features - 28 - HASH theory](https://www.youtube.com/watch?v=2P8J932VSfQ) (ST)
* [STM32F7 OLT - 20. Peripheral - HASH](https://www.youtube.com/watch?v=NMz2LIjDjfk) (ST)
* [Mbed TLS](https://github.com/Mbed-TLS/mbedtls)
* [STM32 cryptographic firmware library software expansion for STM32Cube](https://www.st.com/en/embedded-software/x-cube-cryptolib.html)
* [SHA-256 Hash example on mbed OS](https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-tls-hashing/file/c68a6dc8d494/main.cpp/)
* [Hash Calculator Online](https://www.pelock.com/products/hash-calculator)

## CRC
* [Table of weight-5 binary primitive polynomials with (roughly) equally spaced coefficients](https://www.jjj.de/mathdata/eq-primpoly-w5.txt)
* [PCG, A Family of Better Random Number Generators](https://www.pcg-random.org/)
* [Poorman's Quite Random Number Generator (PQRNG) by Hardware](https://www.gniibe.org/memo/development/gnuk/rng/pqrng.html)

> [!TIP]
> If your uC does not offer the HASH peripheral and you are desperate to do it outside the main core, consider experimenting with a CRC peripheral. The CRC peripheral is more prevalent in uCs - all STM32 uC (based on Arm Cortex cores) provide it [[AN4187]](https://www.st.com/resource/en/application_note/an4187-using-the-crc-peripheral-on-stm32-microcontrollers-stmicroelectronics.pdf).

## JPEG and entropy extractors
* [JPEG: Syntax and structure](https://en.wikipedia.org/wiki/JPEG#Syntax_and_structure) (Wikipedia)
* [How can I extract randomness from a JPEG file?](https://crypto.stackexchange.com/questions/43115/how-can-i-extract-randomness-from-a-jpeg-file) (StackExchange)
* [Is this truly a TRNG?](https://crypto.stackexchange.com/questions/43010/is-this-truly-a-trng) (StackExchange)
* [Extracting randomness from highly-biased RNG](https://crypto.stackexchange.com/questions/48629/extracting-randomness-from-highly-biased-rng) (StackExchange)
* [Why does entropy halve during its extraction?](https://crypto.stackexchange.com/questions/41967/why-does-entropy-halve-during-its-extraction) (StackExchange)
* [A Note on Randomness Extraction](https://peteroupc.github.io/randextract.html)
* [Google search: hash entropy extractor](https://www.google.com/search?q=hash+entropy+extractor)

## DCMI peripheral and OV2640 camera sensor
* [AN5020: Introduction to digital camera interface (DCMI) for STM32 MCUs](https://www.st.com/resource/en/application_note/an5020-digital-camera-interface-dcmi-on-stm32-mcus-stmicroelectronics.pdf)
* [BSP OV2640 Component](https://github.com/STMicroelectronics/stm32-ov2640)
* [Fully implementation library support OV2640 camera on STM32 platform](https://github.com/SimpleMethod/STM32-OV2640) (SimpleMethod)
* [ArduCAM Library Introduction](https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/ov2640_regs.h) (ArduCAM)
* [LibDriver OV2640](https://github.com/libdriver/ov2640)

## Exemplary devices
* [MIKROE RNG Click](https://www.mikroe.com/rng-click)
* [Infinite Noise TRNG](https://www.fabtolab.com/infinite-noise-TRNG-random-number-generator), [Infinite Noise TRNG source](https://github.com/waywardgeek/infnoise)
* [A Fast, Cheap, High-Entropy Source for IoT Devices](https://www.hallsteninnovations.com/wp-content/uploads/2017/05/Ben.pdf)

## A book to read
* ["Random Number Generators - Principles and Practices"](https://www.degruyter.com/document/doi/10.1515/9781501506062/html?lang=en) by [David Johnston](https://www.linkedin.com/in/david-johnston-201529)
* [Random Number Generators - Principles and Practices](https://github.com/dj-on-github/RNGBook_Code) (programs and data to accompany the book)
* [Random Number Generators. Part 1. Terminology.](https://www.youtube.com/watch?v=tZse1YyiHdg&list=PLZNqNoh4u1gzKMYgrrgcKK5ozNQ7f_OMP) (TechyTime)

## Testing randomness - exemplary suites
* [Randomness test](https://en.wikipedia.org/wiki/Randomness_test) (Wikipedia)
* [Random Bitstream Tester](https://mzsoltmolnar.github.io/random-bitstream-tester/)
* [Diehard tests](https://en.wikipedia.org/wiki/Diehard_tests) (Wikipedia)
* [TestU01](https://en.wikipedia.org/wiki/TestU01) (Wikipedia)
* [Practically Random](https://sourceforge.net/projects/pracrand/) (SourceForge)
* [gjrand random numbers](https://sourceforge.net/projects/gjrand/) (SourceForge)

# One of the most common fallacies (Which came first, :chicken: or :egg:?[^4])
[TRNG vs PRNG - Entropy?](https://crypto.stackexchange.com/questions/26853/trng-vs-prng-entropy) - the fallacy is discussed in [the top answer](https://crypto.stackexchange.com/a/26862). Don't fall into this fallacy and be aware that there exist no test that can tell apart a TRNG generated stream and a PRNG generated one, assuming that the former is properly whitened and the latter is of a good quality. Statistical tests, such as the ones standardized by the NIST[^5], do not test for true randomness - they test the probability that a perfect random number generator would have produced a sequence less random than the sequence that was tested, given the kind of nonrandomness assessed by the test.

[^4]: There was no first chicken nor the first chicken's egg. We call it evolution :wink:
[^5]: [NIST Computer Security Resource Center](https://csrc.nist.gov/)

# Some results
Test results for the camera plus SHA256:

![Lavarand SHA256](/Assets/Images/lavarand_sha256_1_mln_bits.JPG)

Test results for the camera plus CRC32:

![Lavarand CRC32](/Assets/Images/lavarand_crc32_ca_1_mln_bits.JPG)

The code lets you experiment also with a PRNG solely based on SHA256 algorithm: SHA256(seed++)[^6].

Test results for the PRNG SHA256(seed++):

![PRNG SHA256(seed++)](/Assets/Images/prng_sha256_1_mln_bits.JPG)

[^6]: [Using SHA2 as random number generator?](https://crypto.stackexchange.com/questions/81455/using-sha2-as-random-number-generator) (StackExchange)

# Area 51's mysteries
* The function returning the p-value bigger than 1.0f - it's really baffling. A bug xor an incorrect nomenclature?
* Unable to find a configuration sequence for OV2640 that is free from write/read errors. Still I'm able to produce a viable JPEG file (tested by writing it to an STM32 USB MSC Device and displaying it on a PC). A different version of the chip? A knock-off/fake chip? No clue. Leaving it like this. The chip was officially discontinued in 2009, therefore probably the ones available today are all clones. No time to investigate that further - I will focus on incorporating OV5640 :camera: with the autofocus function into my future projects[^7].

![OV2640 JPEG config results](/Assets/Images/ov2640_jpeg_config.JPG)

[^7]: [Enable OV5640's autofocus function on ESP32 AI-THINKER Board](https://github.com/0015/ESP32-OV5640-AF) and [ESP32 Webcam With Autofocus: Using Adafruit Ov5640 Breakout With Esp32-s3-devkitC-1-N8R8 (Also, a Basic Guide to Using PlatformIO)](https://www.instructables.com/ESP32-Webcam-With-Autofocus-Using-Adafruit-Ov5640-/) (Instructables).

# What next?
Explore more entropy sources. Maybe something from my to-do list: a digital mic plus the I2S peripheral, TC plus VREF + TIM[^8], resistors plus opamps[^9], a Zener diode plus opamps, two transistors plus opamps, and a physical dice roller plus computer vision (Pixy2[^10]).

[^8]: [Poorman's Quite Random Number Generator (PQRNG) by Hardware](https://www.gniibe.org/memo/development/gnuk/rng/pqrng.html)
[^9]: [A true random number generator that utilizes thermal noise in a programmable system-on-chip (PSoC)](https://onlinelibrary.wiley.com/doi/abs/10.1002/cta.3046) by Shunsuke Matsuoka, Shuichi Ichikawa and Naoki Fujieda
[^10]: [Introducing Pixy2](https://pixycam.com/pixy2/)

# Call for action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2024_dzien_otwarty_we/Dzien_Otwarty_WE_2024_Control_Engineering_for_Hobbyists.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Automation and control engineering - do try this at home :exclamation:

190+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned!
