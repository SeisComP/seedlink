/***************************************************************************
 * Copyright (C) GFZ Potsdam                                               *
 * All rights reserved.                                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

class Buffer
  {
  protected:
    Buffer(int size_init): size(size_init) {}
    virtual ~Buffer() {}
  
  public:
    const int size;

    virtual void *data() const =0;
  };

class BufferStore
  {
  public:
    virtual Buffer *get_buffer() =0;
    virtual void queue_buffer(Buffer *buf) =0;
    virtual int size() const { return 0; }
    virtual void enlarge(int newsize) {}
    virtual ~BufferStore() {}
  };

#endif // BUFFER_H

