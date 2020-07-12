Notes common to all four utilities.

1. The heap_alloc() function does not have a corresponding free() function.
   The intent of heap_alloc() is only to allocate from our heap, memory
   with a physical address.  It is assumed that there will be enough memory
   in the heap and the allocated memory does not need to be free'd except
   when the heap itself is free'd.

   It was not my intent to have a robust allocation scheme.  Just a simple
   allocation scheme to show the examples.  You will need to create your
   own allocation scheme if you plan to do more memory allocation.

   Since there is no free() function, if you call the control_in() funtions
   multiple times, you will quickly run out of memory.

2. These examples do not have as much error checking as they probably should.
   It is not my intent to show how to catch common errors.  It is my intent
   to show how to retrieve the Device Descriptor of an attached device.

3. These examples assume that there is only one device plugged in on any
   given downstream port.  i.e.:  These utilities will not retrieve nor
   communicate with any devices plugged into an external hub.  These utilities
   will only retrieve the Device Descriptor from the device plugged in to
   the root hub port.  However, you may have multiple root hub ports occupied.

4. These examples are intended to run on DOS.  They will not run on Linux or
   Windows.  These examples are intended to show how to communicate with the
   USB without relying on any underlining operating system, except for the
   actual memory allocation used.

