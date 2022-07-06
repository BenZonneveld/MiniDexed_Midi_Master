Overview
--------

This started out as a simple usb to midi interface for my MiniDexed. Then I wanted a more userfriendly interface.
After that I thought it would be possible to grab the i2s audio from the MiniDexed and use tinyusb for UAC2 audio.

While doing this I also made some patches for MiniDexed regarding System Exclusive data to make life easy.

* Does MIDI over USB.
* UAC2 audio over USB
* Normal MIDI, yes a second MIDI port!
* Double push on the TG selection screen to disable instances.
* Faster access to settings for the MiniDexed instances.
* Can load sound from sd card using the sd card slot in the display I use. (KMR 1.8 inch TFT)
