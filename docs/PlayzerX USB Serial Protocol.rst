PlayzerX USB Serial Protocol
===============================

Binary Mode Serial Command Structure
--------------------------------------

.. list-table::
   :widths: 10 10 10 10 10 10 10 10 10
   :header-rows: 1

   * - **Byte 1**
     - **Byte 2**
     - **Byte 3**
     - **Byte 4**
     - **Byte 5**
     - **Byte 6**
     - **Byte 7**
     - **Byte 8**
     - **Byte N**
   * - Prefix 1
     - Prefix 2
     - Command Code
     - Command Length
     - data
     - data
     - data
     - data
     - Suffix
   * - 112 "p"
     - 108 "l"
     - xx
     - N
     - xx
     - xx
     - xx
     - xx
     - 10 "\\n"

Command Reference Table
-----------------------

.. list-table::
   :widths: 10 5 10 10 10 10 8 8 8 30
   :header-rows: 1

   * - **Cmd Code** (B3)
     - **Cmd Len** (B4)
     - **Byte 5**
     - **Byte 6**
     - **Byte 7**
     - **Byte 8**
     - **Byte 9**
     - **Byte 10**
     - **Byte 11**
     - **Function Description**

   * - 68("D")
     - 8
     - X[7:0]
     - X[11:8], Y[3:0]
     - Y[11:4]
     - 10("\\n")
     - 
     - 
     - 
     - Send an XY sample to controller (M=255)

   * - 100("d")
     - 9
     - X[7:0]
     - X[11:8], Y[3:0]
     - Y[11:4]
     - M[7:0]
     - 10("\\n")
     - 
     - 
     - Send an XYM sample to controller

   * - 100("d")
     - 11
     - X[7:0]
     - X[11:8], Y[3:0]
     - Y[11:4]
     - R[7:0]
     - G[7:0]
     - B[7:0]
     - 10("\\n")
     - Send an XYRGB sample to controller

   * - 99("c")
     - 5
     - 10("\\n")
     - 
     - 
     - 
     - 
     - 
     - 
     - ClearData (reset FIFO buffer)

   * - 114("r")
     - 8
     - value [7:0]
     - value [15:8]
     - value [23:16]
     - 10("\\n")
     - 
     - 
     - 
     - SetSampleRate (samples/s)

   * - 117("u")
     - 7
     - value [7:0]
     - value [15:8]
     - 10("\\n")
     - 
     - 
     - 
     - 
     - SetBufferUpdateTimer (ms)

   * - 112("p")
     - 5
     - 10("\\n")
     - 
     - 
     - 
     - 
     - 
     - 
     - Ping (device check)

   * - 110("n")
     - 5
     - 10("\\n")
     - 
     - 
     - 
     - 
     - 
     - 
     - GetDeviceInfo

   * - 103("g")
     - 5
     - 10("\\n")
     - 
     - 
     - 
     - 
     - 
     - 
     - GetSamplesRemaining

   * - 105("i")
     - 8
     - 97("a")
     - 105("i")
     - 110("n")
     - 10("\\n")
     - 
     - 
     - 
     - Switch data input to AIN (analog)

   * - 105("i")
     - 8
     - 117("u")
     - 115("s")
     - 98("b")
     - 10("\\n")
     - 
     - 
     - 
     - Switch data input to USB

   * - 73("I")
     - 8
     - 97("a")
     - 105("i")
     - 110("n")
     - 10("\\n")
     - 
     - 
     - 
     - Switch data input to AIN + Flash

   * - 73("I")
     - 8
     - 117("u")
     - 115("s")
     - 98("b")
     - 10("\\n")
     - 
     - 
     - 
     - Switch data input to USB + Flash

   * - 98("b")
     - 6
     - 238(0xEE)
     - 10("\\n")
     - 
     - 
     - 
     - 
     - 
     - ResetDevice (boot MCU)


Value Ranges and Specifications
---------------------------------

- X and Y values are from 0 to 4095 corresponding to -1 to +1 co-ordinates, respectively (2048 is origin)
- M (or R, G, B) range from 0 to 255
- SampleRate can range from 50 to 50000 samples/s
- Total buffer size is 125000 samples for Monochrome version and 83333 samples for RGB
