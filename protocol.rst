SSI (SPI implementation on Tiva)
================================

To start over, we copied over the ti_master.c from Tivaware's examples/peripherals/ssi directory.
We then copied that file again and named it slave.c (ti_master also renamed to master.c) and
search for a SSI ISR implementation on the internet.

This implementation was copied over to the master and initially used IntRegister to register the
interrupt handler. We have also spent quite some time (3 hours) to get the Makefile working and to
fix all missing includes/dependicies.

We then decided to use the startup_isr.c to register the interrupt handler, instead of registering
it in the code itself.

After all that was done, we decided to test the implementation (by sending a simple number, 8), but
it didn't work unfortunately. In order to debug, we connected all outputs (Rx, Tx, Clk, Fss) to a
logic analyzer to see if data is even transmitted over the cable.

The result of it was confusing us, since the Clock was completly off the scale (High all the time)
and we suspected something to be broken hardware-wise. 2 Hours later it turned out that the data
we sent was too small and we therefore couldn't see it on the logic analyzer.

Therefore we increased the data size to a String with a length of 50 characters and we finally saw
something coming through the pipe!
Unfortunately, nothing was being displayed on the receiver side (slave while we were testing) and
we again suspected a cable/hardware issue.

We were able to temporarly fix this by commenting out all UART stuff and replacing it with LEDs.
When digging further into the issue, we found out that the UART port and the SSI port were the
same... a setup which is just made for failure.
So we moved the SSI over to port B but it still didn't work.

After looking into another project's code for enabling the UART, we re-ordered the order of the
commands which get called to setup the UART and it finally worked.

Back to solving the initial problem, the SSI not appearing to send data properly.
We extended the ISR which gets called when data on the interrupt arrives with the follwing LEDs:

Blue - interrupt called (Rx & Tx)
Green - Rx called

It was lighting blue all the time, so something was definitely happening. The original statement
to check if the interrupt was caused by receiving or transmitting data was checking the following:

unsigned long data_source == SSIIntStatus(SSI2_BASE, true)

if(data_source & SSI_RXTO) ...

This was checking if the SSI register was overran. For testing, we changed it to SSI_RXFF, which
checks if the register was only half-full or more.

We also added some UART messages for debugging and saw that the program was now entering the
branch for receiving messages.

Now it was stuck at the SSIDataGet function we use for reading data from the Rx register.
We put the SSIDataGet function into a loop with about 32,000 runs, but the LED was still blue.
This made us believe that it's actually running too often, so we decided a preprocessor directive
NUM_DATA which holds the amount of bytes being sent/to be received.

Now when sending the data, instead of looping through all characters, we only looped till NUM_DATA.
The same for reading the data. We were getting closer, as the output (we added some UARTprintf's)
on the receiving part was printing out the message, but when resetting the sender part (using the
reset button), the message was being truncated (parts of it being cut of, being printed the next time
the receiver gets a message).
And we noticed something: each time the message was printed out, the null-terminator (0x00) was printed
too.

Because of this, we added a check in the master's code to verify if the character we are sending is a
null-terminator, and if it is, kill the loop. Now it finally worked - we got the entire message.

The next step would be adding "messages" (actually bitmasks) to control the LED's on the master, using
the slave's buttons. For our tests we are sending a hardcoded value (RED & GREEN LED) from the master
to the slave.
To get this to work, we removed the 0x00 check from the master and set the data length to 1. Now, the
next problem appeared - there is no data sent (or at least the interrupt is not being executed) unless
we sent more than 4 bytes.
This appears to be a problem with the ISR implementation, as it only executes the interrupt if the 
receiving register is half full or more. We were able to confirm this by resetting the sender part
multiple times and we got the message each time. As a temporary fix, we simply send the data 4 times,
so we don't have to press the button 4 times each.

Next thing we tried is swapping the direction of the data transfer. Currently, it is from Master to
Slave, our target is that the Slave sends data to the Master. As our first try, we swapped the files
(master.c <-> slave.c) and also set the modes for each to SSI_MODE_MASTER/SSI_MODE_SLAVE respectively.

Theoretically, this should be enough to make the Slave talk to the Master, but when we tested it, the
Slave only "sent" 5 bytes of data ("sent" because it was only written to the register, not the output
itself actually). The Master's interrupt was never triggered.

Later on we learned that the Slave can't talk to the Slave directly, but instead has to either request
the Master to to be able to talk or get set by the Master to talk.