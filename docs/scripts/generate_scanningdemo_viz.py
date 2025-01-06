import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ------------------------------------------------------------------------------
# USER-ADJUSTABLE PARAMETERS
# ------------------------------------------------------------------------------
npts = 512          # total points (entire buffer)
k = 1                # Lissajous pattern index from the PlayzerX demo
interval_ms = 1      # delay (ms) between animation frames
                     # => 1 ms * 5000 frames = ~5 seconds total

# We'll run exactly 5000 frames at 1 ms interval => ~5 seconds:
frames_to_animate = 400

# ------------------------------------------------------------------------------
# COMPUTE X, Y ARRAYS (SIMILAR TO SCANNINGDEMO, IGNORING 'M' FOR THIS VISUAL)
# ------------------------------------------------------------------------------
dt = 2.0 * math.pi / npts
x = np.zeros(npts, dtype=float)
y = np.zeros(npts, dtype=float)

for i in range(npts):
    # Reflecting ScanningDemo logic:
    # x[i] = sin(10.f * k * i * dt);
    # y[i] = 0.9f * sin((5.f * (k + 1) + 1) * i * dt);
    x[i] = math.sin(10.0 * k * i * dt)
    y[i] = 0.9 * math.sin((5.0 * (k + 1) + 1) * i * dt)

# ------------------------------------------------------------------------------
# MATPLOTLIB SETUP
# ------------------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 8))
ax.set_xlim(-1.1, 1.1)
ax.set_ylim(-1.1, 1.1)
ax.set_aspect('equal', 'box')
ax.set_title("PlayzerX ScanningDemo Visualization\n"
             "(Showing the whole buffer and the current sample)\n"
             f"Number of points: {npts}, Sample rate: Faster")

# We'll create a static scatter for all npts points, then dynamically update
# each point's color/size to reflect how the device is stepping through them.
points = ax.scatter(x, y, s=5, c="gray")  # all start as 'future' (gray)

# We'll also add a text label at the top-left to show samples remaining:
samples_text = ax.text(0.03, 0.95, "", transform=ax.transAxes,
                       ha="left", va="top", fontsize=10,
                       bbox=dict(boxstyle="round", fc="w", alpha=0.7))

# For easier color/size management, store arrays for them:
colors = np.full(npts, "gray", dtype=object)  # all "future"
sizes = np.full(npts, 5.0)
alphas = np.full(npts, 0.5)

# Create proxy artists for the legend
blue_patch = plt.Line2D([0], [0], marker='o', color='w', label='Sampled (Blue)',
                        markerfacecolor='blue', markersize=5, alpha=0.5)
green_patch = plt.Line2D([0], [0], marker='o', color='w', label='Currently outputting (Green)',
                         markerfacecolor='green', markersize=10)
red_patch = plt.Line2D([0], [0], marker='o', color='w', label='Remaining samples (Red)',
                       markerfacecolor='red', markersize=5, alpha=0.5)

# Add the legend to the plot
ax.legend(handles=[blue_patch, green_patch, red_patch], loc='upper right')

ax.set_xlim(-1.1, 1.1)
ax.set_ylim(-1.1, 1.4)

# ------------------------------------------------------------------------------
# ANIMATION FUNCTIONS
# ------------------------------------------------------------------------------
def init():
    points.set_color(colors)
    points.set_sizes(sizes)
    samples_text.set_text("")
    return points, samples_text

def update(frame):
    # 'frame' is the current "time step"
    # We'll treat it as the index for the 'current' sample being output.

    # If frame >= npts, that means we've conceptually scanned all samples.
    # But we may still be animating to fill 5 seconds total. We'll freeze.
    if frame >= npts:
        return points, samples_text

    # Past (scanned) samples: 0..frame-1
    colors[:frame] = "blue"
    sizes[:frame] = 5.0
    alphas[:frame] = 0.5

    # Current sample
    colors[frame] = "green"
    sizes[frame] = 30.0
    alphas[frame] = 1.0

    # Future samples: frame+1..(npts-1)
    if frame < npts - 1:
        colors[frame+1:] = "red"
        sizes[frame+1:] = 5.0
        alphas[frame+1:] = 0.5

    # Update scatter
    points.set_color(colors)
    points.set_sizes(sizes)
    points.set_alpha(alphas)

    # Show how many samples remain un-scanned
    samples_remaining = max(0, npts - 1 - frame)
    samples_text.set_text(f"Current sample: {frame}\nSamples remaining: {samples_remaining}")

    return points, samples_text

# 5-second animation => 5000 frames * 1 ms each
ani = FuncAnimation(
    fig,
    update,
    frames=range(frames_to_animate),
    init_func=init,
    blit=False,
    interval=interval_ms
)

# -- If you only want to save, you can skip plt.show() and do the save step below. --
# plt.show()

# -- Save as a GIF (requires Pillow) --
ani.save("scanningdemo_vis.gif", writer="pillow", fps=30)
print("Saved animation as scanningdemo_vis.gif")
