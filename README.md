# ubitx-experimental

I am new to github and this is my first attempt at a github project. You have been warned.
I have ordered my UBITX HF radio, and while I wait for it to arrive I am experimenting(?) with the factory release source code.
Note that none of these changes have been tested because I have not received my UBITX HF radio yet. It propably won't arrive until Feb 2018. At that point I will do some s/w testing. You have been warned again.

This revision of the S/W originated with the factory installed version of the ubitx hf radio s/w. I take no credit for the great job that was done to produce that easily understandible software.

This experimental version has been modified to remove all compiler warnings in the original arduino ino files.
The printline1 routine in the original seemed to output to line 2 of the display and conversely the printline2 routine displayed on line 1 of the lcd display. These two routines have been converted into macros (slightly less code and slightly faster execution without sacrificing readability). PRINTLINE1 now outputs to line1 of the LCD display and PRINTLINE2 outputs to line2 of the LCD display. All the original calls to these routines have been adjusted to call the appropriate PRINTLINEx macro. Hopefully everything still gets displayed where it was intended.
It seems that the same wiring can be used for a 4x20 LCD display as is used for a 2x16 display. Since I have a 4x20 display I added an #ifdef to the software that allows for a 4x20 LCD display in place of the 2x16 LCD display. When a 4x20 display is enabled there are 2 extra macro routines PRINTLINE3 and PRINTLINE4 that can be executed to write to the 3rd and 4th lines of the 4x20 display. The default is for the 2x16 display.

Now the not yet implemented stuff:

My ulitimate goal is to attempt to introduce a morse decoder into the s/w and use LCD lines 3 and 4 of the 4x20 LCD for CW related information. I have previously implemented a stand alone Geortzel based CW decoder on a nano using a microphone shield as the cw audio input. For this implementation the plan is to connect analog7 to the UBITX speaker output through the appropriate resistor network.
The challenge is getting the existing radio's control software and the additional CW decoder software all fitting in the data and code space on the nano. This may be possible if I remove the CAT software from the original and highly optimize the cw decoder implementation. Time will tell.
