/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2023
 *  
 *  This code is donated to the Freeware community.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and if distributed, have the same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  23 Jan 2023
 *
 */


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "registry.h"

// let's write a few things to the registry and dump it to the screen.
//
int main(int argc, char *argv[]) {
  int ret;
  
  // first we must allocate and initialize the registry
  ret = allocate_initialize_registry(1 * 1024 * 1024);
  if (ret < 0) {
    printf(" ***** Error:  Did not allocate the Registry\n");
    return ret;
  }
  
  // create a hive called 'Kernel' and a single cell called 'temp' with a value of 1234
  registry_write_int("/System/Kernel/temp", 1234);
  // to create an empty hive, we then delete the 'temp' cell
  registry_remove("/System/Kernel/temp");
  
  // then create the 'App0' hive and the 'Left' and 'Right' cells within it
  registry_write_int("/System/App0/Left", 100);
  registry_write_int("/System/App0/Right", 100);

  // and finally create the 'APIC' and 'HPET' cells
  registry_write_boolean("/System/APIC", 1);
  registry_write_boolean("/System/HPET", 1);
  
  // then dump the registry to see what it looks like  
  dump_registry();
}

// this would be a kernel call getting the current microseconds from the intended epoc.
//  for this demo, we just return 0
uint64_t timestamp(uint64_t now) {
  return 0;
}

// this would be a spin lock routine.
//  for this demo, we just return
void spin_lock(uint32_t *lock) {
  return;
}

// this would be a spin unlock routine.
//  for this demo, we just return
void spin_unlock(uint32_t *lock) {
  return;
}
