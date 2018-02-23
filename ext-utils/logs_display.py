import os.path
from functools import partial

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk

import logs_parser


FILE_FOLDER = os.path.abspath(os.path.dirname(__file__))
CSS = b"""
button.processing {
    background: blue;
}

button.decoded {
    background: green;
}

button.error {
    background: red;
}

button.sic {
    background: yellow;
}

button.marsala {
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

        self.slots = [Gtk.Button('') for _ in range(carrier_size)]
        self.reference = [Gtk.Button('') for _ in range(carrier_size)]
        carrier = builder.get_object('carrier')
        carrier.set_sensitive(False)
        reference = builder.get_object('reference')
        reference.set_sensitive(False)
        for button, ref in zip(self.slots, self.reference):
            carrier.pack_start(button, True, True, 0)
            reference.pack_start(ref, True, True, 0)

        self.frames = logs_parser.process_frames(log_filename)
        self.on_next_frame()

    def on_delete_window(self, *args):
        Gtk.main_quit(*args)

    def on_date_selected(self, adjustment):
        for button in self.slots:
            button.set_label('')
            context = button.get_style_context()
            context.remove_class('sic')
            context.remove_class('processing')
            context.remove_class('decoded')
            context.remove_class('error')
            context.remove_class('marsala')

        index = int(adjustment.get_value())
        if not index:
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
            self.slots[slot_id].set_label(str(packets))

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
        self.frame = next(self.frames, [])

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
                button.set_label(str(event.content.get(slot_id, '')))

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

    def run(self):
        self.window.show_all()
        Gtk.main()


def main():
    args = logs_parser.command_line_parser().parse_args()
    win = MainWindow(args.log_file, args.slots_count)
    win.run()


if __name__ == '__main__':
    main()
