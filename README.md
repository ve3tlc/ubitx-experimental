# ubitx-experimental

I am new to github and this is my first attempt at a github project. You have been warned.
I have ordered my UBITX HF radio, and while I wait for it to arrive I am experimenting(?) with the factory release source code.
Note that none of these changes have been tested because I have not received my UBITX HF radio yet. It propably won't arrive until Feb 2018. At that point I will do some s/w testing. You have been warned again.

This revision of the S/W originated with the factory installed version of the ubitx hf radio s/w. I take no credit for the great job that was done to produce that easily understandible software.

This experimental version has been modified to remove all compiler warnings in the original arduino ino files.
The printline1 routine in the original seemed to output to line 2 of the display and conversely the printline2 routine displayed on line 1 of the lcd display. These two routines have been converted into macros (slightly less code and slightly faster execution without sacrificing readability). PRINTLINE1 now outputs to line1 of the LCD display and PRINTLINE2 outputs to line2 of the LCD display. All the original calls to these routines have been adjusted to call the appropriate PRINTLINEx macro. Hopefully everything still gets displayed where it was intended.
It seems that the same wiring can be used for a 4x20 LCD display as is used for a 2x16 display. Since I have a 4x20 display I added an #ifdef to the software that allows for a 4x20 LCD display in place of the 2x16 LCD display. When a 4x20 display is enabled there are 2 extra macro routines PRINTLINE3 and PRINTLINE4 that can be executed to write to the 3rd and 4th lines of the 4x20 display. The default is for the 2x16 display.

I got a little sidelined when I realised I could use my LDG-817Z autotuner with the UBITX. I beefed up the UBITX cat implementation but that may have been in vain since there isn't a simple way of getting the rs232 traffic to/from the auto tuner to the Radiono and utilize the one button auto-tune of the LDG-817Z. I can still use the tuner in manual mode but even that requires another button on the UBITX. This extra button would put the UBITX in CW mode and start a transmit. That also has its problem in that if I use the spare ananlog 7 pin to get more buttons I will lose the ability to implement a cw decoder. In my professional life I have used the i2c port expanders so that may be an option that leaves the analog 7 pin available and provides extra buttons for the front panel. 

Now the not yet implemented stuff:

My initial goal was to attempt to introduce a morse decoder into the s/w and use LCD lines 3 and 4 of the 4x20 LCD for CW related information. This would require a connection to the spare analog 7 pin. I did a test build withstripped down geortzel tone detection and still had code space but marginal data space. More work required before anything gets committed.

