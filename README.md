# rmdlconv
copyright (c) 2022, rexx

## instructions
1. drag and drop .mdl file on rmdlconv.exe

OR

1. make a batch file with one or more of the supported commands.
2. run the batch file.

---
### supported versions
main versions:
- Portal 2 (v49) -> Apex Legends Season 3 (v54 - rmdl v10)
- Titanfall 2 (v53) -> Apex Legends Season 3 (v54 - rmdl v10)

partially supported:
- Titanfall (v52) -> Titanfall 2 (v53)

unsupported but planned:
- Portal 2 (v49) -> Titanfall 2 (v53)
- Titanfall (v52) -> Apex Legends Season 3 (v54 - rmdl v10)


### supported commands
- "-nopause": automatically close console after running
- "-convertmodel": path to model(s) you wish to convert
  examples: "-convertmodel C:\Among\us.mdl" "-convertmodel C:\Among"
- "-targetversion": version you would like models to be upgraded to
  examples: "-targetversion 53" "-targetversion 54"
- "-outputdir": custom directory for files to be output into
  examples: "-outputdir E:\SuS"
- "-convertsequence": unfinished

### known issues
animation conversion is not currently supported and there may be various issues when using models in game

converted models will almost definitely **not work** in R5Reloaded if they only contain 1 bone, as the game deals with them differently
