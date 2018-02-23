import re
import math
import tarfile
import argparse
from collections import defaultdict


class Event:
    def __init__(self, date, frame_content):
        self.date = date
        self.content = frame_content

    @property
    def max_size(self):
        return math.floor(math.log10(max(self.content.values()))) + 1

    def _format_carrier(self, size):
        max_size = self.max_size
        packet_count = self.content.get
        pattern = '─' * (max_size + 2)
        up = '╭{}╮'.format('┬'.join([pattern] * size))
        down = '╰{}╯'.format('┴'.join([pattern] * size))
        inner = '│{}│'.format('│'.join(
            ' {:>{pad}} '.format(packet_count(slot_id, ''), pad=max_size)
            for slot_id in range(size)
        ))
        return '{}\n{}\n{}'.format(up, inner, down)

    def format(self, size):
        return 'Date: {}\n{}\n'.format(self.date, self._format_carrier(size))


class SicEvent(Event):
    SYMBOLS = {
            None: '▾',
            True: '✓',
            False: '✗',
    }

    def __init__(self, date, frame_content, slot_iterated, packet=None, decoded=None, replicas=None):
        super().__init__(date, frame_content)
        self.slot = slot_iterated
        self.status = decoded
        self.packet = packet
        self.replicas = replicas

    def format(self, size):
        sic_position = ' ' * ((self.max_size + 3) * (self.slot + 1) - 2)
        sic_symbol = '{} Packet n°{}'.format(self.SYMBOLS[self.status], self.packet) if not self.replicas else '▿'
        carrier = self._format_carrier(size)
        slots = 'Date: {}\n{}{}\n{}\n'.format(self.date, sic_position, sic_symbol, carrier)

        if self.replicas:
            interferences = '\n\t'.join('{} → {}'.format(*i) for i in self.replicas)
            slots = '{}Interference elimination:\n\t{}\n'.format(slots, interferences)

        return slots


class MarsalaEvent(SicEvent):
    @property
    def is_marsala(self):
        return True


class Packet:
    ADD_REPLICA = re.compile(
            r'SatPhyRxCarrierPerFrame:AddCrdsaPacket\(\): A replica of the packet is in slot (\d+)')

    def __init__(self, date, slot, *replicas):
        self.date = date
        self.slot = slot
        self.replicas = replicas

    @classmethod
    def from_logs(cls, logs, slot, date):
        def extract_replicas():
            for line in logs:
                replica = cls.ADD_REPLICA.search(line)
                if replica:
                    yield int(replica.group(1))
                else:
                    break
        return cls(date, slot, *extract_replicas())

    def __str__(self):
        slots = ', '.join(map(str, (self.slot,) + self.replicas))
        return 'At {}, received packet in slots {}'.format(self.date, slots)


class Frame:
    MESSAGE_DATE = re.compile(r'^\+(\d+\.\d+)s\s')
    ADD_PACKET = re.compile(
            r'SatPhyRxCarrierPerFrame:AddCrdsaPacket\(\): Packet in slot (\d+) '
            'was added to the CRDSA packet container')
    END_FRAME = re.compile(r'SatPhyRxCarrierPerFrame:DoFrameEnd\(\): Packets in container, will process the frame')

    def __init__(self):
        self.slots = defaultdict(list)

    def __len__(self):
        return sum(map(len, self.slots.values()))

    def __iter__(self):
        for _, packets in sorted(self.slots.items()):
            yield packets

    def receive_packet(self, packet):
        slot_content = self.slots[packet.slot]
        assert all(packet.date == p.date for p in slot_content)
        slot_content.append(packet)

    @classmethod
    def from_logs(cls, logs):
        self = cls()

        for line in logs:
            packet_added = cls.ADD_PACKET.search(line)
            if packet_added:
                date = float(cls.MESSAGE_DATE.search(line).group(1))
                slot = int(packet_added.group(1))
                self.receive_packet(Packet.from_logs(logs, slot, date))
            elif cls.END_FRAME.search(line):
                break

        return self


def command_line_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log_file', type=argparse.FileType('rb'), help='')
    parser.add_argument('-s', '--slots-count', type=int, default=150, help='')

    return parser


def logs_content(file_handle):
    with file_handle:
        filename = file_handle.name
        if not tarfile.is_tarfile(filename):
            yield from map(bytes.decode, file_handle)
            return

    with tarfile.open(filename) as tarball:
        with tarball.extractfile('Components.log') as f:
            yield from map(bytes.decode, f)


def generate_frames_from_logs(logs):
    while True:
        frame = Frame.from_logs(logs)
        if not frame:
            break
        yield frame


