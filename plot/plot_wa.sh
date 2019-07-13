#!/bin/sh

jahr=$(date --date="today" "+%Y")

verz=/auswertung/${jahr}

[ -d ${verz} ] || mkdir -p ${verz}

for i in $(seq 1 1)
do
	dateiname=$(date --date="$i days ago" "+%Y-%m-%d")

	for ext in WAAGE00 WAAGE01 WAAGE02 WAAGE03 WAAGE04 WAAGE05 WAAGE06 WAAGE07
	do
		cat /daten/${dateiname}_${ext} | spike -k2 -s30 | awk '{print $1 "\t" $2 "\t" $3}' > /tmp/${dateiname}_${ext}
	done

	for ext in BME00
	do
		cat /daten/${dateiname}_${ext} | spike -k2 -s1 | spike -k3 -s1 | spike -k4 -s1 | awk '{print $1 "\t" $2 "\t" $3 "\t" $4}'  > /tmp/${dateiname}_${ext}
		cat /daten/${dateiname}_${ext} | spike -k2 -s1 | spike -k3 -s1 | spike -k4 -s1 > /tmp/${dateiname}_ALL

		regen=$(cat /daten/${dateiname}_${ext} | awk '{x=x+$5}END{x=x/2; print x}')
	done

	datei=${verz}/${dateiname}

	join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE00 > /tmp/join1
	join /tmp/join1 /tmp/${dateiname}_WAAGE01 > ${datei}.wa0001

	join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE02 > /tmp/join2
	join /tmp/join2 /tmp/${dateiname}_WAAGE03 > ${datei}.wa0203

	join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE04 > /tmp/join2
	join /tmp/join2 /tmp/${dateiname}_WAAGE05 > ${datei}.wa0405

	join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE06 > /tmp/join2
	join /tmp/join2 /tmp/${dateiname}_WAAGE07 > ${datei}.wa0607

	cat /tmp/${dateiname}_BME00 > ${datei}.bme00
	cat /tmp/${dateiname}_ALL > ${datei}.all
done

zwdatei=${verz}/zwoche.$(date --date="today" "+%Y-%m-%d")

echo > ${zwdatei}.wa0001
echo > ${zwdatei}.wa0203
echo > ${zwdatei}.wa0405
echo > ${zwdatei}.wa0607
echo > ${zwdatei}.bme00
echo > ${zwdatei}.all

for i in $(seq 0 13)
do
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").wa0001 | grep -v '^#' >> ${zwdatei}.wa0001
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").wa0203 | grep -v '^#' >> ${zwdatei}.wa0203
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").wa0405 | grep -v '^#' >> ${zwdatei}.wa0405
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").wa0607 | grep -v '^#' >> ${zwdatei}.wa0607
	####
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").bme00 | grep -v '^#' >> ${zwdatei}.bme00
	cat ${verz}/$(date --date="14 days ago + $i day" "+%Y-%m-%d").all | grep -v '^#' >> ${zwdatei}.all

done


regendatei=${verz}/monat.$(date --date="today" "+%Y-%m-%d")
echo > ${regendatei}.regen

for i in $(seq 0 31)
do
	datum=$(date --date="32 days ago + $i day" "+%Y-%m-%d")

	regen=$(cat ${verz}/${datum}.all | grep -v '^#' | awk '{x=x+$5}END{x=x/2; print x}')

	if [ $i == 31 ]
	then
		rgestern=$regen
		echo "rgestern: " $rgestern
	fi;

	echo ${datum} ${regen} >> ${regendatei}.regen
done

gnuplot -e "datei='${regendatei}'" -e "ext='regen'" /opt/bienen/bin/regen.plt


gnuplot -e "datei='${datei}'" -e "ext='wa0001'" -e "stocka='Stock-00'" -e "stockb='Stock-01'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${datei}'" -e "ext='wa0203'" -e "stocka='Stock-02'" -e "stockb='Stock-03'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${datei}'" -e "ext='wa0405'" -e "stocka='Stock-04'" -e "stockb='Stock-05'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${datei}'" -e "ext='wa0607'" -e "stocka='Stock-06'" -e "stockb='Stock-07'" /opt/bienen/bin/waage_daten.plt

gnuplot -e "datei='${zwdatei}'" -e "ext='wa0001'" -e "stocka='Stock-00'" -e "stockb='Stock-01'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${zwdatei}'" -e "ext='wa0203'" -e "stocka='Stock-02'" -e "stockb='Stock-03'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${zwdatei}'" -e "ext='wa0405'" -e "stocka='Stock-04'" -e "stockb='Stock-05'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${zwdatei}'" -e "ext='wa0607'" -e "stocka='Stock-06'" -e "stockb='Stock-07'" /opt/bienen/bin/waage_daten.plt

gnuplot -e "datei='${datei}'" -e "ext='all'" /opt/bienen/bin/bme_daten.plt
gnuplot -e "datei='${zwdatei}'" -e "ext='all'" /opt/bienen/bin/bme_daten.plt

mutt -s "Bienenwaage-V2 - $(date --date='1 days ago' '+%Y-%m-%d')" -a ${datei}.wa0001.png -a ${zwdatei}.wa0001.png -a ${datei}.wa0203.png -a ${zwdatei}.wa0203.png -a ${datei}.wa0405.png -a ${zwdatei}.wa0405.png -a ${datei}.wa0607.png -a ${zwdatei}.wa0607.png -a ${datei}.all.png -a ${zwdatei}.all.png -a ${regendatei}.png -- ak@it-klinger.de <<EOF
Bienenwaage-V2

Regen: $regen [l/m2]
Regen: $rgestern [l/m2]

${datei}.wa0001.png
${datei}.wa0203.png
${datei}.wa0405.png
${datei}.wa0607.png
${zwdatei}.wa0001.png
${zwdatei}.wa0203.png
${zwdatei}.wa0405.png
${zwdatei}.wa0607.png
${datei}.all.png
${zwdatei}.all.png
${regendatei}.png
EOF

exit 0;

