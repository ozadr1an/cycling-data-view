////////////////////////////////////////////////////////////////////////////////
// The following FIT Protocol software provided may be used with FIT protocol
// devices only and remains the copyrighted property of Dynastream Innovations Inc.
// The software is being provided on an "as-is" basis and as an accommodation,
// and therefore all warranties, representations, or guarantees of any kind
// (whether express, implied or statutory) including, without limitation,
// warranties of merchantability, non-infringement, or fitness for a particular
// purpose, are specifically disclaimed.
//
// Copyright 2008 Dynastream Innovations Inc.
////////////////////////////////////////////////////////////////////////////////
// ****WARNING****  This file is auto-generated!  Do NOT edit this file.
// Profile Version = 1.50Release
// Tag = $Name: AKW1_500 $
////////////////////////////////////////////////////////////////////////////////


#if !defined(FIT_HRV_MESG_HPP)
#define FIT_HRV_MESG_HPP

#include "fit_mesg.hpp"

namespace fit
{

class HrvMesg : public Mesg
{
   public:
      HrvMesg(void) : Mesg(Profile::MESG_HRV)
      {
      }

      HrvMesg(const Mesg &mesg) : Mesg(mesg)
      {
      }

      ///////////////////////////////////////////////////////////////////////
      // Returns number of time
      ///////////////////////////////////////////////////////////////////////
      FIT_UINT8 GetNumTime(void) const
      {
         return GetFieldNumValues(0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
      }

      ///////////////////////////////////////////////////////////////////////
      // Returns time field
      // Units: s
      // Comment: Time between beats
      ///////////////////////////////////////////////////////////////////////
      FIT_FLOAT32 GetTime(FIT_UINT8 index) const
      {
         return GetFieldFLOAT32Value(0, index, FIT_SUBFIELD_INDEX_MAIN_FIELD);
      }

      ///////////////////////////////////////////////////////////////////////
      // Set time field
      // Units: s
      // Comment: Time between beats
      ///////////////////////////////////////////////////////////////////////
      void SetTime(FIT_UINT8 index, FIT_FLOAT32 time)
      {
         SetFieldFLOAT32Value(0, time, index, FIT_SUBFIELD_INDEX_MAIN_FIELD);
      }

      ///////////////////////////////////////////////////////////////////////
      // Returns checksum field
      ///////////////////////////////////////////////////////////////////////
      FIT_CHECKSUM GetChecksum(void) const
      {
         return GetFieldUINT8Value(252, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
      }

      ///////////////////////////////////////////////////////////////////////
      // Set checksum field
      ///////////////////////////////////////////////////////////////////////
      void SetChecksum(FIT_CHECKSUM checksum)
      {
         SetFieldUINT8Value(252, checksum, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
      }

};

} // namespace fit

#endif // !defined(FIT_HRV_MESG_HPP)