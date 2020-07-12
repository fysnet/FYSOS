/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The System Core, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef MOD_MBR
#define MOD_MBR

#pragma pack(1)

char strtstr[] = 
         "\nModify MBR  v1.00.00  Forever Young Software -- (c) Copyright 1984-2017\n\n";

struct COMMAND_LINE {
  int    entry;         //
  bool   set_active;    //
  bool   set_inactive;  //
  bit8u  id;            //
  bool   id_given;      //
  bit32u base_lba;      //
  bool   base_given;    //
  bit32u size;          //
  bool   size_given;    //
};

void lba_to_chs(const bit32u lba, bit8u *cyl, bit8u *head, bit8u *sector);
bool parse_command(int argc, char *argv[], char *filename, struct COMMAND_LINE *items);


#endif // MOD_MBR
