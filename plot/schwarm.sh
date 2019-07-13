#!/bin/sh

alarm=0
ext_alarm=""

for ext in WAAGE00 WAAGE01 WAAGE02 WAAGE03 WAAGE04 WAAGE05 WAAGE06 WAAGE07
do
	datei=/daten/$(date --date="today" "+%Y-%m-%d")_$ext

	/usr/bin/tail -n16 $datei | /opt/bienen/bin/findjump -k2 -s300

	if [ $? != 0 ]
	then
		echo Schwarm-Alarm in Datei $datei um $(date) >> /tmp/schwarm.log
		alarm=1;
		ext_alarm=$ext
	fi
done

if [ $alarm != 0 ]
then
	echo "schicke Scharm-Alarm bei $ext_alarm - $datei"
	/opt/bienen/bin/plot_wa_today.sh "$ext_alarm - $datei"
fi