def generate_sic_from_logs(logs, date, frame_content):
    begin_decode = re.compile(r'SatPhyRxCarrierPerFrame:PerformSicCycles\(\): Iterating packet in slot: (\d+)')
    begin_replica = re.compile(r'SatPhyRxCarrierPerFrame:FindAndRemoveReplicas\(\): Processing replica in slot: (\d+)')
    before_if = re.compile(r'BEFORE INTERFERENCE ELIMINATION, RX sat: .* IF sat: (.+) RX gnd:')
    after_if = re.compile(r'AFTER INTERFERENCE ELIMINATION, RX sat: .* IF sat: (.+) RX gnd:')
    # end_processed = re.compile(r'SatPhyRxCarrierPerFrame:PerformSicCycles\(\): This packet has already been processed')
    end_decode = re.compile(r'SatPhyRxCarrierPerFrame:PerformSicCycles\(\): Packet error: (\d)')
    end_sic = re.compile(r'SatPhyRxCarrierPerFrame:ProcessFrame\(\): All successfully received packets processed')
    begin_marsala = re.compile(r'SatPhyRxCarrierMarsala:PerformMarsala\(\): Iterating packet in slot: (\d+)')
    end_marsala = re.compile(r'SatPhyRxCarrierMarsala:PerformMarsala\(\): Packet error: (\d)')

    iterated_slot = 0
    current_packet = 0
    interferences = []
    for line in logs:
        if end_sic.search(line):
            if interferences:
                yield SicEvent(date, frame_content.copy(), iterated_slot, replicas=interferences)
            return

        slot = begin_decode.search(line)
        if slot:
            if interferences:
                yield SicEvent(date, frame_content.copy(), iterated_slot, replicas=interferences)
                interferences = []
            s = int(slot.group(1))
            if s != iterated_slot:
                iterated_slot = s
                current_packet = 0
            current_packet += 1
            yield SicEvent(date, frame_content.copy(), iterated_slot, current_packet)
            continue

        replica = begin_replica.search(line)
        if replica:
            if interferences:
                yield SicEvent(date, frame_content.copy(), iterated_slot, replicas=interferences)
                interferences = []
            iterated_slot = int(replica.group(1))
            frame_content[iterated_slot] -= 1
            continue

        marsala = begin_marsala.search(line)
        if marsala:
            if interferences:
                yield SicEvent(date, frame_content.copy(), iterated_slot, replicas=interferences)
                interferences = []
            s = int(marsala.group(1))
            if s != iterated_slot:
                iterated_slot = s
                current_packet = 0
            current_packet += 1
            yield MarsalaEvent(date, frame_content.copy(), iterated_slot, current_packet)
            continue

        # processed = end_processed.search(line)
        # if processed:
        #     yield SicEvent(date, frame_content.copy(), iterated_slot, current_packet, False)
        #     continue

        decoded = end_decode.search(line)
        if decoded:
            error = int(decoded.group(1))
            yield SicEvent(date, frame_content.copy(), iterated_slot, current_packet, not error)
            if not error:
                frame_content[iterated_slot] -= 1
                current_packet = 0
            continue

        correlated = end_marsala.search(line)
        if correlated:
            error = int(correlated.group(1))
            yield SicEvent(date, frame_content.copy(), iterated_slot, current_packet, not error)
            if not error:
                current_packet = 0
            continue

        interference = before_if.search(line)
        if interference:
            interferences.append([float(interference.group(1))])
            continue

        interference = after_if.search(line)
        if interference:
            interferences[-1].append(float(interference.group(1)))
            continue


def animate_frame_content(frame):
    frame_content = {}
    for slot in frame:
        date = slot[0].date
        slot_id = slot[0].slot
        frame_content[slot_id] = len(slot)
        yield Event(date, frame_content.copy())


def animate_sic(logs, frame_content):
    message_date = re.compile(r'^\+(\d+\.\d+)s\s')
    sic_begin = re.compile(
            r'SatPhyRxCarrierPerFrame:PerformSicCycles\(\): '
            'Searching for the next successfully received packet')
    for line in logs:
        if sic_begin.search(line):
            date = float(message_date.search(line).group(1))
            yield from generate_sic_from_logs(logs, date, frame_content)
            return


def simplify_frame_content(frame):
    return {
            slot[0].slot: len(slot)
            for slot in frame
    }


def process_frames(log_filename):
    logs = logs_content(log_filename)
    for frame in generate_frames_from_logs(logs):
        events = list(animate_frame_content(frame))
        frame_content = simplify_frame_content(frame)
        events.extend(animate_sic(logs, frame_content))
        yield events


def parse_logs(log_filename):
    logs = logs_content(log_filename)
    for frame in generate_frames_from_logs(logs):
        yield from animate_frame_content(frame)
        frame_content = simplify_frame_content(frame)
        yield from animate_sic(logs, frame_content)


def main():
    args = command_line_parser().parse_args()

    carrier_size = args.slots_count

    for event in parse_logs(args.log_file):
        print(event.format(carrier_size))


if __name__ == '__main__':
    main()
