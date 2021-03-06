#!/bin/sh

cd "$(dirname "$0")"

# build
rm -rf build &>/dev/null
mkdir build
cd build || exit 1
(CXXFLAGS="-frecord-gcc-switches" cmake ../.. && make neon-diff) || exit 1
echo "-- Used compiler flags"
gccflags=$(readelf -p .GCC.command.line neon-diff | cut -c 13- | grep -vP '^(\s*$|[^-]|-(I|D_|aux|frecord))' | sed -E -e ':a;N;$!ba;' -e 's/\n/ /g')
echo "   $gccflags"
cd ..

# patch file
if [[ ! -f performance.diff ]]; then
	gunzip -k performance.diff.gz
fi

# time it
echo "-- Measuring performance"
echo -e "compiled with '$gccflags'" > performance.log

# defaults
s=0; n=3; cmd="build/neon-diff performance.diff"
echo " $cmd"
for i in $(seq 1 $n); do
	echo -n " ..."
	t=$(bash -c "time -p $cmd &>/dev/null" 2>&1 | sed -E -e ':a;N;$!ba;' -e 's/real ([0-9.]+).*/\1/')
	s=$(bc <<<"scale=4; $s + $t")
	echo " ${t}sec"
done
s=$(bc <<<"scale=2; $s / $n")
echo " average ${s}sec"
echo -e "command '$cmd'\n\taverage time was ${s}sec over $n executions" >> performance.log

# strip spaces like before
s=0; n=3; cmd="build/neon-diff --ignore-spaces performance.diff"
echo " $cmd"
for i in $(seq 1 $n); do
	echo -n " ..."
	t=$(bash -c "time -p $cmd &>/dev/null" 2>&1 | sed -E -e ':a;N;$!ba;' -e 's/real ([0-9.]+).*/\1/')
	s=$(bc <<<"scale=4; $s + $t")
	echo " ${t}sec"
done
s=$(bc <<<"scale=2; $s / $n")
echo " average ${s}sec"
echo -e "command '$cmd'\n\taverage time was ${s}sec over $n executions" >> performance.log

# process whole blocks
s=0; n=3; cmd="build/neon-diff --reparse-range performance.diff"
echo " $cmd"
for i in $(seq 1 $n); do
	echo -n " ..."
	t=$(bash -c "time -p $cmd &>/dev/null" 2>&1 | sed -E -e ':a;N;$!ba;' -e 's/real ([0-9.]+).*/\1/')
	s=$(bc <<<"scale=4; $s + $t")
	echo " ${t}sec"
done
s=$(bc <<<"scale=2; $s / $n")
echo " average ${s}sec"
echo -e "command '$cmd'\n\taverage time was ${s}sec over $n executions" >> performance.log
