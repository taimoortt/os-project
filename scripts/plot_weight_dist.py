#!/usr/bin/python3
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
n_cells = 7

def heatmap(data, row_labels, col_labels, ax=None,
            cbar_kw=None, cbarlabel="", **kwargs):
    """
    Create a heatmap from a numpy array and two lists of labels.

    Parameters
    ----------
    data
        A 2D numpy array of shape (M, N).
    row_labels
        A list or array of length M with the labels for the rows.
    col_labels
        A list or array of length N with the labels for the columns.
    ax
        A `matplotlib.axes.Axes` instance to which the heatmap is plotted.  If
        not provided, use current axes or create a new one.  Optional.
    cbar_kw
        A dictionary with arguments to `matplotlib.Figure.colorbar`.  Optional.
    cbarlabel
        The label for the colorbar.  Optional.
    **kwargs
        All other arguments are forwarded to `imshow`.
    """

    if ax is None:
        ax = plt.gca()

    if cbar_kw is None:
        cbar_kw = {}

    # Plot the heatmap
    im = ax.imshow(data, **kwargs)

    # Create colorbar
    cbar = ax.figure.colorbar(im, ax=ax, **cbar_kw)
    cbar.ax.set_ylabel(cbarlabel, rotation=-90, va="bottom")

    # Show all ticks and label them with the respective list entries.
    ax.set_xticks(np.arange(data.shape[1]), labels=col_labels)
    ax.set_yticks(np.arange(data.shape[0]), labels=row_labels)

    # Let the horizontal axes labeling appear on top.
    ax.tick_params(top=True, bottom=False,
                   labeltop=True, labelbottom=False)

    # Rotate the tick labels and set their alignment.
    plt.setp(ax.get_xticklabels(), rotation=-30, ha="right",
             rotation_mode="anchor")

    # Turn spines off and create white grid.
    ax.spines[:].set_visible(False)

    ax.set_xticks(np.arange(data.shape[1]+1)-.5, minor=True)
    ax.set_yticks(np.arange(data.shape[0]+1)-.5, minor=True)
    ax.grid(which="minor", color="w", linestyle='-', linewidth=3)
    ax.tick_params(which="minor", bottom=False, left=False)

    return im, cbar

def annotate_heatmap(im, data=None, valfmt="{x:.2f}",
                     textcolors=("black", "white"),
                     threshold=None, **textkw):
    """
    A function to annotate a heatmap.

    Parameters
    ----------
    im
        The AxesImage to be labeled.
    data
        Data used to annotate.  If None, the image's data is used.  Optional.
    valfmt
        The format of the annotations inside the heatmap.  This should either
        use the string format method, e.g. "$ {x:.2f}", or be a
        `matplotlib.ticker.Formatter`.  Optional.
    textcolors
        A pair of colors.  The first is used for values below a threshold,
        the second for those above.  Optional.
    threshold
        Value in data units according to which the colors from textcolors are
        applied.  If None (the default) uses the middle of the colormap as
        separation.  Optional.
    **kwargs
        All other arguments are forwarded to each call to `text` used to create
        the text labels.
    """

    if not isinstance(data, (list, np.ndarray)):
        data = im.get_array()

    # Normalize the threshold to the images color range.
    if threshold is not None:
        threshold = im.norm(threshold)
    else:
        threshold = im.norm(data.max())/2.

    # Set default alignment to center, but allow it to be
    # overwritten by textkw.
    kw = dict(horizontalalignment="center",
              verticalalignment="center")
    kw.update(textkw)

    # Get the formatter in case a string is supplied
    if isinstance(valfmt, str):
        valfmt = matplotlib.ticker.StrMethodFormatter(valfmt)

    # Loop over the data and create a `Text` for each "pixel".
    # Change the text's color depending on the data.
    texts = []
    for i in range(data.shape[0]):
        for j in range(data.shape[1]):
            kw.update(color=textcolors[int(im.norm(data[i, j]) > threshold)])
            text = im.axes.text(j, i, valfmt(data[i, j], None), **kw)
            texts.append(text)

    return texts

def get_weights(fname):
    lines = []
    last_index = -1
    with open(fname, "r") as fin:
        lines = fin.readlines()
    for i, line in enumerate(lines):
        words = line.split(" ")
        if words[0] == "total_loss:":
            last_index = i
            break
    wdiff_percell = []
    for i in range(n_cells):
        final_w = []
        ideal_w = []
        words = lines[last_index - n_cells + i].split(" ")
        for i in range(2, len(words)-1, 3):
            final_w.append( float( words[i][1:-1] ) )
        for i in range(4, len(words)-1, 3):
            ideal_w.append( float( words[i][:-2] ) )
        assert(len(final_w) == len(ideal_w))
        wdiff = []
        for i in range(len(final_w)):
            wdiff.append( final_w[i] - ideal_w[i] )
        wdiff_percell.append( wdiff )
    return wdiff_percell

def get_baseline_weights(fname):
    n_slices = 5
    lines = []
    last_index = -1
    with open(fname, "r") as fin:
        lines = fin.readlines()
    for i, line in enumerate(lines):
        words = line.split(" ")
        if words[0] == "total_loss:":
            last_index = i
            break
    wdiff_percell = []
    for i in range(n_cells):
        final_w = [1/n_slices for j in range(n_slices)]
        ideal_w = []
        words = lines[last_index - n_cells + i].split(" ")
        for i in range(4, len(words)-1, 3):
            ideal_w.append( float( words[i][:-2] ) )
        assert(len(final_w) == len(ideal_w))
        wdiff = []
        for i in range(len(final_w)):
            wdiff.append( final_w[i] - ideal_w[i] )
        wdiff_percell.append( wdiff )
    return wdiff_percell

def plot_heatmap(tag, baseline=False):
    if baseline:
        wdiff_percell = np.array(get_baseline_weights(tag+".log"))
    else:
        wdiff_percell = np.array(get_weights(tag+".log"))

    cells = []
    for i in range(n_cells):
        cells.append("c" + str(i))
    n_slices = len(wdiff_percell[0])
    slices = []
    for i in range(n_slices):
        slices.append("s" + str(i))

    #fig, ax = plt.subplots(figsize=(7,8))
    fig, ax = plt.subplots()
    im, cbar = heatmap(wdiff_percell, cells, slices, ax=ax, vmin=-0.5, vmax=0.2)
    texts = annotate_heatmap(im)
    sum_diff = np.sum(np.square(wdiff_percell))
    ax.set_title("total_diff: %.3f" % (sum_diff))
    fig.tight_layout()
    if baseline:
        fig.savefig("baseline_heatmap.png")
    else:
        fig.savefig(tag+"_heatmap.png")

plot_heatmap("naive2_1", True)
plot_heatmap("naive2_1", False)
plot_heatmap("annealing2_1", False)
