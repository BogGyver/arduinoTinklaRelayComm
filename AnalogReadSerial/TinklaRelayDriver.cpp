/* USB EHCI Host for Teensy 3.6
 * Copyright 2017 Michael McElligott
 * Copyright 2017 Paul Stoffregen (paul@pjrc.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <Arduino.h>
#include "USBHost_t36.h"  // Read this header first for key info
#include "TinklaRelayDriver.h"

#define TINKLA_RELAY_VID   0xbbaa
#define TINKLA_RELAY_PID   0xddcc


void TinklaRelay::init()
{
	contribute_Pipes(mypipes, sizeof(mypipes)/sizeof(Pipe_t));
	contribute_Transfers(mytransfers, sizeof(mytransfers)/sizeof(Transfer_t));
	contribute_String_Buffers(mystring_bufs, sizeof(mystring_bufs)/sizeof(strbuf_t));
	driver_ready_for_device(this);
}

bool TinklaRelay::claim(Device_t *dev, int type, const uint8_t *descriptors, uint32_t len)
{
  	if (dev->idVendor != TINKLA_RELAY_VID) return false;
	if (dev->idProduct != TINKLA_RELAY_PID) return false;
	rxpipe = txpipe = NULL;
  return true;
}

void TinklaRelay::disconnect()
{
	updatetimer.stop();
	//txtimer.stop();
}


void TinklaRelay::rx_callback(const Transfer_t *transfer)
{
	if (!transfer->driver) return;
	((TinklaRelay *)(transfer->driver))->rx_data(transfer);
}

void TinklaRelay::tx_callback(const Transfer_t *transfer)
{
        if (!transfer->driver) return;
        ((TinklaRelay *)(transfer->driver))->tx_data(transfer);
}

void TinklaRelay::rx_data(const Transfer_t *transfer)
{
	
}

void TinklaRelay::tx_data(const Transfer_t *transfer)
{
	
}


size_t TinklaRelay::write(const void *data, const size_t size)
{
	return 0;
}


void TinklaRelay::Task()
{
	
}