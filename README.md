# StarWarsEmpireAtWar
Sunday, 20 December 2020

This project was created to facilitate in modding Star Wars Empire At War. The motiviation for this project was that dealing with XML files by hand and linking them and trying to understand can be a lot of work. While being fun, there's room for error.

The repo is split into 3 projects: CLI, GUI, DLL
DLL is the meat of the code. CLI and GUI both reference the DLL. This way we don't have a lot of repeating code. This version is Windows, but it wouldn't be hard to make a macOS branch. I may even add it just for fun.

CLI needs no introduction. For those who loathe the GUI or just want a CLI for those moments that you don't want to be bothered with a GUI.

The GUI of course is the main reason for this project as stated above.

This project is also in pure C. I have seen CLIs out there that I'm not too sure what they use. I have also seen basic GUIs, but they require .NET. I'm not against it, but what if you want to install this on a retro machine that doesn't have all of that setup? Installing .NET 4 or even .NET 5 may be great since there's more security and a lot of great C# magic, but may not work on some setups. This project should theoretically work on older hardware. Perhaps with some minor adjustments.

I don't expect anyone to jump in and help, but even if people fork this would be great. I think also in the future there could be integration with other things other than the XML and strings. A rough idea would be as follows:

Milestone 1: XML/Strings
Milestone 2: integration with audio
Milestone 3: integration with images
Milestone 4: integration with video

There could be more, but for now that's it. This file will be updated in the future.