// File/Entry.cpp - This file is part of eln

/* eln is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   eln is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with eln.  If not, see <http://www.gnu.org/licenses/>.
*/

// Entry.C

#include "Entry.h"
#include "EntryData.h"
#include "EntryFile.h"
#include <QDebug>
#include "Assert.h"

Entry::Entry(EntryData *data): data_(data), file_(0) {
}

Entry::Entry(EntryFile *file): data_(file ? file->data(): 0), file_(file) {
}

Entry::~Entry() {
  if (file_) {
    file_->saveNow();
    delete file_;
  } else {
    delete data_;
  }
}   

EntryData *Entry::data() const {
  ASSERT(isValid());
  return data_;
}

EntryFile *Entry::file() const {
  ASSERT(hasFile());
  return file_;
}
