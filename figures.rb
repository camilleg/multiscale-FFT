#!/usr/bin/env ruby

abort "Usage: #$0 out.png numrows in.csv" if ARGV.size != 3
$out, $numrows, $csv = ARGV
$numrows = $numrows.to_i

# For $numrows, 3 reproduces the published paper's figures.
# 4 goes one farther.
# Larger values' plots are unreadably skinny.

require 'csv'
$a = CSV.read $csv
$a = $a[1..-1] # Strip header line.
N = $a.size # 2048 rows.
# 12 columns, the last of which is nil because of a trailing comma in the .csv.

require 'gnuplot'

def plot filename, n, row, data
  x = (0..n).map {|v| v.to_f/n * 2*Math::PI}
  # If data is a horizontal line, don't hide it under the top or bottom line
  # like autoscaling would.
  if data.max == 0.0
    ymin = -1.0
    ymax = 1.0
  else
    ymin = [0.0, data.min].min
    ymax = [1.0, data.max * 1.1].max
  end
  Gnuplot.open {|gp|
    Gnuplot::Plot.new(gp) {|p|
      p.output filename
      p.terminal "png size #{1900 >> row},350"
      p.title  "N=#{n}"
      p.xlabel "Frequency, 0 to 2π"
      p.ylabel "Amplitude"
      p.xrange "[0:#{2*Math::PI}]"
      p.yrange "[#{ymin}:#{ymax}]"
      p.data << Gnuplot::DataSet.new([x,data]) {|ds|
	ds.with = 'lines'
	ds.linecolor = 'rgb "#0000FF"' # or 'rgb "blue"'
	# ds.linewidth = 2
	ds.notitle
      }
    }
  }
end

def png(i) "/tmp/#{i}.png" end
def pngrow(i) "/tmp/row#{i}.png" end

$numrows.times {|j|
  $n = N >> j
  $plots = 1 << j
  b = $a.map {|x| x[-2-j].to_f}
  $plots.times {|i| plot png(i), $n, j, b[$n*i...$n*(i+1)]}
  cols = (0...$plots).map{|i| png(i)}.join(' ')
  `convert #{cols} +append #{pngrow(j)}`
  `rm #{cols}`
}
rows = (0...$numrows).map{|i| pngrow(i)}.join(' ')
`convert #{rows} -append #$out`
`rm #{rows}`

# To tile the plots, use imagemagick's convert,
# because rubygems.org/gems/gnuplot-multiplot is clunky.
