file=$1

echo generating histogram of file: $file

echo "
set term postscript enhanced color solid 8
set output 'histo.ps'
set xlabel 'Value'
set ylabel 'Frequency'

binwidth=0.1
#set boxwidth binwidth

bin(x,width)=width*floor(x/width) + binwidth/2.0

set title 'Histogram'

" > gnu.tmp

echo "plot '$file' using (bin(\$1,binwidth)):(1.0) smooth freq with boxes t 'histogram of values in $file' " >> gnu.tmp

gnuplot gnu.tmp

ps2pdf histo.ps

rm -rf *.tmp

open histo.pdf
