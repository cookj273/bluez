/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015,2016 Felipe F. Tonello <eu@felipetonello.com>
 *  Copyright (C) 2016 ROLI Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */

#ifndef LIBMIDI_H
#define LIBMIDI_H

#include <stdint.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>

#define MIDI_MAX_TIMESTAMP 8191
#define MIDI_MSG_MAX_SIZE 12
#define MIDI_SYSEX_MAX_SIZE (4 * 1024)

struct midi_buffer {
	uint8_t *data;
	size_t len;
};

/* MIDI I/O Write parser */

struct midi_write_parser {
	int64_t rtime;                  /* last writer's real time */
	snd_seq_event_type_t rstatus;   /* running status event type */
	struct midi_buffer midi_stream; /* MIDI I/O byte stream */
	size_t stream_size;             /* what is the maximum size of the midi_stream array */
	snd_midi_event_t *midi_ev;      /* midi<->seq event */
};

int midi_write_init(struct midi_write_parser *parser, size_t buffer_size);

static inline void midi_write_free(struct midi_write_parser *parser)
{
	free(parser->midi_stream.data);
	snd_midi_event_free(parser->midi_ev);
}

static inline void midi_write_reset(struct midi_write_parser *parser)
{
	parser->rstatus = SND_SEQ_EVENT_NONE;
	parser->midi_stream.len = 0;
}

static inline bool midi_write_has_data(const struct midi_write_parser *parser)
{
	return parser->midi_stream.len > 0;
}

static inline const uint8_t * midi_write_data(const struct midi_write_parser *parser)
{
	return parser->midi_stream.data;
}

static inline size_t midi_write_data_size(const struct midi_write_parser *parser)
{
	return parser->midi_stream.len;
}

typedef void (*midi_read_ev_cb)(const struct midi_write_parser *parser, void *);

/* It creates BLE-MIDI raw packets from the a sequencer event. If the packet
   is full, then it calls write_cb and resets its internal state as many times
   as necessary.
 */
void midi_read_ev(struct midi_write_parser *parser, const snd_seq_event_t *ev,
                  midi_read_ev_cb write_cb, void *user_data);

/* MIDI I/O Read parser */

struct midi_read_parser {
	uint8_t rstatus;                 /* running status byte */
	int64_t rtime;                   /* last reader's real time */
	int16_t timestamp;               /* last MIDI-BLE timestamp */
	int8_t timestamp_low;            /* MIDI-BLE timestampLow from the current packet */
	int8_t timestamp_high;           /* MIDI-BLE timestampHigh from the current packet */
	struct midi_buffer sysex_stream; /* SysEx stream */
	snd_midi_event_t *midi_ev;       /* midi<->seq event */
};

int midi_read_init(struct midi_read_parser *parser);

static inline void midi_read_free(struct midi_read_parser *parser)
{
	free(parser->sysex_stream.data);
	snd_midi_event_free(parser->midi_ev);
}

static inline void midi_read_reset(struct midi_read_parser *parser)
{
	parser->rstatus = 0;
	parser->timestamp_low = 0;
	parser->timestamp_high = 0;
}

/* Parses raw BLE-MIDI messages and populates a sequencer event representing the
   current MIDI message. It returns how much raw data was processed.
 */
size_t midi_read_raw(struct midi_read_parser *parser, const uint8_t *data,
                     size_t size, snd_seq_event_t *ev /* OUT */);

#endif /* LIBMIDI_H */
