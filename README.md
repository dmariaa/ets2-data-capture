# ets2-data-capture

Installed into [Euro Truck Simulator 2:tm:](https://eurotrucksimulator2.com/),
this piece of software extracts frames, depth maps and telemetry data as you drive
in the game.

Written as part of my degree in videogame design and development final project,
I have used it to generate a dataset [ETS2 Dataset](https://ets2-dataset.dmariaa.es)
to train depth estimation neural networks.

> :warning: &nbsp;This software is highly experimental, not suitable for
> production environments.

# Compiling
Just download the repo including subprojects, make sure you have installed
DirectX SDK, open it with Visual Studio 2019 and build the solution. Last step of build will copy the resulting
DLL to the Euro Truck Simulator 2:tm: plugins folder (STEAM VERSION) and it will
be loaded as you load the game.

# Opening GUI

Just use Ctrl+º to open the GUI when you are in the ETS2 menu.
