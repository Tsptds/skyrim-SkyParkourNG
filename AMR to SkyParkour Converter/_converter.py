import re
import sys

filename = sys.argv[1]

with open(filename, "r") as file:
    raw_annotations = file.read()

# Regular expressions for parsing
motion_re = re.compile(r'^([\d.]+)\s+animmotion\s+([\-\d.]+)\s+([\-\d.]+)\s+([\-\d.]+)')
sound_re  = re.compile(r'^([\d.]+)\s+(SoundPlay\..+)')

# Containers for parsed events
motions = []
sounds  = []

# Parse the raw annotations
for line in raw_annotations.splitlines():
    line = line.strip()
    if m := motion_re.match(line):
        t, x, y, z = m.groups()
        motions.append((float(t), float(x), float(y), float(z)))
    elif s := sound_re.match(line):
        t, desc = s.groups()
        sounds.append((float(t), desc))

# Build cumulative SkyParkour_Move entries from start
moves = []
start_x, start_y, start_z = motions[0][1], motions[0][2], motions[0][3]
for i in range(1, len(motions)):
    t0 = motions[i-1][0]
    t1, x1, y1, z1 = motions[i]
    duration = t1 - t0
    dx, dy, dz = x1 - start_x, y1 - start_y, z1 - start_z
    moves.append((t0, f"SkyParkour_Move.{dx}_{dy}_{dz}@{duration:.6f}"))

# Merge moves and sounds, sorted by timestamp
all_events = [(t, desc) for t, desc in moves] + sounds
all_events.sort(key=lambda e: e[0])

# Output to file
with open(filename, "w") as f:
    for t, desc in all_events:
        f.write(f"{t:.6f} {desc}\n")

print(f"Converted: {filename}")
