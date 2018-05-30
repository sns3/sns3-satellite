#!/usr/bin/env python3

# Copyright (c) 2018
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
#

import os.path
from functools import partial

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk

import logs_parser


FILE_FOLDER = os.path.abspath(os.path.dirname(__file__))
CSS = b"""
.slots GtkLabel, .slots label {
    padding-top: 10px;
    padding-bottom: 10px;
    border: 1px solid black;
    border-radius: 5px;
    font-weight: bold;
}

GtkLabel.processing, label.processing {
    color: white;
    background: blue;
}

GtkLabel.decoded, label.decoded {
    color: white;
    background: green;
}

GtkLabel.error, label.error {
    color: white;
    background: red;
}

GtkLabel.sic, label.sic {
    background: yellow;
}

GtkLabel.marsala, label.marsala {
    background: cyan;
}
"""


class MainWindow:
    def __init__(self, log_filename, carrier_size=150):
        builder = Gtk.Builder()
        builder.add_from_file(os.path.join(FILE_FOLDER, 'display.glade'))
        builder.connect_signals(self)

        cssprovider = Gtk.CssProvider()
        cssprovider.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_screen(
                Gdk.Screen.get_default(),
                cssprovider,
                Gtk.STYLE_PROVIDER_PRIORITY_USER)

        self.window = builder.get_object('main_window')
        self.set_status = builder.get_object('status').set_label
        self.slider = builder.get_object('date_selector')
        self.backward_sic = builder.get_object('backward_to_sic')
        self.forward_sic = builder.get_object('forward_to_sic')
        self.controls = builder.get_object('controls')
        self.backward_controls = builder.get_object('backward_controls')
        self.forward_controls = builder.get_object('forward_controls')

        self.carrier = builder.get_object('carrier')
        self.carrier_ref = builder.get_object('reference')
        self.size_tracker = logs_parser.CarrierSizeTracker(carrier_size)
        self.frames = logs_parser.process_frames(log_filename, self.size_tracker)
        self._build_slots()
        self.on_next_frame()

    def _build_slots(self, old_carrier_size=None):
        carrier_size = self.size_tracker.size

        if old_carrier_size != carrier_size:
            if old_carrier_size is not None:
                for slot, ref in zip(self.slots, self.reference):
                    slot.destroy()
                    ref.destroy()

            self.slots = [Gtk.Label('') for _ in range(carrier_size)]
            self.reference = [Gtk.Label('') for _ in range(carrier_size)]
            for slot, ref in zip(self.slots, self.reference):
                self.carrier.pack_start(slot, True, True, 0)
                self.carrier_ref.pack_start(ref, True, True, 0)

    def on_date_selected(self, adjustment):
        for label in self.slots:
            label.set_text('')
            context = label.get_style_context()
            context.remove_class('processing')
            context.remove_class('decoded')
            context.remove_class('error')
            context.remove_class('sic')
            context.remove_class('marsala')

        index = int(adjustment.get_value())
        if not index:
            self.forward_sic.set_sensitive(True)
            self.backward_sic.set_sensitive(True)

            if not self.frame:
                self.controls.set_sensitive(False)
                self.set_status('Date: —\n\nSimulation ended')
            else:
                self.backward_controls.set_sensitive(False)
                self.forward_controls.set_sensitive(True)
                self.set_status('Date: {0[0].date}'.format(self.frame))

            return
        else:
            self.backward_controls.set_sensitive(True)
            self.forward_controls.set_sensitive(index != len(self.frame))

        sic_index = self._index_of_first_sic_event
        self.forward_sic.set_sensitive(index < sic_index)
        self.backward_sic.set_sensitive(index > sic_index)

        event = self.frame[index - 1]
        date = event.date
        for slot_id, packets in event.content.items():
            self.slots[slot_id].set_text(str(packets))

        try:
            current_slot = event.slot
        except AttributeError:
            self.set_status('Date: {}\n\nReceiving packets'.format(date))
        else:
            replicas = event.replicas
            current_button = self.slots[current_slot]
            if replicas:
                interferences = '\n\t'.join('{} → {}'.format(*i) for i in replicas)
                self.set_status('Date: {}\n\nInterference elimination:\n\t{}'.format(date, interferences))
                current_button.get_style_context().add_class('sic')
            else:
                if getattr(event, 'is_marsala', False):
                    css_class = 'marsala'
                    extra_msg = '\n\tMARSALA pass'
                else:
                    css_class, extra_msg = {
                            None: ('processing', ''),
                            True: ('decoded', '\n\t→ decoded'),
                            False: ('error', '\n\t→ phy error'),
                    }[event.status]
                self.set_status('Date: {}\n\nPacket n°{}{}'.format(date, event.packet, extra_msg))
                current_button.get_style_context().add_class(css_class)

    def on_next_frame(self, *args):
        carrier_size = self.size_tracker.size
        self.frame = next(self.frames, [])
        self._build_slots(carrier_size)

        self._index_of_first_sic_event = -1
        try:
            date = self.frame[-1].date
        except IndexError:
            pass
        else:
            for index, event in enumerate(self.frame):
                if event.date == date:
                    self._index_of_first_sic_event = index
                    break

            event = self.frame[index]
            for slot_id, button in enumerate(self.reference):
                button.set_text(str(event.content.get(slot_id, '')))

        self.slider.set_upper(float(len(self.frame)))
        self.slider.set_value(0.0)
        self.on_date_selected(self.slider)

    def on_start_frame(self, *args):
        self.slider.set_value(0.0)
        self.on_date_selected(self.slider)

    def on_end_frame(self, *args):
        self.slider.set_value(float(len(self.frame)))
        self.on_date_selected(self.slider)

    def on_sic_begin(self, *args):
        sic_index = self._index_of_first_sic_event
        self.slider.set_value(float(sic_index))
        self.on_date_selected(self.slider)

    def on_previous(self, *args):
        self.slider.set_value(self.slider.get_value() - 1)
        self.on_date_selected(self.slider)

    def on_next(self, *args):
        self.slider.set_value(self.slider.get_value() + 1)
        self.on_date_selected(self.slider)

    def on_delete_window(self, *args):
        Gtk.main_quit(*args)

    def run(self):
        self.window.show_all()
        Gtk.main()


def main():
    args = logs_parser.command_line_parser().parse_args()
    win = MainWindow(args.log_file, args.slots_count)
    win.run()


if __name__ == '__main__':
    main()
