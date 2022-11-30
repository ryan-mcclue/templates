export_set(file, terminal) = sprintf("set t push; set t %s; set o '%s'", terminal, file)
export_reset = "set t pop; set o"
normalise(val, min, max) = (val - min) / (max - min)

N = (9 < ARGC)?9:ARGC
array ARGV[N]

do for [i=1:N] {
  eval sprintf("ARGV[%d] = ARG%d", i, i);
}

eval export_set("metrics.png", "pngcairo")

# NOTE(Ryan): Look at https://i.stack.imgur.com/x6yLm.png to see possible palette gradient endpoints
set palette defined (0 "dark-orange", 1 "sienna1")
unset colorbox

set multiplot layout |ARGV|,1

set yrange [0:]
set grid x,y
unset xtics

# IMPORTANT(Ryan): If use 'plot for []', graphs will overlap 
do for [i=1:N] {
  set xlabel ARGV[i]
  plot ARGV[i] u 0:1 notitle w lp lc palette frac (i / (|ARGV| + 0.0))
}

unset multi

eval export_reset
