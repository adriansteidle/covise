/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

#ifndef __RRDISPLAYCLIENT_H
#define __RRDISPLAYCLIENT_H

#include "../include/rrsocket.h"
#include "../include/rrthread.h"
#include "../rr/rr.h"
#include "../rr/rrframe.h"
#include "../include/genericQ.h"
#include "../include/rrprofiler.h"
#ifdef _WIN32
#define snprintf _snprintf
#endif

#ifdef __linux__
#include <sys/prctl.h>
#endif

struct rrxevent;

class rrdisplayclient : public Runnable
{
public:
    rrdisplayclient(void);

    virtual ~rrdisplayclient(void)
    {
        _deadyet = true;
        _q.release();
        if (_t)
        {
            _t->stop();
            delete _t;
            _t = NULL;
        }
        if (_sd)
        {
            delete _sd;
            _sd = NULL;
        }
    }

    rrframe *getbitmap(int, int, int, int, bool stereo, bool spoil);
    bool frameready(void);
    void sendframe(rrframe *);
    struct rrxevent *getevent();
    void run(void);
    void sendheader(rrframeheader h, bool eof);
    void recvevents();
    void send(char *, int);
    void save(char *, int);
    int recv(char *, int);
    void connect(char *, unsigned short);
    void connect(rrsocket *sock);
    void record(bool domovie)
    {
        _domovie = domovie;
    }

    int _np;
    bool _dosend, _domovie;

private:
    rrsocket *_sd;
    static const int NB = 4;
    rrcs _bmpmutex;
    rrframe _bmp[NB];
    rrevent _ready;
    genericQ _q;
    genericQ _evq;
    Thread *_t;
    bool _deadyet;
    rrprofiler _prof_total;
    int _dpynum;
    rrversion _v;
};

#define endianize(h)                         \
    {                                        \
        if (!littleendian())                 \
        {                                    \
            h.size = byteswap(h.size);       \
            h.winid = byteswap(h.winid);     \
            h.framew = byteswap16(h.framew); \
            h.frameh = byteswap16(h.frameh); \
            h.width = byteswap16(h.width);   \
            h.height = byteswap16(h.height); \
            h.x = byteswap16(h.x);           \
            h.y = byteswap16(h.y);           \
            h.dpynum = byteswap16(h.dpynum); \
        }                                    \
    }

#define endianize_v1(h)                      \
    {                                        \
        if (!littleendian())                 \
        {                                    \
            h.size = byteswap(h.size);       \
            h.winid = byteswap(h.winid);     \
            h.framew = byteswap16(h.framew); \
            h.frameh = byteswap16(h.frameh); \
            h.width = byteswap16(h.width);   \
            h.height = byteswap16(h.height); \
            h.x = byteswap16(h.x);           \
            h.y = byteswap16(h.y);           \
        }                                    \
    }

#define cvthdr_v1(h, h1)                     \
    {                                        \
        h1.size = h.size;                    \
        h1.winid = h.winid;                  \
        h1.framew = h.framew;                \
        h1.frameh = h.frameh;                \
        h1.width = h.width;                  \
        h1.height = h.height;                \
        h1.x = h.x;                          \
        h1.y = h.y;                          \
        h1.qual = h.qual;                    \
        h1.subsamp = h.subsamp;              \
        h1.flags = h.flags;                  \
        h1.dpynum = (unsigned char)h.dpynum; \
    }

class rrcompressor : public Runnable
{
public:
    rrcompressor(int myrank, rrdisplayclient *parent)
        : _bytes(0)
        , _storedframes(0)
        , _frame(NULL)
        , _b(NULL)
        , _lastb(NULL)
        , _myrank(myrank)
        , _deadyet(false)
        , _parent(parent)
    {
        if (parent)
        {
            _np = parent->_np;
        }
        _ready.wait();
        _complete.wait();
        char temps[20];
        if (_parent->_dosend)
            snprintf(temps, 19, "Compress %d", myrank);
        else
            snprintf(temps, 19, "Comp(mov)%d", myrank);
        _prof_comp.setname(temps);
    }

    virtual ~rrcompressor(void)
    {
        shutdown();
        if (_frame)
        {
            free(_frame);
            _frame = NULL;
        }
    }

    void run(void)
    {
#ifdef __linux__
        char name[100];
        sprintf(name, "rrcompressor %d", _myrank);
        prctl(PR_SET_NAME, name, 0, 0, 0);
#endif
        while (!_deadyet)
        {
            try
            {

                _ready.wait();
                if (_deadyet)
                    break;
                compresssend(_b, _lastb);
                _complete.signal();
            }
            catch (...)
            {
                _complete.signal();
                throw;
            }
        }
    }

    void go(rrframe *b, rrframe *lastb)
    {
        _b = b;
        _lastb = lastb;
        _ready.signal();
    }

    void stop(void)
    {
        _complete.wait();
    }

    void shutdown(void)
    {
        _deadyet = true;
        _ready.signal();
    }
    void compresssend(rrframe *, rrframe *);
    void send(void);

    long _bytes;

private:
    void store(rrcompframe *c)
    {
        _storedframes++;
        if (!(_frame = (rrcompframe **)realloc(_frame, sizeof(rrcompframe *) * _storedframes)))
            _throw("Memory allocation error");
        _frame[_storedframes - 1] = c;
    }

    int _storedframes;
    rrcompframe **_frame;
    rrframe *_b, *_lastb;
    int _myrank, _np;
    rrevent _ready, _complete;
    bool _deadyet;
    rrcs _mutex;
    rrprofiler _prof_comp;
    rrdisplayclient *_parent;
};

#endif
