set confirm off
set trace-commands on

define target remote
target extended-remote $arg0
set mem inaccessible-by-default off
source /Users/igordanilov/.ufbt/current/scripts/debug/flipperapps.py
fap-set-debug-elf-root /Users/igordanilov/.ufbt/build
end
