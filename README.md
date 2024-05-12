# dsbf_dump

This homebrew can dump the Nintendo DS BIOS and firmware from a Nintendo DS, DSi and 3DS with a Slot-1 flashcard or a Slot-2 flashcart.

### Usage
1. Get a flash cartridge that can run NDS homebrew.
1. Download `dsbf_dump.nds`, place it on your SD card.
1. Run the app, and it should place your BIOS and firmware in a `FW############` folder on your SD card.
    - Replace `############` with the MAC address of the device

### Usage (IS-NITRO)
1. Download `dsbf_dump.nds` and rename the file extension to `.srl`
1. Load the app into IS-NITRO-DEBUGGER
1. For each section that is dumped:
    - Use the debugger to dump the memory contents
    - Find the area with `0xDE`s, this is where the dump presides. Cut off all the parts before and including the `0xDE`s
    - Find the area of `0xAD`s, this is the end of the dump. Cut off all the parts including and after the `0xAD`s

### Credits
- [cory1492](https://web.archive.org/web/20071027043203/http://nds.cmamod.com:80/2007/01/24/dsbf_dump-79-bios-firmware-dumper/), of which this program is a spiritual successor to. 
    - The original source was uploaded by cory1492 [here](https://kippykip.com/index.php?threads/483/), and has since been completely rewritten
- [SombrAbsol](https://github.com/SombrAbsol), who designed the new icon
- [Mythra](https://github.com/Mythra), who added needed workarounds to dump IS-NITRO firmwares
